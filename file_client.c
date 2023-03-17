#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

char pid[10];
void* communicationThread();
int inputCheck(char* input);

int main(){
	//client ilk acildiginda getpid ile pid alinip manager'a yollaniyor.
	//ayni zamanda manager ile namedpipe uzerinden iletisim icin client kendisi bir thread olusturuyor.
	
	int fd;
	
	char * myfifo = "/tmp/mainfifo";
	mkfifo(myfifo,0666);
	
	int x = getpid();
	sprintf(pid,"%d",x);
	fd = open(myfifo,O_WRONLY);
	write(fd,pid,10);
	close(fd);
	
	pthread_t tid;
	
	pthread_create(&tid, NULL, communicationThread, NULL);
	
	pthread_join(tid, NULL);
		
	return 0;
}

void* communicationThread(){
	//manager ile iletisim kurulan thread
	
	int fd;
	char myfifo[20] = "/tmp/";
	strcat(myfifo,pid);
	
	mkfifo(myfifo,0666);
	
	char input[30];
	char response[50];
	
	char tempStr[30];
	
	
	while(1){
		fgets(input,30,stdin);
		strcpy(tempStr,input);
		if(inputCheck(tempStr)==1){
			fd = open(myfifo,O_WRONLY);
			write(fd,input,30);
			close(fd);
			
			fd = open(myfifo,O_RDONLY);
			read(fd,response,50);
			printf("%s",response);
			if((strcmp(response,"terminating")==0)){
				break;
			}
			close(fd);
				
		}

	}

}

int inputCheck(char* input){
	//input kontrolu yapilan kisim
	
	char StrCreate[] = "create";
	char StrDelete[] = "delete";
	char StrRead[] = "read";
	char StrWrite[] = "write";
	char StrExit[] = "exit\n";
	
	char delim[2] = " ";
	int input_size = strlen(input);
	
	char* ptr = strtok(input,delim);
	
	while(ptr != NULL){
		if(strcmp(ptr,StrCreate)==0){
			if(strtok(NULL,delim)!=NULL){
				return 1;
			}
		}else if(strcmp(ptr,StrDelete)==0){
			if(strtok(NULL,delim)!=NULL){
				return 1;
			}
		}else if(strcmp(ptr,StrRead)==0){
			if(strtok(NULL,delim)!=NULL){
				return 1;
			}
		}else if(strcmp(ptr,StrWrite)==0){
			if(strtok(NULL,delim)!=NULL){
				if(strtok(NULL,delim)!=NULL){
				return 1;
				}
			}
		}else if(strcmp(ptr,StrExit)==0){
			return 1;
		}
		printf("wrong input!");
		return 0;
	}
}










