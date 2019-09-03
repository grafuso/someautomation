#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <arpa/inet.h>
#include <sys/fcntl.h>

#define FIFO_NAME "MYFIFO"
#define FIFO_TEMP "MYTEMP"
#define OK 1
#define ERR 0

int make_fifo(const char *name);
int write_fifo(const char *name);
int get_temp(const char *name);
int getData(int sockfd);
void sendData(int sockfd,int x);
void error(char *msg);

int main(int argc, char *argv[]) {
	int sockfd, newsockfd, portno = 8888, addrlen;
	char buffer[256];
	struct sockaddr_in address;
	struct timeval tv;
	int n,data,option=1;

	printf("Server running on port #%d\n", portno);

	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error("ERROR opening socket");

	if(setsockopt(sockfd,SOL_SOCKET,SO_REUSEADDR,&option,sizeof(option))<0)
                puts("failed SO_REUSEADDR on sockfd");

	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(portno);

	if(bind(sockfd, (struct sockaddr *) &address,sizeof(address)) < 0)
		error("ERROR on binding");
	listen(sockfd,5);
	addrlen = sizeof(address);

	if(!make_fifo(FIFO_NAME)){
		printf("ERROR creating fifo!\n");
		exit(1);
	}
	while(1){
		printf("Waiting for client...\n");
		if((newsockfd = accept( sockfd, (struct sockaddr *) &address, (socklen_t*) &addrlen)) < 0)
			error("ERROR on accept");
		tv.tv_sec = 5;  // TIMEOUT SERVERILLE TULEVAAN DATAAN
		tv.tv_usec = 0;  // INITOIDAAN JUST IN CASE

		if(setsockopt(newsockfd, SOL_SOCKET, SO_RCVTIMEO, (char *)&tv,sizeof(struct timeval))<0)
			error("ERROR setsockopt");
		printf("New connection from IP: %s PORT: %d\n",inet_ntoa(address.sin_addr), ntohs(address.sin_port));
		while(1) {
			data = getData(newsockfd);
			// TIEDSTO SAAPUNUT SFTP HAKEMISTOON
			if(data==1) {
				// ilmoitetaan scanner prosessille tiedoston saapumisesta pipeen kirjoittamalla
				data = write_fifo(FIFO_NAME);
				sendData(newsockfd,data);
				break;
			}
			if(data < 0) {
				sendData(newsockfd,data);
				break;
			}
			// LÄMPÖTILAN LÄHETYS
			if(data > 1){
				data = get_temp(FIFO_TEMP);
				sendData(newsockfd,data);
				break;
			}
			if(data == 0) {
				// TÄSSÄ VAAN SAMMUTETAAN SOCKET CONNECTION
				//printf("Client closed socket!\n");
				break;
			}
			//printf("Client sent: %d\n", data);
			//data = func(data);
		}
		close(newsockfd);

		// -2 VOIDAAN SAMMUTTAA KOKO SERVERI
		// SAMAN VOISI TEHDÄ KILL PID, joten tämä ehkä turvallisuusriski?
		if (data == -2){
			unlink(FIFO_NAME);
			break;
		}
	}
	return 0;
}
// Tehdään FIFO
int make_fifo(const char *name) {
	if(mkfifo(name,0666)<0) {
		unlink(name);
		if(mkfifo(name,0666)<0)
			return ERR;
	}
	return OK;
}

// Kirjoitetaan pipeen
int write_fifo(const char *name) {
	int pipe,nr_sent;
	if((pipe = open(name,O_WRONLY))<0) 
		perror("open fifo");
	nr_sent = write(pipe,"1",1);
	close(pipe);
	return OK;
}
// Haetaan lämpötila ja palautetaan se inttinä
int get_temp(const char *name) {
	pid_t pid;
	int status,pipe,num;
	char s[300];
	
	switch(pid = fork()) {
	case -1:
        	error("fork");

	case 0:
		execl("/home/pi/C++/give_temp","give_temp",(char *) 0);
		error("exec");

	default:
		make_fifo(name);
		if((pipe = open(name,O_RDONLY))<0)
			perror("open fifo");
		while((num = read(pipe, s, 300)) >0) {
			if (num == -1)
				perror("read fifo");
			else {
				s[num] = '\0';
				printf("tick: read %d bytes: %s\n", num, s);
			}
		}
		close(pipe);
		waitpid(pid,&status,0);
	}
	if(WIFEXITED(status)) {
//		printf("Lapsi exittasi statuksella: %d\n",WEXITSTATUS(status));
		unlink(name);	
	}
	return atoi(s);
}

void error(char *msg){
	perror(  msg );
	exit(1);
}

void sendData(int sockfd,int x){
	int n;

	char buffer[32];
	sprintf(buffer, "%d\n",x);
	if ((n = write(sockfd, buffer, strlen(buffer))) < 0)
	error("ERROR writing to socket");
	buffer[n] = '\0';
}

int getData(int sockfd){
	char buffer[32];
	int n;

	if((n = read(sockfd,buffer,31)) < 0) {
		perror("ERROR reading from socket");
		return -1;
	}
	buffer[n] = '\0';
	return atoi(buffer);
}
