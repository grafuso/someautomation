#include "GPIOClass.h"

#define HEIGHT 6
#define WIDTH 3
#define DEPTH 3
#define ROUNDS 3

using namespace std;

// protos
void flip_bits(int pattern[3][3]);

int ledmatriisi(void)
{
	int i,j,k,l=0,x=0;
	int *dice[6];

	GPIOClass* gpio4 = new GPIOClass("4");
	GPIOClass* gpio17 = new GPIOClass("17");
	GPIOClass* gpio18 = new GPIOClass("18");
	GPIOClass* gpio23 = new GPIOClass("23");
	GPIOClass* gpio24 = new GPIOClass("24");
	GPIOClass* gpio27 = new GPIOClass("27");

	cout << "GPIO pins exported" << endl;

	GPIOClass* cathodes[] = {gpio4, gpio17, gpio27};
	GPIOClass* anodes[] = {gpio18, gpio23, gpio24};

	int one[3][3] = {{1,1,1},
	                 {1,0,1},
		         {1,1,1}};
	int two[3][3] = {{0,1,1},
	                 {1,1,1},
			 {1,1,0}};
	int three[3][3] = {{0,1,1},
	                   {1,0,1},
			   {1,1,0}};
	int four[3][3] = {{0,1,0},
	                  {1,1,1},
			  {0,1,0}};
	int five[3][3] = {{0,1,0},
	                  {1,0,1},
		          {0,1,0}};
	int six[3][3] = {{0,1,0},
	                 {0,1,0},
		         {0,1,0}};

	flip_bits(one);
	flip_bits(two);
	flip_bits(three);
	flip_bits(four);
	flip_bits(five);
	flip_bits(six);

	dice[0] = (int *)one;
	dice[1] = (int *)two;
	dice[2] = (int *)three;
	dice[3] = (int *)four;
	dice[4] = (int *)five;
	dice[5] = (int *)six;

	cout << "Variables set.." << endl;

	for(i=0;i<WIDTH;i++)
	{
		cathodes[i]->setdirection_gpio("out");
		cathodes[i]->setvalue_gpio(0);

		anodes[i]->setdirection_gpio("out");
		anodes[i]->setvalue_gpio(1);
	}

	cout << "GPIO pins directions set to \"out\" and values to 0." << endl;
	cout << "Starting to blink leds!" << endl;

	while(x<ROUNDS)
	{
		for(k=0;k<HEIGHT;k++)
		{
			for(int b=0;b<100;b++)
			{
				for(i=0; i<9;i++)
				{
					if(l>2)
						l=0;

					cathodes[0]->setvalue_gpio(*(dice[k]+sizeof(char)*i));		// 0 3 6
					cathodes[1]->setvalue_gpio(*(dice[k]+sizeof(char)*(++i)));	// 1 4 7
					cathodes[2]->setvalue_gpio(*(dice[k]+sizeof(char)*(++i)));	// 2 5 8

					anodes[l]->setvalue_gpio(0);
					usleep(1000);
					anodes[l]->setvalue_gpio(1);
					l++;
				}
			}
		}
		x++;
	}

	delete gpio4;
	delete gpio17;
	delete gpio18;
	delete gpio23;
	delete gpio24;
	delete gpio27;
	gpio4 = NULL;
	gpio17 = NULL;
	gpio18 = NULL;
	gpio23 = NULL;
	gpio24 = NULL;
	gpio27 = NULL;

	cout << "Exiting..." << endl;

	return 0;
}

void flip_bits(int pattern[3][3])
{
	int i,j;
	for(i=0;i<3;i++)
		for(j=0;j<3;j++)
		{
			if(pattern[i][j] == 1)
				pattern[i][j] = 0;
			else
				pattern[i][j] = 1;
		}
}
