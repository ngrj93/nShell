#ifndef SHELL_H
#define SHELL_H
#include "shell.h"
#endif

//static class member variable definitions
int Shell::child_count=0; 
pid_t Shell::child[MAXCHILD]={0};

//class constructor
Shell::Shell(){
	child_count=0;
	prev_pwd=pwd=getenv("PWD");
}

void Shell::ctrlc_handler(int sig){
	for(int i=0;i<child_count;i++)
		kill(child[i],SIGKILL);  //kill all child processes
	child_count=0;
}

void Shell::ctrlz_handler(int sig){
	for(int i=0;i<child_count;i++)
		kill(child[i],SIGSTOP);  //suspend all child processes
}

//populate vector<int> history with history file contents initially
void Shell::populate_history(){
	string s;
	fin.seekg(0,ios::beg);
	while(getline(fin,s))
			history.push_back(s);
	fin.seekg(0,ios::beg);
}

//execute command string
void Shell::execute(){
	int input_fd=0,output_fd=1,pipefd[2],status;
	unsigned int argc=0;
	while(argc<cmd.size()){
		char *command;  //command to be executed 
		char *argv[MAXARGS];  //command arguments
		memset(argv,0,sizeof(argv));
		if(pipe(pipefd)==-1)
			handle_error(PIPE_FAULT);
		output_fd=pipefd[1];
		int temp_fd=output_fd;
		argc=second_parse(argc,&command,argv,input_fd,output_fd); //parse command string into sub commands
		if(skip){ //skip if invalid command string
			skip=false;
			free_array(argv,MAXARGS);
			continue;
		}
		if(argc>=cmd.size() && temp_fd==output_fd) //check for last sub command in command string
			output_fd=1;
		if(is_shell_builtin(command)){  //check if shell builtin command or not
			int save_in_fd=dup(0),save_out_fd=dup(1); //file descriptor redirection
			dup2(input_fd,0);
			dup2(output_fd,1);
			if(input_fd!=0)
				close(input_fd);
			if(output_fd!=1)
				close(output_fd);
			execute_shell_builtin(command,argv); //execute shell builtin
			dup2(save_in_fd,0);
			dup2(save_out_fd,1);
			if(save_in_fd!=0)
				close(save_in_fd);
			if(save_out_fd!=1)
				close(save_out_fd);
		}
		else if((child[child_count++]=fork())==0){ //fork child process if not shell builtin
			if(background){  //if background option is enabled
				setpgid(0,0);
				cout<<'['<<getpid()<<']'<<endl;
			  freopen("/dev/null", "r", stdin); //redirect input to /dev/null
				if(!strcmp(command,"sleep")){
					free(argv[1]);
					argv[1]=strdup("0");
				}
			}
			else
				dup2(input_fd,0);
			dup2(output_fd,1);
			if(execvp(command,argv)==-1) //execute command
				perror(argv[0]);
		}
		background=false;
		close(pipefd[1]);
		input_fd=pipefd[0];
		free_array(argv,MAXARGS);
		if(argc>=cmd.size())  //command string completely executed
			break;
	}
	cmd.clear();
	waitpid(-1,&status,WUNTRACED); //wait for child process execution to finish
}

//check if entered command is shell builtin or not
bool Shell::is_shell_builtin(char *command){
 if(!strcmp(command,"history")||!strcmp(command,"fg")||!strcmp(command,"!")||
	 !strcmp(command,"exit")||!strcmp(command,"echo")||!strcmp(command,"pwd")||
	 !strcmp(command,"cd")||!strcmp(command,"export")||!strcmp(command,"fg"))
	 return true;
 else
	 return false;
}

//shell builtin execute factory function
void Shell::execute_shell_builtin(char *command,char *argv[MAXARGS]){
	string s=string(command);
	if(s=="history")
		history_func(argv);
	else if(s=="!")
		bang_func(argv);
	else if(s=="exit")
		exit_func();
	else if(s=="cd")
		cd_func(argv);
	else if(s=="pwd")
		pwd_func(argv);
	else if(s=="echo")
		echo_func(argv);
	else if(s=="export")
		export_func(argv);
	else if(s=="fg");
		foreground();
}

//user menu
void Shell::menu(){
	while(1){
		cout<<"nsh>"<<pwd<<":~$ ";
		string input;
		if(std::getline(cin,input)==NULL)
			exit_func();
		if(input.find('!')==string::npos && !input.empty()){
			fout<<input<<endl; //populate history file and vector<string> history
			history.push_back(input);
		}
		first_parse(input); //parse user entered string
	}
}

//program critical error handling function
void Shell::handle_error(int errsv){
	cerr<<"Error: ";
	if(errsv==PIPE_FAULT)
		cerr<<"Pipe could not be established!"<<endl;
	else if(errsv==FILE_OPEN_FAULT)
		cerr<<"History file could not be opened!"<<endl;
	exit(-1);
}

void Shell::free_array(char *argv[],int count){ //memory cleanup 
	for(int i=0;i<count;i++)
		free(argv[i]);
}

void Shell::exit_func(){ 			//graceful exit
	ctrlc_handler(SIGINT);
	fin.close();fout.close();
	for(unsigned int i=0;i<env.size();i++)  //unset set environment variables
		unsetenv(env[i].c_str());
	exit(0);
}
