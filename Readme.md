Author - Nagaraj Poti 
Roll no - 20162010

Compilation steps - 
	Call make in the directory containing the source files
	Object file to be run - ./nsh

This shell was designed with a focus on minimising lines of code (~380 lines in total) while meeting required specifications

Implemented functionality - 
	All shell commands(ls,cat,vi,grep,etc.)
	Shell builtin commands(echo,cd,export,pwd,history,!,exit,fg,bg)
		echo - export variables($) , quotation marks
		cd - cd ~ , cd -, cd path
		export - multiple variables 
		history - history 4
		! - !!, !-1, !1, !l
		fg - bring suspended process to foreground
		bg - send process to background (works with pipes also)
		& - send process to background (works with pipes also) 
	ctrl-c handling, ctrl-z handling
	input, output redirection
	meta-characters
	
	
