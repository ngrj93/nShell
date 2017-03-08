#include<bits/stdc++.h>
#include<cctype>
#include<errno.h>
#include<sys/types.h>
#include<fcntl.h>
#include<unistd.h>
#include<sys/wait.h>
#include<signal.h>

#define PIPE_FAULT 1001
#define FILE_OPEN_FAULT 1002
#define MAXARGS 64
#define MAXCHILD 1000

using namespace std;

bool background=false;
bool skip=false;

class Shell{
private:
	char *pwd;
	char *prev_pwd;
	vector<string> cmd;
	vector<string> history;
	vector<string> env;
	static pid_t child[MAXCHILD];
	static int child_count;

public:
	ifstream fin;
	ofstream fout;

	Shell(){
		child_count=0;
		prev_pwd=pwd=getenv("PWD");
	}

	static void ctrlc_handler(int sig){
		for(int i=0;i<child_count;i++)
			kill(child[i],SIGKILL);
		child_count=0;
	}

	static void ctrlz_handler(int sig){
		for(int i=0;i<child_count;i++)
			kill(child[i],SIGSTOP); 
	}

	void populate_history(){
		string s;
		fin.seekg(0,ios::beg);
		while(getline(fin,s))
				history.push_back(s);
		fin.seekg(0,ios::beg);
	}

	void first_parse(string arg){
		char *input=strdup(arg.c_str());
		char *input_copy=strdup(input);
		const char *delimiters=" \n\t|<>\'\"&!$=";
		char *token=strtok(input,delimiters);
		int j=0,diff=0,size=0;
		while(input_copy[diff+size+j]!='\0' && check_character(input_copy[diff+size+j])){
			if(!isspace(input_copy[diff+size+j]))
				cmd.push_back(string(1,input_copy[diff+size+j]));
			j++;
		}
		while(token!=NULL){
			diff=token-input;
			size=strlen(token);
			cmd.push_back(string(token));
			j=0;
			while(input_copy[diff+size+j]!='\0' && check_character(input_copy[diff+size+j])){
				if(!isspace(input_copy[diff+size+j]))
					cmd.push_back(string(1,input_copy[diff+size+j]));
				j++;
			}
			token=strtok(NULL,delimiters);
		}
		execute();
	}

	bool check_character(char c){
		if(c=='|'||c=='<'||c=='>'||c=='\"'||c=='\''||c==' '||c=='\t'
				||c=='\n'||c=='&'||c=='!'||c=='$'||c=='=')
			return true;
		else
			return false;
	}

	void execute(){
		int input_fd=0,output_fd=1,pipefd[2],status;
		unsigned int argc=0;
		while(argc<cmd.size()){
			char *command;
			char *argv[MAXARGS];
			memset(argv,0,sizeof(argv));
			if(pipe(pipefd)==-1)
				handle_error(PIPE_FAULT);
			output_fd=pipefd[1];
			int temp_fd=output_fd;
			argc=second_parse(argc,&command,argv,input_fd,output_fd);
			if(skip){
				skip=false;
				free_array(argv,MAXARGS);
				continue;
			}
			if(argc>=cmd.size() && temp_fd==output_fd)
				output_fd=1;
			if(is_shell_builtin(command)){
				int save_in_fd=dup(0),save_out_fd=dup(1);
				dup2(input_fd,0);
				dup2(output_fd,1);
				if(input_fd!=0)
					close(input_fd);
				if(output_fd!=1)
					close(output_fd);
				execute_shell_builtin(command,argv);
				dup2(save_in_fd,0);
				dup2(save_out_fd,1);
				if(save_in_fd!=0)
					close(save_in_fd);
				if(save_out_fd!=1)
					close(save_out_fd);
			}
			else if((child[child_count++]=fork())==0){
				if(background){
					setpgid(0,0);
					cout<<'['<<getpid()<<']'<<endl;
				  freopen("/dev/null", "r", stdin);
					if(!strcmp(command,"sleep")){
						free(argv[1]);
						argv[1]=strdup("0");
					}
				}
				else
					dup2(input_fd,0);
				dup2(output_fd,1);
				if(execvp(command,argv)==-1)
					perror(argv[0]);
			}
			background=false;
			close(pipefd[1]);
			input_fd=pipefd[0];
			free_array(argv,MAXARGS);
			if(argc>=cmd.size())
				break;
		}
		cmd.clear();
		waitpid(-1,&status,WUNTRACED);
	}

	bool is_shell_builtin(char *command){
	 if(!strcmp(command,"history")||!strcmp(command,"fg")||!strcmp(command,"!")||
		 !strcmp(command,"exit")||!strcmp(command,"echo")||!strcmp(command,"pwd")||
		 !strcmp(command,"cd")||!strcmp(command,"export")||!strcmp(command,"fg"))
		 return true;
	 else
		 return false;
	}

