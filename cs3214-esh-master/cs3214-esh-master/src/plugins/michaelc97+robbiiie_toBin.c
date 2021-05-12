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
    printf("Plugin 'toBin' initialized...\n");
    return true;
}

/**
 * Returns a binary sting of a number from 0 to 15
 */
static bool toBin(struct esh_command * com){
	
	//Checks to see if toBin was really called
	if (strcmp(com->argv[0], "toBin")){
		return false;
	}
	//Checks to see if a number was typed in with the command
	if (com->argv[1] == NULL) {
		printf("You must enter an integer between 0 and 15 to be converted to binary\n");
		return false;
	}
	//Gets the number from the command line
	int num = atoi(strdup(com->argv[1]));
	//Checks if the number entered is a valid number
	if (num < 0 || num > 15) {
		printf("Please input a number in the range of 0 to 15\n");
		return false;
	}
	//Counter variable
    	int i = 0;
	//Array to store binary values. Set to 20 as a catch later on. Input values can only range from
    	int arr[4];
    	arr[0] = 20;
    	arr[1] = 20;
    	arr[2] = 20;
    	arr[3] = 20;
	//Loop that fills the array with 0s or 1s depending on the binary digit value
    	for (i = 0; num > 0; i++) {
        	arr[i] = num % 2;
        	num = num / 2;
    	}
	//Loop to change all unmodified elements in the array to zero
    	for (int q = 0; q < 4; q++){
        	if (arr[q] == 20){
            		arr[q] = 0;
        	}
    	}
	//Char* for the answer string
    	char ans[5] = "";
	//Loop that creataes a string of 0z and 1z to represent the binary number entered
    	for (int r = 3; r > -1; r--) {
        	if (arr[r] == 1) {
            		strcat(ans, "1");
        	}
        	else if (arr[r] == 0) {
            	strcat(ans, "0");
        	}
    	}
	//Prints the answer
    	printf("Number in binary is: %s\n", ans);
	return true;
	
	//return false;
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = toBin
};
