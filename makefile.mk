all: file_manager file_client
file_manager: file_manager.c
	gcc file_manager.c -o m -lpthread
	
file_client: file_client.c
	gcc file_client.c -o c -lpthread
	
