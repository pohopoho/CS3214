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

static void toBin(struct esh_command * com){
	if (com->argv[1] == NULL) {
		printf("You must enter an integer to be converted to binary");
	}

	int num = atoi(strdup(com->argv[1]));
	int i = 0;
	int arr[4];
	for (i = 0; num > 0; i++) {
		arr[i] = num % 2;
		num = num / 2;
	}
	
	char ans[5];
	for (int r = 0; r < 4; r++) {
		if (arr[r] == 1) {
			strcat(ans, "1");
		}
		else if (arr[r] == 0) {
			strcat(ans, "0");
		}
	}
	
	printf("Number in binary is: %s\n", ans);
}

struct esh_plugin esh_module = {
  .rank = 1,
  .init = init_plugin,
  .process_builtin = toBin
};
