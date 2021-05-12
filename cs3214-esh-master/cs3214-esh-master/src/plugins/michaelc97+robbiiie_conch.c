//  On my honor:                                                                                                                                                                                                               //  //  //  //
//  //  //  //  //  //  - I have not discussed the C language code in my program with
//  //  //  //  //  //    anyone other than my instructor or the teaching assistants
//  //  //  //  //  //    assigned to this course.
//  //  //  //  //  //
//  //  //  //  //  //  - I have not used C language code obtained from another student,
//  //  //  //  //  //    the Internet, or any other unauthorized source, either modified
//  //  //  //  //  //    or unmodified.
//  //  //  //  //  //
//  //  //  //  //  //  - If any C language code or documentation used in my program
//  //  //  //  //  //    was obtained from an authorized source, such as a text book or
//  //  //  //  //  //    course notes, that has been clearly noted with a proper citation
//  //  //  //  //  //    in the comments of my program.
//  //  //  //  //  //
//  //  //  //  //  //  - I have not designed this program in such a way as to defeat or
//  //  //  //  //  //    interfere with the normal operation of the grading code.
//  //  //  //  //  //
//  //  //  //  //  //    Name: Robert Andrews
//  //  //  //  //  //    PID: robbiiie
//  //  //  //  //  //                                                                                                                                                                                                                 //  //  //  //    Name: Michael Cheung                                                                                                                                                                                         //  //  //  //    PID: michaelc97 

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../esh.h"
#include <string.h>

static bool    
init_plugin(struct esh_shell *shell)
{
    printf("Plugin 'conch' initialized...\n");
        return true;
}

static bool conch(struct esh_command *com)
{
	if (strcmp(com->argv[0], "conch")){
		return 0;
	}
	
	if (com->argv[1] == NULL) {
		printf("You must enter a question for the magic conch to answer\n");
		return 0;
	}
	
	int a = 1;
	char *p = com->argv[a];
	int num = 0;
	while(p)
	{
		num = num + strlen(p);
		a++;
		p = com->argv[a];
	}

	//printf("%d\n", num);	
	if(num > 41)
	{
		printf("Maybe someday.\n");
	}
	else if(num > 30)
	{
		printf("Nothing.\n");
	}
	else if(num > 24)
	{
		printf("Neither.\n");
	}
	else if(num > 20)
	{
		printf("I don't think so.\n");
	}
	else if(num > 13)
	{
		printf("No.\n");
	}
	else if(num > 5)
	{
		printf("Yes.\n");
	}
	else
	{
		printf("Try asking again.\n");
	}

	return false;
}
                                                                                                                                   
struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = conch
};
                                                                                                                                   
                    
