/*--------------------------------------------------
@author - Nagaraj Poti   @rollno - 20162010
--------------------------------------------------*/
#ifndef SHELL_H
#define SHELL_H
#include "shell.h"
#endif

//foreground a suspended process
void Shell::foreground(){
	kill(-1,SIGCONT); //send SIGCONT to child processes
}

//export command
void Shell::export_func(char *argv[MAXARGS]){
	int i=1;
	while(argv[i]!=0){
		if(!strcmp(argv[i],"=")){
			setenv(argv[i-1],argv[i+1],1); //parse x=5 like strings 
			env.push_back(string(argv[i-1])); 
			i++;
		}
		i++;
	}
}

//echo command
void Shell::echo_func(char *argv[MAXARGS]){
	int i=1;
	while(argv[i]!=0){
		if(!strcmp(argv[i],"\"") || !strcmp(argv[i],"\'")){ //quotation mark checking 
			char *temp=argv[i];
			i++;
			while(argv[i]!=0 && argv[i]!=temp)
				cout<<argv[i++]<<" ";
		}
		else if(!strcmp(argv[i],"$")){ //check for exported variables $
			i++;
			char *temp=getenv(argv[i++]);
			if(temp!=NULL)
				cout<<temp<<" ";
		}
		else
			cout<<argv[i++]<<" ";
	}
	cout<<endl;
}

//cd command
void Shell::cd_func(char *argv[MAXARGS]){
	char * temp;
	if(argv[1]!=0){
		if(!strcmp(argv[1],"~"))   //cd ~
			temp=getenv("HOME");		
		else if(!strcmp(argv[1],"-")) //cd -
			temp=prev_pwd;
		else
			temp=argv[1]; //cd path
		if(chdir(temp)!=-1){  
			prev_pwd=pwd;
			pwd=getcwd(NULL,0);
			setenv("PWD",pwd,1); //set pwd to current directory
		}
		else
			perror(argv[0]);
	}
}

//pwd command
void Shell::pwd_func(char *argv[MAXARGS]){
	cout<<pwd<<endl;
}

//history command
void Shell::history_func(char *argv[MAXARGS]){
	int count=50;  //display last 50 records
	if(argv[1]!=0)
		count=atoi(argv[1]);  //display n records entered by user
	unsigned int i=((signed int)history.size()-count)<0 ? 0 : (signed int)history.size()-count;
	for(;i<history.size();i++)
		cout<<i<<" "<<history[i]<<endl;
}

//! comand 
void Shell::bang_func(char *argv[MAXARGS]){
	string s;
	int i=1,cur=history.size()-1;
	if(argv[i]!=0){
		if(strcmp(argv[i],"!")){  //!!
			char *e;
			int num=strtol(argv[i],&e,10); //check for ! followed by number
			if(e==argv[i]||*e!='\0'){ //if NaN
				s=string(argv[i++]);
				while(argv[i]!=0)
					s=" "+string(argv[i++]);
				for(int j=history.size()-1;j>=0;j--){
					if(history[j].find(s)==0){
						cur=j;
						break;
					}
				}
			}
			else{  //if a number
				if(num<0) //if negative number
					cur=history.size()+num;
				else if(num<(signed int)history.size())
					cur=num;
			}
		}
	}
	cmd.clear();
	if(strcmp(argv[1],"!")){
		fout<<history[cur]<<endl;  //update history with command 
		history.push_back(history[cur]);
	}
	first_parse(history[cur]);  //parse and execute command
}

