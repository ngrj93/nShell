#ifndef HEADER_FILES
#define HEADER_FILES
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
#endif

#ifndef GLOBAL_H
#define GLOBAL_H
#include "global.h"
#endif

using namespace std;

#ifndef SHELL_DEF
#define SHELL_DEF
class Shell{
private:
	//class member variables
	char *pwd;	
	char *prev_pwd;
	vector<string> cmd;
	vector<string> history;
	vector<string> env;
	static pid_t child[MAXCHILD];
	static int child_count;
	
public:
	ifstream fin;  //history file descriptors
	ofstream fout;

	//class member functions	
	Shell();
	static void ctrlc_handler(int);
	static void ctrlz_handler(int);
	void populate_history();
	void first_parse(string);
	bool check_character(char);
	void execute();
	bool is_shell_builtin(char *);
	void execute_shell_builtin(char *,char *[]);
	int second_parse(unsigned int,char **,char *[],int &,int &);
	void foreground();
	void export_func(char *[]);
	void echo_func(char *[]);
	void cd_func(char *[]);
	void pwd_func(char *[]);
	void history_func(char *[]);
	void bang_func(char *[]);
	void menu();
	void handle_error(int);
	void free_array(char *[],int);
	void exit_func();
};
#endif
