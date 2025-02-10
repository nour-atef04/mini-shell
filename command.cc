
/*
 * CS354: Shell project
 *
 * Template file.
 * You will need to add more code here to execute the command table.
 *
 * NOTE: You are responsible for fixing any bugs this code may have!
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <fcntl.h>
#include <unistd.h>
#include <pwd.h>
#include <ctime>
#include <cstring>
#include "command.h"

SimpleCommand::SimpleCommand()
{
	// Creat available space for 5 arguments
	_numberOfAvailableArguments = 5;
	_numberOfArguments = 0;
	_arguments = (char **) malloc( _numberOfAvailableArguments * sizeof( char * ) );
}

void
SimpleCommand::insertArgument( char * argument )
{
	if ( _numberOfAvailableArguments == _numberOfArguments  + 1 ) {
		// Double the available space
		_numberOfAvailableArguments *= 2;
		_arguments = (char **) realloc( _arguments,
				  _numberOfAvailableArguments * sizeof( char * ) );
	}
	
	_arguments[ _numberOfArguments ] = argument;

	// Add NULL argument at the end
	_arguments[ _numberOfArguments + 1] = NULL;
	
	_numberOfArguments++;
}

Command::Command()
{
	// Create available space for one simple command
	_numberOfAvailableSimpleCommands = 1;
	_simpleCommands = (SimpleCommand **)
		malloc( _numberOfSimpleCommands * sizeof( SimpleCommand * ) );

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_appendToFile = 0;
	_pipe = 0;
	_exitShell = 0;
}

void
Command::insertSimpleCommand( SimpleCommand * simpleCommand )
{
	if ( _numberOfAvailableSimpleCommands == _numberOfSimpleCommands ) {
		_numberOfAvailableSimpleCommands *= 2;
		_simpleCommands = (SimpleCommand **) realloc( _simpleCommands,
			 _numberOfAvailableSimpleCommands * sizeof( SimpleCommand * ) );
	}
	
	_simpleCommands[ _numberOfSimpleCommands ] = simpleCommand;
	_numberOfSimpleCommands++;
}

void
Command:: clear()
{
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		for ( int j = 0; j < _simpleCommands[ i ]->_numberOfArguments; j ++ ) {
			free ( _simpleCommands[ i ]->_arguments[ j ] );
		}
		
		free ( _simpleCommands[ i ]->_arguments );
		free ( _simpleCommands[ i ] );
	}

	if ( _outFile ) {
		free( _outFile );
	}

	if ( _inputFile ) {
		free( _inputFile );
	}

	if ( _errFile ) {
		free( _errFile );
	}

	_numberOfSimpleCommands = 0;
	_outFile = 0;
	_inputFile = 0;
	_errFile = 0;
	_background = 0;
	_appendToFile = 0;
	_pipe = 0;
	_exitShell = 0;
}

void
Command::print()
{
	printf("\n\n");
	printf("              COMMAND TABLE                \n");
	printf("\n");
	printf("  #   Simple Commands\n");
	printf("  --- ----------------------------------------------------------\n");
	
	for ( int i = 0; i < _numberOfSimpleCommands; i++ ) {
		printf("  %-3d ", i );
		for ( int j = 0; j < _simpleCommands[i]->_numberOfArguments; j++ ) {
			printf("\"%s\" \t", _simpleCommands[i]->_arguments[ j ] );
		}
	}

	printf( "\n\n" );
	printf( "  Output       Input        Error        Background\n" );
	printf( "  ------------ ------------ ------------ ------------\n" );
	printf( "  %-12s %-12s %-12s %-12s\n", _outFile?_outFile:"default",
		_inputFile?_inputFile:"default", _errFile?_errFile:"default",
		_background?"YES":"NO");
	printf( "\n\n" );
	
}

void handle_child_signal(int sig){
  int writeFlags = O_CREAT | O_WRONLY | O_APPEND;
  int permissions = S_IRUSR | S_IWUSR;
  int output = open("child_log", writeFlags, permissions);
  
  if ( output < 0 ){ // error opening file
    perror("error opening write file");
    exit(2);
  }
  
  time_t t = time(NULL);
  const char* timeStr = ctime(&t);
  char log[256];
  snprintf(log, 256, "Child terminated at: %s", timeStr);
  write(output, log, strlen(log));
  
  close(output);
  
}

void Command::execute() {

    // exit command
    if(_exitShell){
      printf("  Good bye!!\n\n");
      exit(0);
    }

    if (_numberOfSimpleCommands == 0) {
        prompt();
        return;
    }
    
    print();
    
    // cd command
    if(_simpleCommands[0] && !strcmp(_simpleCommands[0]->_arguments[0], "cd")){
      if(!_simpleCommands[0]->_arguments[1]){
        const char *homeDir = getenv("HOME");
        if(!homeDir) perror("home directory not found!");
        else if ( chdir(homeDir) == -1 ) perror("cd failed!");
      }
      else{
        if(chdir(_simpleCommands[0]->_arguments[1]) == -1){
          perror("cd failed");
        }
      }
      clear(); 
      prompt();
      return;
    }

    // save default stdin, stdout, and stderr because we'll modify them later
    int defaultIn = dup(0);
    int defaultOut = dup(1);
    int defaultErr = dup(2);
    
    int input, output, error;
    int fdpipe[2];
    
    if(_inputFile){
      input = open(_inputFile, O_RDONLY);
      if( input < 0 ){ // error opening read file
        perror("error opening read file!");
        exit(2);
        }
    }
    else{ 
      input = defaultIn;
    }
    
    for (int i = 0; i < _numberOfSimpleCommands; i++) {
      
        dup2(input, 0); // redirect standard input
        
        if (i == _numberOfSimpleCommands - 1) {  // output redirection will be in last command
            if (_outFile){
              int writeFlags = O_CREAT | O_WRONLY | ( _appendToFile ? O_APPEND : O_TRUNC );
              int permissions = S_IRUSR | S_IWUSR;
              output = open(_outFile, writeFlags, permissions);
              
              if(_errFile){
                error = output;
                dup2(error, 2); // redirect standard error
              }
              else{
                error = defaultErr;
              }
              
              if ( output < 0 ){ // error opening write file
                perror("error opening write file");
                exit(2);
              }
            }
            else { 
              output = defaultOut;
            }
                                
        } else {  // intermediate pipe between two commands
            if (pipe(fdpipe) == -1){
              perror( "error creating pipe!");
	      exit( 2 );
            }
            output = fdpipe[1];
            input = fdpipe[0];
        }
        
        dup2(output, 1); // redirect standard output
        close(output);

        // forking the process for each command
        pid_t p_id = fork();
        if (p_id == 0) {  // child process
            close(fdpipe[0]);
            close(fdpipe[1]);
            close(defaultIn); 
            close(defaultOut); 
            close(defaultErr);
            execvp(_simpleCommands[i]->_arguments[0], _simpleCommands[i]->_arguments);
            perror("Error executing command");
            exit(1);
        }
        else if ( p_id < 0 ){
          perror("error forking process!");
          exit(2);
        }

        signal(SIGCHLD, handle_child_signal);
        if (!_background) {
            waitpid(p_id, NULL, 0);  // wait if not in background
        }
    }

    // restore original stdin, stdout, and stderr
    dup2(defaultIn, 0);
    dup2(defaultOut, 1);
    dup2(defaultErr, 2);
    close(defaultIn); 
    close(defaultOut); 
    close(defaultErr);
    _outFile = NULL;
    _errFile = NULL; // since error and output are in the same directory
    clear();
    printf("\n");
    prompt();
}


// Shell implementation

void
Command::prompt()
{
	printf("myshell> ");
	fflush(stdout);
}

Command Command::_currentCommand;
SimpleCommand * Command::_currentSimpleCommand;

int yyparse(void);

void handle_ctrlc(int sig){}

int 
main()
{
	signal(SIGINT, handle_ctrlc);
	Command::_currentCommand.prompt();
	yyparse();
	return 0;
}

