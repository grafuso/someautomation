extern "C" {
#include "homecontroller.h"
#include "sauna.h"
}
#include "ledmatriisi2.h"
#include <fstream>
#include <string>
#include <ctime>
#include <cstdlib>

#define PATH "/home/homectrl/spool/in/device2/queue/"
#define LOGFILE "/home/homectrl/scanner_test.log"
#define DATAFILE "/home/homectrl/filecontent_test.log"
#define FIFO_NAME "MYFIFO"

int logging(string logfile,string s);
int readDataFromFile(string datafile,string logfile);
int readFifo(string name,string logfile);

using namespace std;

int main(){
        int devStatus;
	bool devAlready=false;
	gpioCfgInterfaces(3);

        while(1){
		// BLOCKED UNTILL THERE IS DATA IN FIFO
		if(readFifo(FIFO_NAME,LOGFILE)) {
			devStatus=dirscanner(PATH,strlen(PATH),devStatus);
			if(devStatus==0){
				if(devAlready){
				// Tässä laitetaan laite pois päältä!
				logging(LOGFILE,"Device turned OFF=0");
				devAlready=false;
				}
			}
			if(devStatus==1){
				if(!devAlready){
					// Tässä laitetaan laite päälle!
					logging(LOGFILE,"Device turned ON=1");
					readDataFromFile(DATAFILE,LOGFILE);
					devAlready=true;
				}
			}
			if(devStatus==-1){
				// Tässä sammutetaan koko scanneri ja varmuudeksi myös laite, ettei se jää päälle!
				// Laitteen sammutus ennen break; komentoa
				break;
			}
		}else
			logging(LOGFILE,"Client sent 0. Transfer failed!");
		
        }
        logging(LOGFILE,"Exiting... quiting all processes!");
	return 0;
}

int logging(string logfile, string s){

	time_t now = time(0);
	tm *stamp = localtime(&now);
	string result = asctime(stamp);
	result = result.replace(result.length()-1,1," ");

	ofstream outputFile (logfile.c_str(),ios::app);
	result.append(s);
	outputFile << result << endl;
	outputFile.close();

	return 0;
}

int readDataFromFile(string datafile,string logfile){
/* This function assumes that PARAMETERS in *.on file are static */
/* sauna_on_time uses GPIO pin 4 */
/* heat_sauna uses GPIO pin 11 */
	string line,parameter;
	size_t pos = 0;

	ifstream filetoread(datafile.c_str());	
	
	if(filetoread.is_open()){
		while(getline(filetoread,line)){
			pos = line.find('=');
			parameter = line.substr(0,pos);
			line.erase(0,pos + 1);
			if(parameter == "SAUNA_HEAT" && !line.empty()){
				if(sauna_heat(atoi(line.c_str())))
					logging(logfile.c_str(),"WARNING: Check your values for servo!");
				else{
					line.append(" astetta asetettu saunaan! (SAUNA_HEAT)");
					logging(logfile.c_str(),line);
				}
			}
			else if(parameter == "SAUNA_ON_TIME" && !line.empty()){
				//if(sauna_on_time(atoi(line.c_str())))
				//	logging(logfile.c_str(),"ERROR: Servo failure!");	
				//else{
					line.append(" minuuttia saunan lämpeämiseen. Servo ei vielä kytketty. (SAUNA_ON_TIME)");
					logging(logfile.c_str(),line);
				//}
			}
			else if(parameter == "SAUNA_TIMER" && !line.empty()){
				//if(sauna_timer(atoi(line.c_str()))))
					//logging(logfile.c_str(),"ERROR: Servo failure!");
				//else
					logging(logfile.c_str(),"Tätä ominaisuutta ei ole vielä tehty.(SAUNA_TIMER)");
			}
			//else
			//	logging(logfile.c_str(),line);
		}	
		filetoread.close();
	}
	else
		logging(logfile.c_str(), "Unable to open file for reading!");

	return 0;
}

int readFifo(string name,string logfile) {
	int pipe,num;
	char c[32];
	

	if((pipe = open(name.c_str(),O_RDONLY))<0) {
		logging(logfile.c_str(), "Unable to open fifo!");
		return 0;
	}
	if((num = read(pipe,c,sizeof(c))) < 0) {
		logging(logfile.c_str(), "Unable to read fifo!");
		return 0;
	}
	close(pipe);
	return atoi(c);
}
