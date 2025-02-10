
/*
 * CS-413 Spring 98
 * shell.y: parser for shell
 *
 * This parser compiles the following grammar:
 *
 *	cmd [arg]* [> filename]
 *
 * you must extend it to understand the complete shell grammar
 *
 */

%token	<string_val> WORD

%token 	NOTOKEN EXIT GREAT NEWLINE LESS APPEND ERROR APPEND_ERROR PIPE BACKGROUND

%union	{
		char   *string_val;
	}

%{
extern "C" 
{
	int yylex();
	void yyerror (char const *s);
}
#define yylex yylex
#include <stdio.h>
#include "command.h"
%}

%%

goal:	
	commands
	;

commands: 
	command
	| commands command 
	;

command: simple_command
        ;

simple_command:	
	piped_commands iomodifier_opt background_opt error_redirection_opt NEWLINE {
		printf("   Yacc: execute command\n");
		Command::_currentCommand.execute();
	}
	| EXIT NEWLINE {
		Command::_currentCommand._exitShell = 1;
		Command::_currentCommand.execute();
	}
	| NEWLINE
	| error NEWLINE {yyerrok;}
	;

piped_commands:
	piped_commands PIPE command_and_args
	| command_and_args
	;

command_and_args:
	command_word arg_list {
		Command::_currentCommand.
			insertSimpleCommand( Command::_currentSimpleCommand );
	}
	;

arg_list:
	arg_list argument
	| /* can be empty */
	;

argument:
	WORD {
               printf("   Yacc: insert argument \"%s\"\n", $1);

	       Command::_currentSimpleCommand->insertArgument( $1 );\
	}
	;

command_word:
	WORD {
               printf("   Yacc: insert command \"%s\"\n", $1);
	       
	       Command::_currentSimpleCommand = new SimpleCommand();
	       Command::_currentSimpleCommand->insertArgument( $1 );
	}
	;
	
background_opt:
	BACKGROUND {
		printf("   Yacc: insert run in background");
		Command::_currentCommand._background = 1;
	}
	|
	;

iomodifier_opt:
	GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
	}
	| LESS WORD {
		printf("   Yacc: insert input \"%s\"\n", $2);
		Command::_currentCommand._inputFile = $2;
	}
	| APPEND WORD {
		printf("   Yacc: insert append output \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._appendToFile = 1;
	}
	| iomodifier_opt GREAT WORD {
		printf("   Yacc: insert output \"%s\"\n", $3);
		Command::_currentCommand._outFile = $3;
		Command::_currentCommand._appendToFile = 0;
	}
	| iomodifier_opt LESS WORD {
		printf("   Yacc: insert input \"%s\"\n", $3);
		Command::_currentCommand._inputFile = $3;
	}
	| iomodifier_opt APPEND WORD {
		printf("   Yacc: insert append output \"%s\"\n", $3);
		Command::_currentCommand._outFile = $3;
		Command::_currentCommand._appendToFile = 1;
	}
	| /* can be empty */ 
	;
	
error_redirection_opt:
	ERROR WORD {
		printf("   Yacc: insert output error \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._errFile = $2;
	}
	| APPEND_ERROR WORD {
		printf("   Yacc: insert append error \"%s\"\n", $2);
		Command::_currentCommand._outFile = $2;
		Command::_currentCommand._errFile = $2;
		Command::_currentCommand._appendToFile = 1;
	}
	|
	;

%%

void
yyerror(const char * s)
{
	fprintf(stderr,"%s", s);
}

#if 0
main()
{
	yyparse();
}
#endif
