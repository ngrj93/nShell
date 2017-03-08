/*--------------------------------------------------
@author - Nagaraj Poti   @rollno - 20162010
--------------------------------------------------*/
#ifndef SHELL_H
#define SHELL_H
#include "shell.h"
#endif

//first parse of user input string
void Shell::first_parse(string arg){
	char *input=strdup(arg.c_str());
	char *input_copy=strdup(input);
	const char *delimiters=" \n\t|<>\'\"&!$="; //delimiters of interest
	char *token=strtok(input,delimiters);
	int j=0,diff=0,size=0;
	//handle multiple consecutive delimiters of interest
	while(input_copy[diff+size+j]!='\0' && check_character(input_copy[diff+size+j])){ 
		if(!isspace(input_copy[diff+size+j])) //ignore spaces
			cmd.push_back(string(1,input_copy[diff+size+j]));
		j++;
	}
	while(token!=NULL){ //tokenization of input string
		diff=token-input;
		size=strlen(token);
		cmd.push_back(string(token));
		j=0;
		//handle multiple consecutive delimiters of interest
		while(input_copy[diff+size+j]!='\0' && check_character(input_copy[diff+size+j])){
			if(!isspace(input_copy[diff+size+j]))
				cmd.push_back(string(1,input_copy[diff+size+j]));
			j++;
		}
		token=strtok(NULL,delimiters);
	}
	execute(); //execution of user input string
}

bool Shell::check_character(char c){
	if(c=='|'||c=='<'||c=='>'||c=='\"'||c=='\''||c==' '||c=='\t'
			||c=='\n'||c=='&'||c=='!'||c=='$'||c=='=')
		return true;
	else
		return false;
}

//second parse of individual sub command strings
int Shell::second_parse(unsigned int argc,char **command,char *argv[MAXARGS],int &input_fd,int &output_fd){
	int count=0;
	while(argc<cmd.size()){
		if(cmd[argc]=="\"" || cmd[argc]=="\'"){ //check for valid quotation marks
			string temp1=cmd[argc++],temp2;
			while(argc<cmd.size() && cmd[argc]!=temp1)
				temp2+=cmd[argc++];
			if(argc>=cmd.size()){
				cerr<<"Invalid quotation marks!"<<endl;
				skip=true; //skip command on error
			}
			if(count==0)
				*command=strdup(temp2.c_str()); //populate command to be executed
			argv[count]=strdup(temp2.c_str()); //populate arguments to command
			count++;
		}
		else if(cmd[argc]==">"){ //output redirection handling
		  argc++;
	    if((output_fd=creat(cmd[argc].c_str(),S_IRWXU))==-1){
		    perror(cmd[argc].c_str());
	      skip=true;
	    }
	  }
	  else if(cmd[argc]=="<"){ //input redirection handling
	    argc++;
		  if((input_fd=open(cmd[argc].c_str(),O_RDONLY))==-1){
		    perror(cmd[argc].c_str());
	      skip=true;
	    }
	  }
	  else if(cmd[argc]=="|"){ //pipe symbol
		  argc++;
	    break;
	  }
	  else if(cmd[argc]=="&" || !strcmp(cmd[argc].c_str(),"bg")) //background symbol 
	    background=true;
   else{
	  	if(count==0)
	    	*command=strdup(cmd[argc].c_str()); //populate command string and arguments
		  argv[count]=strdup(cmd[argc].c_str());
			count++;
		}
   argc++;
	}
  return argc;
}
