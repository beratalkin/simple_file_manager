#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

char file_list[10][20]={NULL};
int file_list_index[10] = {0};
//burada file listesini tanimladim ve kontroller icin ona paralel olarak bir integer array tanimladim

typedef struct Task{
	char* clientPid;
}Task;

Task taskQueue[10];
int taskOrder = 0;

pthread_mutex_t mutexQueue;
pthread_cond_t condQueue;

const char* executeTask(char* input);
void submitTask(Task task);
void* startThread();

int main(){
	//main fonksiyonum clientlar acildiginda onlardan client pid'si bekliyor
	//gelen pid'lere göre her bir client icin ayri namedpipe threadi olusturuluyor
	pthread_t thread_pool[5];
	pthread_mutex_init(&mutexQueue, NULL);
	pthread_cond_init(&condQueue,NULL);
	for(int i = 0; i<5;i++){
		if(pthread_create(&thread_pool[i], NULL, &startThread, NULL) != 0){
			perror("Failed to create thread");
		}
	}
	int fd;
	
	char * myfifo = "/tmp/mainfifo";
	mkfifo(myfifo,0666);
	
	char input[30], response[30];
	
	while(1){
		sleep(1);
		fd = open(myfifo,O_RDONLY);
		read(fd,input,30);
		close(fd);
		printf("client pid: %s\n", input);
		Task t = {
			.clientPid = input
		};
		submitTask(t);
	}
	
	for(int i = 0; i<5;i++){
		if(pthread_join(thread_pool[i], NULL) != 0){
			perror("Failed to join thread");
		}
	}
	pthread_mutex_destroy(&mutexQueue);
	pthread_cond_destroy(&condQueue);
	return 0;
}

void clientCommunication(Task* task){
	//burasi manager threadlerinin clientlarla iletisim kurdugu yer
	int fd;
	char  myfifo[20] = "/tmp/";
	strcat(myfifo,task->clientPid);
	mkfifo(myfifo,0666);
	
	char input[30];
	const char* response;
	
	while(1){
		
		fd = open(myfifo,O_RDONLY);
		read(fd,input,30);
		close(fd);
		
		fd = open(myfifo,O_WRONLY);
	
		pthread_mutex_lock(&mutexQueue);
		response = executeTask(input);
		pthread_mutex_unlock(&mutexQueue);
	
		write(fd,response,50);
		close(fd);
		
	}
}

void submitTask(Task task){
	printf("submitted task\n");
	printf("%s",file_list[0]);
	pthread_mutex_lock(&mutexQueue);
	taskQueue[taskOrder] = task;
	taskOrder++;
	pthread_mutex_unlock(&mutexQueue);
	pthread_cond_signal(&condQueue);
}

void* startThread(){
	//threadleri baslatmak icin kullanılan fonksiyon
	printf("thread started\n");
	while(1){
		Task task;
		pthread_mutex_lock(&mutexQueue);
		while(taskOrder==0){
			pthread_cond_wait(&condQueue,&mutexQueue);
		}
		task = taskQueue[0];
		for(int i = 0; i<taskOrder-1;i++){
			taskQueue[i]=taskQueue[i+1];
		}
		taskOrder--;
		pthread_mutex_unlock(&mutexQueue);
		clientCommunication(&task);	
	}
}

const char* executeTask(char * input){
	//bu da create delete gibi gorevlerin yerine getirildigi fonksiyon
	FILE* fptr;
	
	char StrCreate[] = "create";
	char StrDelete[] = "delete";
	char StrRead[] = "read";
	char StrWrite[] = "write";
	char StrExit[] = "exit";
	
	char delim[2] = " ";
	int input_size = strlen(input);
	//strtok kullanarak string islemleri yaptim.
	
	char* ptr = strtok(input,delim);
	
	while(ptr != NULL){
		
		if((strncmp(StrCreate,ptr,6)==0)){
			char* filename = strtok(NULL,delim);
			filename[strlen(filename)-1]=0;
			for(int i = 0;i<10;i++){
				if(strcmp(file_list[i],filename)==0){
					return "there is a file with the same name\n";
				}
			}
			for(int i = 0; i<10;i++){
				if(file_list_index[i]==0){
					strcpy(file_list[i],filename);
					fptr=fopen(filename,"w");
					fclose(fptr);
					file_list_index[i]=1;
					return "file created succesfully\n";
				}
			}
			
		}else if((strncmp(StrDelete,ptr,6)==0)){
			char* filename = strtok(NULL,delim);
			filename[strlen(filename)-1]=0;
			for(int i = 0;i<10;i++){
				if(strcmp(file_list[i],filename)==0){
					strcpy(file_list[i],"\0");
					remove(filename);
					file_list_index[i]=0;
					return "file deleted.\n";
				}
			}
			return "no such file to delete.\n";
			
		}else if((strncmp(StrRead,ptr,4)==0)){
			char* filename = strtok(NULL,delim);
			filename[strlen(filename)-1]=0;
			for(int i = 0;i<10;i++){
				if(strcmp(file_list[i],filename)==0){
					char readStr[50];
					fptr=fopen(filename,"r");
					fgets(readStr,50,fptr);
					printf("%s",readStr);
					fclose(fptr);
					return "read succesfully.\n";
					
				}
			}
			return "no such file to read.\n";
			
		}else if((strncmp(StrWrite,ptr,5)==0)){
			char* filename = strtok(NULL,delim);
			char* write = strtok(NULL,delim);
			for(int i = 0;i<10;i++){
				if(strcmp(file_list[i],filename)==0){
					fptr=fopen(filename,"a");
					fputs(write,fptr);
					fclose(fptr);
					return "wrote succesfuly\n";
					
				}
			}
			return "no such file to write.\n";
			
		}else if((strncmp(StrExit,ptr,4)==0)){
			return "terminating";
		}
	}
	
	return "error";
}








