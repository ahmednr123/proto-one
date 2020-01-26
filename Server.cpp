#include <stdio.h> 
#include <netdb.h> 
#include <netinet/in.h> 
#include <stdlib.h> 
#include <string.h> 
#include <sys/socket.h> 
#include <sys/types.h> 

#include <pthread.h>

#define MAX 80 
#define PORT 8080 
#define SA struct sockaddr
#define THREADS 10

// Function designed for chat between client and server. 
void func(void *client_fd) 
{ 
	char buff[MAX]; 
	int n;
    int sockfd = (int) client_fd; 
	
    // infinite loop for chat 
	for (;;) { 
		bzero(buff, MAX); 

		// read the message from client and copy it in buffer 
		read(sockfd, buff, sizeof(buff)); 
		// print buffer which contains the client contents 
		printf("From client: %s\t To client : ", buff); 
		bzero(buff, MAX); 
		n = 0; 
		// copy server message in the buffer 
		while ((buff[n++] = getchar()) != '\n') 
			; 

		// and send that buffer to client 
		write(sockfd, buff, sizeof(buff)); 

		// if msg contains "Exit" then server exit and chat ended. 
		if (strncmp("exit", buff, 4) == 0) { 
			printf("Server Exit...\n"); 
			break; 
		} 
	}

    pthread_exit(NULL);
} 

// Driver function 
int main() 
{ 
	int sockfd, connfd, len; 
    int i = 0;
    int connections = 0, rc;
	struct sockaddr_in servaddr, cli;
    pthread_t thread[THREADS];
    pthread_attr_t attr;

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

	// socket create and verification 
	sockfd = socket(AF_INET, SOCK_STREAM, 0); 
	if (sockfd == -1) { 
		printf("socket creation failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully created. Accpeting multiple connections\n"); 
	bzero(&servaddr, sizeof(servaddr)); 

	// assign IP, PORT 
	servaddr.sin_family = AF_INET; 
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY); 
	servaddr.sin_port = htons(PORT); 

	// Binding newly created socket to given IP and verification 
	if ((bind(sockfd, (SA*)&servaddr, sizeof(servaddr))) != 0) { 
		printf("socket bind failed...\n"); 
		exit(0); 
	} 
	else
		printf("Socket successfully binded..\n"); 

    while (connections < THREADS) {
        // Now server is ready to listen and verification 
        if ((listen(sockfd, 5)) != 0) { 
            printf("Listen failed...\n"); 
            exit(0); 
        } 
        else
            printf("Server listening..\n"); 
        len = sizeof(cli); 

        connfd = accept(sockfd, (SA*)&cli, &len); 
        if (connfd < 0) { 
            printf("server acccept failed...\n"); 
            exit(0); 
        } else
            printf("server acccept the client...\n"); 

        rc = pthread_create(&thread[connections++], &attr, func, (void *)connfd );
        if (rc) {
            printf ("Error:unable to create thread, %d \n", rc);
            exit(-1);
        }
    }

    for (i = 0; i < THREADS; i++) {
        rc = pthread_join(thread[i], NULL);
        if (rc) {
            printf("Error:unable to join, %d", rc);
            exit(-1);
        }
    }
    
	close(sockfd); 
    pthread_exit(NULL);
}