	void execute_shell_builtin(char *command,char *argv[MAXARGS]){
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

	int second_parse(unsigned int argc,char **command,char *argv[MAXARGS],int &input_fd,int &output_fd){
		int count=0;
		while(argc<cmd.size()){
			if(cmd[argc]=="\"" || cmd[argc]=="\'"){
				string temp1=cmd[argc++],temp2;
				while(argc<cmd.size() && cmd[argc]!=temp1)
					temp2+=cmd[argc++];
				if(argc>=cmd.size()){
					cerr<<"Invalid quotation marks!"<<endl;
					skip=true;
				}
				if(count==0)
					*command=strdup(temp2.c_str());
				argv[count]=strdup(temp2.c_str());
				count++;
			}
			else if(cmd[argc]==">"){
			  argc++;
		    if((output_fd=creat(cmd[argc].c_str(),S_IRWXU))==-1){
			    perror(cmd[argc].c_str());
		      skip=true;
		    }
		  }
		  else if(cmd[argc]=="<"){
		    argc++;
			  if((input_fd=open(cmd[argc].c_str(),O_RDONLY))==-1){
			    perror(cmd[argc].c_str());
		      skip=true;
		    }
		  }
		  else if(cmd[argc]=="|"){
			  argc++;
		    break;
		  }
		  else if(cmd[argc]=="&" || !strcmp(cmd[argc].c_str(),"bg"))
		    background=true;
      else{
		  	if(count==0)
		    	*command=strdup(cmd[argc].c_str());
			  argv[count]=strdup(cmd[argc].c_str());
				count++;
			}
      argc++;
		}
	  return argc;
	}

	void foreground(){
		kill(-1,SIGCONT);
	}

	void export_func(char *argv[MAXARGS]){
		int i=1;
		while(argv[i]!=0){
			if(!strcmp(argv[i],"=")){
				setenv(argv[i-1],argv[i+1],1);
				env.push_back(string(argv[i-1]));
				i++;
			}
			i++;
		}
	}

	void echo_func(char *argv[MAXARGS]){
		int i=1;
		while(argv[i]!=0){
			if(!strcmp(argv[i],"\"") || !strcmp(argv[i],"\'")){
				char *temp=argv[i];
				i++;
				while(argv[i]!=0 && argv[i]!=temp)
					cout<<argv[i++]<<" ";
			}
			else if(!strcmp(argv[i],"$")){
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

	void cd_func(char *argv[MAXARGS]){
		char * temp;
		if(argv[1]!=0){
			if(!strcmp(argv[1],"~"))
				temp=getenv("HOME");		
			else if(!strcmp(argv[1],"-"))
				temp=prev_pwd;
			else
				temp=argv[1];
			if(chdir(temp)!=-1){
				prev_pwd=pwd;
				pwd=getcwd(NULL,0);
				setenv("PWD",pwd,1);
			}
			else
				perror(argv[0]);
		}
	}

	void pwd_func(char *argv[MAXARGS]){
		cout<<pwd<<endl;
	}

	void history_func(char *argv[MAXARGS]){
		int count=50;
		if(argv[1]!=0)
			count=atoi(argv[1]);
		unsigned int i=((signed int)history.size()-count)<0 ? 0 : (signed int)history.size()-count;
		for(;i<history.size();i++)
			cout<<i<<" "<<history[i]<<endl;
	}

	void bang_func(char *argv[MAXARGS]){
		string s;
		int i=1,cur=history.size()-1;
		if(argv[i]!=0){
			if(strcmp(argv[i],"!")){
				char *e;
				int num=strtol(argv[i],&e,10);
				if(e==argv[i]||*e!='\0'){
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
				else{
					if(num<0)
						cur=history.size()+num;
					else if(num<(signed int)history.size())
						cur=num;
				}
			}
		}
		cmd.clear();
		fout<<history[cur]<<endl;
		history.push_back(history[cur]);
		first_parse(history[cur]);
	}

	void menu(){
		while(1){
			cout<<"nsh>"<<pwd<<":~$ ";
			string input;
			if(std::getline(cin,input)==NULL)
				exit_func();
			if(input.find('!')==string::npos && !input.empty()){
				fout<<input<<endl;
				history.push_back(input);
			}
			first_parse(input);
		}
	}
	
	void handle_error(int errsv){
		cerr<<"Error: ";
		if(errsv==PIPE_FAULT)
			cerr<<"Pipe could not be established!"<<endl;
		else if(errsv==FILE_OPEN_FAULT)
			cerr<<"History file could not be opened!"<<endl;
		exit(-1);
	}

	void free_array(char *argv[],int count){
		for(int i=0;i<count;i++)
			free(argv[i]);
	}

	void exit_func(){
		ctrlc_handler(SIGINT);
		fin.close();fout.close();
		for(unsigned int i=0;i<env.size();i++)
			unsetenv(env[i].c_str());
		exit(0);
	}
};

int Shell::child_count=0;
pid_t Shell::child[MAXCHILD]={0};

int main(){
	signal(SIGINT,Shell::ctrlc_handler);
	signal(SIGTSTP,Shell::ctrlz_handler);
	Shell s;
	s.fout.open("history",ios::out|ios::app);
	s.fin.open("history",ios::in);
	s.populate_history();
	s.menu();
	return 0;
}
