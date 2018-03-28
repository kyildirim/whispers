#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ipc.h>   
#include <sys/msg.h>  
#include <errno.h> 
#include <time.h>

#define INVALID_ARG_COUNT -31

int main(int argc, char **argv){

	if(argc!=2)return INVALID_ARG_COUNT;

	//Seed random
	srand(time(NULL));

	int len = strlen(argv[1]);

	//Create message queue
	int mq = msgget(IPC_PRIVATE, 0600|IPC_CREAT|IPC_EXCL);
	if (mq == -1) {
    	perror("msgget");
    	exit(-1);
	}

	//Message buffer struct
	struct msgbuf {
    	long mtype;     
    	char mtext[1];
	};

	char* msg = argv[1];
	struct msgbuf *mb = (struct msgbuf*)malloc(sizeof(struct msgbuf)+len);
	mb->mtype = 1;

	strcpy(mb->mtext, msg);

	//Send to first process to start the cycle
	int rc = msgsnd(mq, mb, len+1, 0);
	if (rc == -1) {
	    perror("msgsnd");
	    exit(-2);
	}

	int rands[len];
	int r;
	//Generate random numbers for probability calculations.
	for(r = 0; r<len; r++)rands[r]=rand();
	//Fork
	int i;
	for(i = 0; i < len; i++) {
	    int pid = fork();
	    if(pid < 0) {
	        printf("Error");
	        exit(1);
	    } else if (pid == 0) {
	    	int p = i+1;
	    	//printf("%d alive\n", p);
	        struct msgbuf* rMsg = (struct msgbuf*)malloc(sizeof(struct msgbuf)+len);
	        int rc = msgrcv(mq, rMsg, len+1, p, 0);
	        //printf("%d recieved\n", p);
			if (rc == -1) {
			    perror("msgrcv");
			    exit(1);
			}
			if((rands[i]%2)==0)rMsg->mtext[p-1]++;
			rMsg->mtype=p+1;
			if(p==len){
				rMsg->mtype=1;
			}
			rc = msgsnd(mq, rMsg, len+1, 0);
			//printf("%d sent\n", p);
			if(p==1){ //First child waits for last message.
				rc = msgrcv(mq, rMsg, len+1, p, 0);
				printf("%s\n", rMsg->mtext);
			}
	        return 0;
	    } else  {

	    }
	}

	return 0;

}