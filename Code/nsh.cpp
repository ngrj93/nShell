/*--------------------------------------------------
@author - Nagaraj Poti   @rollno - 20162010
--------------------------------------------------*/
#ifndef GLOBAL_H
#define GLOBAL_H
#include "global.h"
#endif

#ifndef SHELL_H
#define SHELL_H
#include "shell.h"
#endif

using namespace std;

int main(){
	signal(SIGINT,Shell::ctrlc_handler);  	//ctrl+c
	signal(SIGTSTP,Shell::ctrlz_handler); 	//ctrl+z
	Shell s;  				//shell application object
	s.fout.open("history",ios::out|ios::app);	//history file initialization
	s.fin.open("history",ios::in);
	s.populate_history();
	s.menu();	//shell user menu
	return 0;
}
