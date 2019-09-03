#ifndef HOMECONTROLLER_H_
#define HOMECONTROLLER_H_

#include <stdio.h>
#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>

#define DEVICE_ON "*.on"
#define DEVICE_OFF "*.off"
#define PROGRAM_QUIT "*.quit"
#define FILECONTENT "/home/homectrl/filecontent_test.log"

int dirscanner(char const* string,size_t size,bool status);
int readwrite_file(const char* filename);
void free_array(char** array, size_t size);
int filemask(char* filename,const char* filemask);
char *deep_cpycat(char const* string,const char* string2);
char *deep_copy(const char* string);
void delete_files(int counter, int file_count,char* string1,const char* string2,char **string3);

struct dirent *drnt;
struct stat file_stat;

void delete_files(int counter, int file_count,char* string1,const char* string2,char **string3){
	for(counter=0;counter<file_count;counter++){
		string1=deep_cpycat(string2,string3[counter]);
		if(unlink(string1))
			printf("Removing file %s failed! Manual removal needed!\n",string3);
		free(string1);
	}
}

int readwrite_file(const char* filename){
	char * buffer = 0;
	long length;
	FILE * f = fopen (filename, "rb");

	if (f){
		fseek (f, 0, SEEK_END);
		length = ftell (f);
		fseek (f, 0, SEEK_SET);
		buffer = (char *)  malloc(length);
		if (buffer)
			fread(buffer,1,length,f);
		fclose (f);
	}
	if(buffer){
		f = fopen (FILECONTENT, "w");

		if (f==NULL){
			free(buffer);
			return 1;
		}else{
			fwrite(buffer,1,length,f);
			fclose(f);
		}
	}else{
		free(buffer);
		return 1;
	}
	free(buffer);
	return 0;
}

char *deep_cpycat(const char* string,const char* string2){
        char *new_string;
        new_string =(char *) malloc(strlen(string)+strlen(string2)+1);
        strcpy(new_string,string);
	strcat(new_string,string2);
        return new_string;
}

char *deep_copy(const char* string){
	char *new_string;
	new_string = (char *) malloc(strlen(string)+1);
	strcpy(new_string,string);
	return new_string;
}

int filemask(char* filename,const char* device_stat){
        const char *filename_ldot;
        const char *device_stat_check;
        filename_ldot = strrchr(filename,'.');
        if(filename_ldot==NULL)
                return 0;
        device_stat_check=strchr(device_stat,'.');
	if(device_stat_check==NULL)
		return 0;
	if(strcmp(++device_stat_check,++filename_ldot)!=0)
		return 0;
	else{
		return 1;
	}
        return 0;
}

void free_array(char** array, size_t size){
	size_t c;
	for(c=0;c<size;c++){
		free(array[c]);
		array[c]=NULL;
	}
	free(array);
	array=NULL;
}
 
int dirscanner(const char* string,size_t size,bool status){
	int i=0,j;
	char **files=NULL;
	DIR *dr;
	char *tmp_string;
	int dev_on_flag=0,dev_off_flag=0,prog_quit_flag=0;
	bool on_found=false, off_found=false, quit_found=false;

	dr=opendir(string);
	if(dr==NULL){
			printf("Cannot open directory!\n");
			return -1;
	}
	while((drnt=readdir(dr))!=NULL){
		if(drnt->d_name[0]=='.'){
			continue;
		}
		files=(char **)realloc(files, (i+1) * sizeof(*files));
		if(files==NULL){
			free_array(files,i);
			return -1;
		}
		files[i]=(char *)malloc((strlen(drnt->d_name)+1)*sizeof(char));
		if(files[i]==NULL){
			free_array(files,i);
			return -1;
		}
		strcpy(files[i],drnt->d_name);
		i++;
	}
	closedir(dr);
	for(j=0;j<i;j++){
		tmp_string=deep_cpycat(string,files[j]);
		if(stat(tmp_string,&file_stat) < 0){
			printf("\nCan't execute stat() for: %s\n",files[j]);
			free(tmp_string);
			free_array(files,i);
			return -1;
		}
		else{
			if(!on_found)
				dev_on_flag=filemask(files[j],DEVICE_ON);
			if(dev_on_flag && !on_found){
				readwrite_file(tmp_string);
				on_found=true;
			}
			if(!off_found)
				dev_off_flag=filemask(files[j],DEVICE_OFF);
			if(dev_off_flag && !off_found)
				off_found=true;
			if(!quit_found)
				prog_quit_flag=filemask(files[j],PROGRAM_QUIT);
			if(prog_quit_flag && !quit_found)
				quit_found=true;
			free(tmp_string);
		}
	}
	if(prog_quit_flag){
		delete_files(j,i,tmp_string,string,files);
		free_array(files,i);
		puts("Scanner program will be shutdown!");
		puts("Device will be shutdown!");
		puts("All files will be removed from directory!");
		return -1;
	}
	if(dev_off_flag){
	/* Jos molemmat on ja off file ovat hakemistossa laite suljetaan!
	   Molemmat tiedostot myös poistetaan.*/
		delete_files(j,i,tmp_string,string,files);	
		free_array(files,i);
		return 0;
	}
	if(!dev_off_flag && dev_on_flag){
	/* Jos off fileä ei ole hakemistossa ja on file on laite kytketään päälle!
	   Tiedostot poistetaan! */
		delete_files(j,i,tmp_string,string,files);
		free_array(files,i);
		return 1;
	}
	/* Kaikissa muissa tapauksissa laite suljetaan ja kaiki tiedostot poistetaan! */
	delete_files(j,i,tmp_string,string,files);	
	free_array(files,i);
	return status;
}
#endif
