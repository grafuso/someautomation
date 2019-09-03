#ifndef _GPIO_CLASS_H
#define GPIO_CLASS_H

#include <string>
#include <iostream>
#include <sstream>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

using namespace std;

class GPIOClass
{
	public:
		GPIOClass(string gnum="4"); // Create a GPIO object that controls GPIOx (if empty will control gpio4 pin)
		~GPIOClass();
		int setdirection_gpio(string direction); // sets GPIO 'in' or 'out'
		int setvalue_gpio(int val); // sets GPIO value
		int getvalue_gpio(string& value); // get GPIO value
		string get_gpionum(); // return the GPIO number associated with the instance of an object
	private:
		int export_gpio();
		int unexport_gpio();

		string gpionum; // GPIO number associated with the instance of an object
		int valuefd;
        	int directionfd;
        	int exportfd;
        	int unexportfd;
};

GPIOClass::GPIOClass(string gnum): valuefd(-1), directionfd(-1), exportfd(-1), unexportfd(-1)
{
	this->gpionum = gnum; // sets GPIOClass object for GPIO pin number "gnum"
	this->export_gpio();

}

GPIOClass::~GPIOClass()
{
	cout << "Unexporting pins.." << endl;
	this->unexport_gpio();
}

int GPIOClass::export_gpio()
{
	int status = -1;

	try
	{
		string export_str = "/sys/class/gpio/export";
		this->exportfd = status = open(export_str.c_str(), O_WRONLY|O_SYNC); //Open "export" file. Convert C++ string to C string. Required for all Linux pathnames
		if (status < 0)
		{
			perror("OPERATION FAILED: Unable to open export device");
			throw 100;
		}

		stringstream ss;
		ss << this->gpionum;
		string gpio = ss.str();
		status = write(this->exportfd, gpio.c_str(), gpio.length()); // write GPIO number to export file
		if(status < 0)
		{
			perror("could not write to export file");
			throw 101;
		}
		status = close(this->exportfd);
		if(status < 0)
		{
			perror("could not close export device");
			throw 102;
		}
	}
	catch(int e)
	{
		switch(e)
		{
		case 101:
			this->unexport_gpio();
			this->export_gpio();
			cout << "unexporting and continuing to export" << endl;
			break;
		default:
			delete this;
			break;
		}
	}
	return status;
}

int GPIOClass::unexport_gpio()
{
	int status = -1;
	string unexport_str = "/sys/class/gpio/unexport";
	this->unexportfd = status = open(unexport_str.c_str(), O_WRONLY|O_SYNC); //Open "unexport" file. Convert C++ st$
        if (status < 0)
        {
                perror("OPERATION FAILED: Unable to open unexport device");
                exit(1);
        }

        stringstream ss;
        ss << this->gpionum;
        string gpio = ss.str();
        status = write(this->unexportfd, gpio.c_str(), gpio.length()); // write GPIO number to unexport file
        if(status < 0)
        {
                perror("could not write to export file");
                exit(1);
        }
        status = close(this->unexportfd);
        if(status < 0)
        {
                perror("could not close export device");
                exit(1);
        }

        return status;

}

int GPIOClass::setdirection_gpio(string direction)
{
	int status = -1;
	string setdir_str = "/sys/class/gpio/gpio" + this->gpionum + "/direction";

	this->directionfd = status = open(setdir_str.c_str(), O_WRONLY|O_SYNC); // open direction file for gpio
	if (status < 0)
	{
		perror("OPERATION FAILED: Unable to open direction device");
		exit(1);
	}

	if(direction.compare("in") != 0 && direction.compare("out") != 0)
	{
		fprintf(stderr, "Invalid direction value. Should be \"in\" or \"out\".\n");
		exit(1);
	}

	status = write(this->directionfd, direction.c_str(), direction.length());
	if(status < 0)
	{
		perror("could not write to direction device");
		exit(1);
	}

	status = close(this->directionfd);
	if(status < 0)
	{
		perror("could not close direction device");
		exit(1);
	}

	return status;
}

int GPIOClass::setvalue_gpio(int val)
{
        int status = -1;
	string setval_str = "/sys/class/gpio/gpio" + this->gpionum + "/value";
	ostringstream oss;
	oss << val;
	string value = oss.str();

	this->valuefd = status = open(setval_str.c_str(), O_WRONLY|O_SYNC); // open value file for gpio
        if (status < 0)
        {
                perror("OPERATION FAILED: Unable to open value device");
		exit(1);
	}

	if(value.compare("1") != 0 && value.compare("0") != 0)
	{
		fprintf(stderr, "Invalid value. Should be \"1\" or \"0\".\n");
		exit(1);
	}

	status = write(this->valuefd, value.c_str(), value.length());
	if(status < 0)
	{
		perror("could not write to value device");
		exit(1);
	}

	status = close(this->valuefd);
	if(status < 0)
	{
		perror("could not close value device");
		exit(1);
	}

	return status;
}

int GPIOClass::getvalue_gpio(string& value)
{
        string getval_str = "/sys/class/gpio/gpio" + this->gpionum + "/value";
	int status = -1;
	char buff[10];

	this->valuefd = status = open(getval_str.c_str(), O_RDONLY|O_SYNC); // open value file for gpio
        if (status < 0)
        {
                perror("OPERATION FAILED: Unable to open value device");
                exit(1);
        }

	status = read(this->valuefd, &buff, 1);
	if (status < 0)
	{
		perror("could not read GPIO value device");
		exit(1);
	}

	buff[1] = '\0';
	value = string(buff);

	if(value.compare("1") != 0 && value.compare("0") != 0)
	{
		fprintf(stderr, "Invalid value read. Should be \"1\" or \"0\".\n");
		exit(1);
	}

	status = close(this->valuefd);
	if(status < 0)
	{
		perror("could not close GPIO value device");
		exit(1);
	}

	return status;
}

string GPIOClass::get_gpionum()
{
	return this->gpionum;
}
#endif
