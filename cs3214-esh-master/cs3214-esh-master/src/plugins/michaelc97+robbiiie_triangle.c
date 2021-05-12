//  On my honor:                                                                                                                                                                                                               //  //  //  //
//  //  //  //  //  - I have not discussed the C language code in my program with
//  //  //  //  //    anyone other than my instructor or the teaching assistants
//  //  //  //  //    assigned to this course.
//  //  //  //  //
//  //  //  //  //  - I have not used C language code obtained from another student,
//  //  //  //  //    the Internet, or any other unauthorized source, either modified
//  //  //  //  //    or unmodified.
//  //  //  //  //
//  //  //  //  //  - If any C language code or documentation used in my program
//  //  //  //  //    was obtained from an authorized source, such as a text book or
//  //  //  //  //    course notes, that has been clearly noted with a proper citation
//  //  //  //  //    in the comments of my program.
//  //  //  //  //
//  //  //  //  //  - I have not designed this program in such a way as to defeat or
//  //  //  //  //    interfere with the normal operation of the grading code.
//  //  //  //  //
//  //  //  //  //    Name: Robert Andrews
//  //  //  //  //    PID: robbiiie
//  //  //  //  //                                                                                                                                                                                                                 //  //  //  //    Name: Michael Cheung                                                                                                                                                                                         //  //  //  //    PID: michaelc97 

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include "../esh.h"

static bool
init_plugin(struct esh_shell *shell)
{
    printf("Plugin 'triangle' initialized...\n");
    return true;
}

/**
 * Method that calculated the area of a triangle based on the supplied
 * height and base values
 */
static int triangle(struct esh_command * com){
	//Checks to make sure this plugin was really called
	if (strcmp(com->argv[0], "triangle")){
		return 0;
	}
	//Checks to see if a height and base have been entered
	if (com->argv[1] == NULL || com->argv[2] == NULL) {
                printf("You must enter a base and height of a triangle\n");
                return 0;
        }
	//Checks that both entered values are positive integers
	if (com->argv[1] < 0 || com->argv[2] < 0){
		printf("Please enter positive values for the base and height\n");
		return 0;
	}
	//Grabs the height
	int height = atoi(com->argv[1]);
	//Grabs the base
	int base = atoi(com->argv[2]);
	//Computes the are
	int area = (height * base) / 2;
	//Prints the result
	printf("Area: %d\n", area);
	return 0;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = triangle
};
