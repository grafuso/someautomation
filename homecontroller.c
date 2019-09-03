/*      DIRSCANNER WITH FILEMASK                */
/*                                              */
/*      Searches files from given directory     */
/*      that matches the filemask.              */
/*		20.6.2014			*/

#include "homecontroller2.h"

struct dirent *drnt;
struct stat file_stat;

void main(void){
	bool status=false;
	
	status=dirscanner(PATH,strlen(PATH),status);

}

 
int dirscanner(char *string,size_t size,bool status){
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
			if(dev_on_flag && !on_found)
				on_found=true;
			if(!off_found)
				dev_off_flag=filemask(files[j],DEVICE_OFF);
			if(dev_off_flag && !off_found)
				off_found=true;
			if(!quit_found)
				prog_quit_flag=filemask(files[j],PROGRAM_QUIT);
			if(prog_quit_flag && !quit_found)
				quit_found=true;
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
		puts("Device will be shutdown! All files removed from directory!");
		return 0;
	}
	if(!dev_off_flag && dev_on_flag){
	/* Jos off fileä ei ole hakemistossa ja on file on laite kytketään päälle!
	   Tiedostot poistetaan! */
		delete_files(j,i,tmp_string,string,files);
		free_array(files,i);
		puts("Device set on! All files will be removed from directory!");
		return 1;
	}
	/* Kaikissa muissa tapauksissa laite suljetaan ja kaiki tiedostot poistetaan! */
	delete_files(j,i,tmp_string,string,files);	
	free_array(files,i);
	puts("Device will remain in its current state! All files will be removed from directory!");
	return status;
}

