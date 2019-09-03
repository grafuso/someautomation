#include <iostream>
#include <string>
#include <fstream>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <string.h>
#include <fcntl.h>

#define TEMPERATURE_DIR "/sys/bus/w1/devices/"
#define TEMP_FILE "/w1_slave"
#define FIFO_TEMP "MYTEMP"

using namespace std;


int main(void){

	int pipe, num, celsius = 0;
	char * fifo;
	size_t pos = 0;
	string line,path,fname;
	DIR* dr = opendir(TEMPERATURE_DIR);
	dirent* entry = readdir(dr);

	path.assign(TEMPERATURE_DIR);
	while(entry){
		fname = entry->d_name;	
		if(fname != "." && fname != ".." && fname != "w1_bus_master1")
			path.append(fname);
		entry = readdir(dr);
	}
	
	path.append(TEMP_FILE);
		
	ifstream read(path.c_str());
	if(read){
		pos = 0;

		getline(read,line);
		getline(read,line);
		pos = line.find('=');
		line.erase(0,pos + 1);
	}
	read.close();

	if((pipe = open(FIFO_TEMP,O_WRONLY))<0) {
		perror("open fifo");
		exit(5);
	}
	fifo = (char *) malloc(sizeof(line.c_str())+1);
	strcpy(fifo,line.c_str());
	if((num = write(pipe,fifo,strlen(fifo)) == -1)) {
		perror("error writing fifo");
		exit(5);
	}
	close(pipe);
	exit(0);
}
