#include <stdio.h>
#include <fcntl.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

enum status { on, off};
typedef enum status status;

//enum boolean { false, true };
//typedef enum boolean boolean;

typedef bool boolean;

unsigned  short i = 0;
int pid[10] = {0};
char process[256];	// Name of user inputted process. 

int curProcess = 0; 	// Current running process. Is global to prevent re-initializing.
int numProcess = 0;	// Total number of active processes. Incremented everytime
			// the shedualer is called.

int pidCount = 0;	// Utilized by the scheduler. Is global to prevent the
			// re-initialization of a process.

status flag = on;

struct sigaction newhandler, oldhandler;
sigset_t sig;


/*
This is the block command for the console reader.
*/

void setblock( int fd, boolean block)
{
   static int blockf, nonblockf;
   static boolean first = true;

   int flags;

   if (first) {
      first = false;
      if (( flags = fcntl(fd,F_GETFL,0)) == -1)
      {
         fprintf(stderr," fcntl - error \n");
         exit(1);
      }
     blockf = flags & ~ O_NDELAY;
     nonblockf = flags | O_NDELAY;
  }
  if ( fcntl(fd,F_SETFL, block ? blockf : nonblockf) == -1 )
      {
         fprintf(stderr," fcntl2 - error \n");
         exit(1);
      }
}

// ----------------------------------------

/*
The scheduler is responsible for forking the process, and
having the children execute the code corresponding to the user's
program. Each child's PID will be stored in a pid array for
handling by the monitor function. 
*/

void scheduler() {

	char path[256];		// Utilized in the execl call.

		pid[pidCount] = fork();		// Parent process stores PID.

		if ( pid[pidCount] == 0 ){	// While child w/o PID runs the user inputted program.

			strcpy(path, "./");  
			strcat(path, process);
			printf("About to execute: %s \n", path);

			execl(path, process, NULL); 

			printf("\n**** In child after execute. *****\n");
			printf("Failure to execute. Terminating child.\n");
			exit(EXIT_FAILURE);
        	}
		else{
			pidCount++;
			numProcess++;
		}
  	
}

/*
The next function's purpose is to handle the signals controlling
the children processes. In a round robin fashion is runs all the
children processes.
*/

void monitor( int code){

	static status flag = on;

	if(numProcess > 0){
		printf("\n\n**** In monitor. *****\n");
		printf("\nSending stop signal to %d\n", pid[curProcess]);

		kill(pid[curProcess], SIGSTOP);

		curProcess++;
		if(curProcess == numProcess) curProcess = 0;
		
		
		printf("Sending continue signal to %d\n", pid[curProcess]);
		kill(pid[curProcess], SIGCONT);
	}
}

/*
The main fuction first handles the user's program input. This is
done by the scheduler function. After which it (the parent) will
set an alarm and pause itself. When the SIGALRM signal is recieved
it will be handled in the next function. At which point these process
will repeat until the program is terminated.
*/

int main(){

	int fd;
	int numch;

		fd = open("/dev/tty",O_RDWR); // Opens the keyboard.
        	setblock(fd,false);	// Read won't block.

        	sigemptyset(&sig);  /* clear the signal mask */
        	newhandler.sa_handler = monitor; // Designates the handler.
        	newhandler.sa_mask = sig;
        	newhandler.sa_flags = 0;

        	if ( sigaction(SIGALRM,&newhandler,&oldhandler) == -1 ){
	
			printf("1 - can't install signal handler \n");
			exit(-1);
		}

        	while (1) {
			printf("\n**** In parent. ***** \n\n");
			alarm(1);
			pause();
			switch  ( numch = read(fd, process, 256))  {
				case -1 :
				case 0 :
					break;
				default: 

					process[numch-1] = '\0';
					scheduler();
					
			}

		
	}
return 0;
}
