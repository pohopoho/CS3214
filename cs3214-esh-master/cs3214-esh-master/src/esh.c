//  On my honor:
//  //  //  //
//  //  //  //  - I have not discussed the C language code in my program with
//  //  //  //    anyone other than my instructor or the teaching assistants
//  //  //  //    assigned to this course.
//  //  //  //
//  //  //  //  - I have not used C language code obtained from another student,
//  //  //  //    the Internet, or any other unauthorized source, either modified
//  //  //  //    or unmodified.
//  //  //  //
//  //  //  //  - If any C language code or documentation used in my program
//  //  //  //    was obtained from an authorized source, such as a text book or
//  //  //  //    course notes, that has been clearly noted with a proper citation
//  //  //  //    in the comments of my program.
//  //  //  //
//  //  //  //  - I have not designed this program in such a way as to defeat or
//  //  //  //    interfere with the normal operation of the grading code.
//  //  //  //
//  //  //  //    Name: Robert Andrews
//  //  //  //    PID: robbiiie
//  //	//  //
//  //	//  //	  Name: Michael Cheung
//  //	//  //    PID: michaelc97

/*
 * esh - the 'pluggable' shell.
 *
 * Developed by Godmar Back for CS 3214 Fall 2009
 * Virginia Tech.
 */

//robbiiie@vt.edu
#include <stdio.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>

#include "esh-sys-utils.h"
#include "esh.h"

struct list jList;
struct termios * term;

static void
usage(char *progname)
{
    printf("Usage: %s -h\n"
        " -h            print this help\n"
        " -p  plugindir directory from which to load plug-ins\n",
        progname);

    exit(EXIT_SUCCESS);
}

/* Build a prompt by assembling fragments from loaded plugins that 
 * implement 'make_prompt.'
 *
 * This function demonstrates how to iterate over all loaded plugins.
 */
static char *
build_prompt_from_plugins(void)
{
    char *prompt = NULL;

    for (struct list_elem * e = list_begin(&esh_plugin_list);
         e != list_end(&esh_plugin_list); e = list_next(e)) {
        struct esh_plugin *plugin = list_entry(e, struct esh_plugin, elem);

        if (plugin->make_prompt == NULL)
            continue;

        /* append prompt fragment created by plug-in */
        char * p = plugin->make_prompt();
        if (prompt == NULL) {
            prompt = p;
        } else {
            prompt = realloc(prompt, strlen(prompt) + strlen(p) + 1);
            strcat(prompt, p);
            free(p);
        }
    }

    /* default prompt */
    if (prompt == NULL)
        prompt = strdup("esh> ");

    return prompt;
}

/**
 *  *  * Assign ownership of ther terminal to process group
 *      * pgrp, restoring its terminal state if provided.
 *       *
 *    *     * Before printing a new prompt, the shell should
 *      *      * invoke this function with its own process group
 *       *       * id (obtained on startup via getpgrp()) and a
 *        *        * sane terminal state (obtained on startup via
 *         *         * esh_sys_tty_init()).
 *          *          */
static void give_terminal_to(pid_t pgrp, struct termios *pg_tty_state)
{
    esh_signal_block(SIGTTOU);
    int rc = tcsetpgrp(esh_sys_tty_getfd(), pgrp);
    if (rc == -1)
        esh_sys_fatal_error("tcsetpgrp: ");

    if (pg_tty_state)
        esh_sys_tty_restore(pg_tty_state);
    esh_signal_unblock(SIGTTOU);
}

/*
 * Helper function to change job status in the jobs list,
 * it searches for the correct child id number in the list, then
 * changes the status based on how it terminated/stopped 
 */
static void child_status_change(pid_t child, int status)
{
	struct list_elem * iter = list_begin(&jList);
	struct list_elem * cmd_iter;
	while(iter != list_end(&jList)) //loop through the jobs list
	{
		struct esh_pipeline * curr_pipe = list_entry(iter, struct esh_pipeline, elem);
		cmd_iter = list_begin(&curr_pipe->commands);
		while(cmd_iter != list_end(&curr_pipe->commands)) //loop through each command in a job pipeline
		{
			if(list_entry(cmd_iter, struct esh_command, elem)->pid == child) //look for the correct process
			{
				if(WIFEXITED(status))		//if process exited normally
				{
					list_remove(cmd_iter);
					give_terminal_to(getpid(), term);
				}
				if(WTERMSIG(status) == 9)	//if process was send SIGKILL via kill command
				{
					list_remove(cmd_iter);
					give_terminal_to(getpid(), term);
				}
				if(WIFSTOPPED(status))		//if process was sent SIGSTOP via stop command or ctrl-Z
				{
					curr_pipe->status = STOPPED;
					printf("\n[%d]+ Stopped\t(", curr_pipe->jid);
					int a = 0;
					char * p = list_entry(cmd_iter, struct esh_command, elem)->argv[a];
					
					while(p)
					{
						printf("%s ", p);
						a++;
						p = list_entry(cmd_iter, struct esh_command, elem)->argv[a];
					}
					printf(")\n");
					esh_sys_tty_save(&curr_pipe->saved_tty_state);
					give_terminal_to(getpid(), term);	
				}
				if(WTERMSIG(status) == 2)	//if process was sent SIGINT via ctrl-C
				{
					list_remove(cmd_iter);
					printf("\n");
					give_terminal_to(getpid(), term);
				}
				if(list_empty(&curr_pipe->commands))	//remove job from jobs list if status was changed correctly
				{
					//printf("list is empty\n");	//DEBUG
					list_remove(iter);		
				}
			}
			cmd_iter = list_next(cmd_iter);	
		}
		//printf("Exited inner while\n");	//DEBUG
		iter = list_next(iter);
	}
}
/* Wait for all processes in this pipeline to complete, or for
 *  * the pipeline's process group to no longer be the foreground
 *   * process group.
 *    * You should call this function from a) where you wait for
 *     * jobs started without the &; and b) where you implement the
 *      * 'fg' command.
 *       *
 *        * Implement child_status_change such that it records the
 *         * information obtained from waitpid() for pid 'child.'
 *          * If a child has exited or terminated (but not stopped!)
 *           * it should be removed from the list of commands of its
 *            * pipeline data structure so that an empty list is obtained
 *             * if all processes that are part of a pipeline have
 *              * terminated.  If you use a different approach to keep
 *               * track of commands, adjust the code accordingly.
 *                */
static void wait_for_job(struct esh_pipeline *pipeline)
{
    assert(esh_signal_is_blocked(SIGCHLD));
    //printf("entered wait_for_job\n");			//DEBUG
    while (pipeline->status == FOREGROUND && !list_empty(&pipeline->commands)) {
        int status;

        pid_t child = waitpid(-1, &status, WUNTRACED);
        if (child != -1)
            child_status_change(child, status);
    }
	
    //printf("hello\n");
}

/*
 *  * SIGCHLD handler.
 *   * Call waitpid() to learn about any child processes that
 *    * have exited or changed status (been stopped, needed the
 *     * terminal, etc.)
 *      * Just record the information by updating the job list
 *       * data structures.  Since the call may be spurious (e.g.
 *        * an already pending SIGCHLD is delivered even though
 *         * a foreground process was already reaped), ignore when
 *          * waitpid returns -1.
 *           * Use a loop with WNOHANG since only a single SIGCHLD
 *            * signal may be delivered for multiple children that have
 *             * exited.
 *              */
static void handler(int sig, siginfo_t *info, void * _ctxt)
{
    pid_t child;
    int status;

    assert(sig == SIGCHLD);

    while ((child = waitpid(-1, &status, WUNTRACED|WNOHANG)) > 0) {
        child_status_change(child, status);
    }
}

/*
 * Helper method for finding a job in the jobs list by giving searching for the job id
 * basic search function that loops through a list and finds the desired job
 */ 
static struct esh_pipeline * get_job(int jId)
{
	struct list_elem * it = list_begin(&jList);
	while(it != list_end(&jList))
	{
		if(list_entry(it, struct esh_pipeline, elem)->jid == jId)
		{
			return list_entry(it, struct esh_pipeline, elem); 	
		}
		it = list_next(it);
	}
	return NULL;
}
            

/* The shell object plugins use.
 * Some methods are set to defaults.
 */
struct esh_shell shell =
{
    .build_prompt = build_prompt_from_plugins,
    .readline = readline,       /* GNU readline(3) */ 
    .parse_command_line = esh_parse_command_line /* Default parser */
};

/*
 * This is the area where commands, builtins, i/o redir, piping, plugins are handled
 * Our shell has one indefinite loop where we first determine if user input matches a shell plugin, if so the shell handles it.
 * Then the shell checks to see if input is a built in command. The shell parses the builtin and handles it appropriately. Else, 
 * the shell attempts to run the specified process. If unsuccessful, it will return back to the prompt. Our shell has a jobs list
 * that keeps track of all jobs started/running/stopped. Our shell also has a custom SIGCHLD handler.
 */
int
main(int ac, char *av[])
{
    int opt;
    list_init(&esh_plugin_list);

    /* Process command-line arguments. See getopt(3) */
    while ((opt = getopt(ac, av, "hp:")) > 0) {
        switch (opt) {
        case 'h':
            usage(av[0]);
            break;

        case 'p':
            esh_plugin_load_from_directory(optarg);
            break;
        }
    }

    esh_plugin_initialize(&shell);
	
    list_init(&jList);					//create jobs list
    int jobID = 0;
    esh_signal_sethandler(SIGCHLD, handler);    	//set SIGCHLD handler

    term = esh_sys_tty_init();
    give_terminal_to(getpid(), term);

    setpgid(0,0);
    /* Read/eval loop. */
    for (;;) {
        /* Do not output a prompt unless shell's stdin is a terminal */
        char * prompt = isatty(0) ? shell.build_prompt() : NULL;
        char * cmdline = shell.readline(prompt);
        free (prompt);

        if (cmdline == NULL)  /* User typed EOF */
            break;

        struct esh_command_line * cline = shell.parse_command_line(cmdline); //COMMAND
        free (cmdline);
        if (cline == NULL)                  /* Error in command line */
            continue;

        if (list_empty(&cline->pipes)) {    /* User hit enter */	
            esh_command_line_free(cline);
            continue;
        }
	
	struct esh_pipeline * pline = list_entry(list_begin(&cline->pipes), struct esh_pipeline, elem);
	struct esh_command * commandLine = list_entry(list_begin(&pline->commands), struct esh_command, elem);
        
	int plugin = 0;		//guard value for plugins
	struct list_elem * plug_iter = list_begin(&esh_plugin_list);			//handle plugins
	while(plug_iter != list_end(&esh_plugin_list))
	{
		if(list_entry(plug_iter, struct esh_plugin, elem)->process_builtin == NULL)
		{
			plug_iter = list_next(plug_iter);
			plugin = 1;
			continue;
		}
		if(list_entry(plug_iter, struct esh_plugin, elem)->process_builtin(commandLine))
                {
			plug_iter = list_next(plug_iter);
			plugin = 1;
                        continue;
                }
		plug_iter = list_next(plug_iter);
	} 
	if(plugin == 1)				//skip the rest of the for eval loop if its a plugin
	{
		continue;
	}
	if(list_empty(&jList))			//if the job list is empty start counting from 0
	{
		jobID = 0;
	}
	
	struct list_elem * iterator = list_begin (&pline->commands);	//iterator for going through the pipeline/commandline

	pid_t c_pid;			//pid for child forking	
	//esh_signal_block(SIGCHLD);	//DEBUGi
	
	/*
 	 * Handle built-ins
 	 */
	
	if(strcmp(commandLine->argv[0], "jobs") == 0)		//jobs builtin function
	{
		struct list_elem * i = list_begin(&jList);
		struct list_elem * c;
		while(i != list_end(&jList))			//iterate through jobs list and print info
		{
			printf("[%d] ", list_entry(i, struct esh_pipeline, elem)->jid);
			struct esh_pipeline * pipel = list_entry(i, struct esh_pipeline, elem);
			c = list_begin(&pipel->commands);
			while(c != list_end(&pipel->commands))
			{
				if(list_entry(i, struct esh_pipeline, elem)->status == 2)
				{
					printf("Stopped\t");
				}
				else
				{
					printf("Running\t");	
				}
				printf("(");
				int a = 0; 
				struct esh_command * p_cmd = list_entry(c, struct esh_command, elem);
                		char * p = p_cmd->argv[a];
                		while(p)
                		{
                        		printf("%s ", p);
                        		a++;
                        		p = p_cmd->argv[a];
                		}
				printf(")\n");
				c = list_next(c);
			}
			i = list_next(i);
		}
		//printf("jobs\n");				//DEBUG
	}
	else if(strcmp(commandLine->argv[0], "exit") == 0)	//exits the shell
	{
		exit(0);
	}
	else if(strcmp(commandLine->argv[0], "fg") == 0)	//moves a process into the foreground
	{
		esh_signal_block(SIGCHLD);			//block signal because we are accessing data
		//printf("ENTER fg\n");				//DEBUG
		struct esh_pipeline * j = get_job(atoi(commandLine->argv[1]));
		j->status = FOREGROUND;				//change status to foreground
		//printf("FOREGROUNDED\n");			//DEBUG
		give_terminal_to(j->pgrp, term);
		kill(-j->pgrp, SIGCONT);			//sends continue signal to process in case it was stopped
	
                int a = 0; 
                char * p = list_entry(list_begin(&j->commands), struct esh_command, elem)->argv[a];	//prints each command in the
                while(p)										//pipeline 
                {
                	printf("%s ", p);
                	a++;
                	p = list_entry(list_begin(&j->commands), struct esh_command, elem)->argv[a];
                }
	
                //give_terminal_to(j->pgrp, term);		//DEBUG
                printf("\n");
		wait_for_job(j);
		give_terminal_to(getpid(), term);
		//printf("fg\n");				//DEBUG
		esh_signal_unblock(SIGCHLD);			//unblock signal when finished accessing data
	}
	else if(strcmp(commandLine->argv[0], "bg") == 0)	//moves a process into the background
	{
		struct esh_pipeline * j = get_job(atoi(commandLine->argv[1]));
		j->status = BACKGROUND;				//change status to background
		kill(-j->pgrp, SIGCONT);			//sends continue signal 
		//printf("bg\n");				//DEBUG
	}
	else if(strcmp(commandLine->argv[0], "kill") == 0)	//sends SIGKILL to a specific process group
	{
		kill(-get_job(atoi(commandLine->argv[1]))->pgrp, SIGKILL);	//perform a job lookup to send signal to
		//printf("killed job\n");			//DEBUG
	}
	else if(strcmp(commandLine->argv[0], "stop") == 0)	//sends SIGSTOP to a specific process group
	{
		kill(-get_job(atoi(commandLine->argv[1]))->pgrp, SIGSTOP);	//perform a job lookup to seng signal to
		//printf("stop\n");				//DEBUG
	}
	else							//If command is not builtin or plugin
	{
		esh_signal_block(SIGCHLD);			//block SIGCHLD
		bool piping = false;				//piping sentinel
		jobID++;					
		pline->jid = jobID;
		if(list_size(&pline->commands) > 1)		//modify piping sentinel value if necessary
		{
			piping = true;
		}
		int pipe_1[2];
		int pipe_2[2];
	
		while(iterator != list_end(&pline->commands))	//go through each command in the pipeline
		{
			if(piping == true)
			{
				if(iterator != list_rbegin(&pline->commands))		//initialize pipes if necessary
				{
					pipe(pipe_1);
					pipe(pipe_2);
				}
			}
			if((c_pid = fork()) == 0)		//FORK
			{
				//CHILD CODE
				if(iterator == list_begin(&pline->commands))		//set process group for the pipeline if first
				{							//command
					//printf("At the beginning\n");			//DEBUG	
					pline->pgrp = getpid();
				}
				
				list_entry(iterator, struct esh_command, elem)->pid = getpid();	//set pid for command
				
				if(pline->bg_job == true)			//if command has &
				{
					pline->status = BACKGROUND;
					//printf("Backgrounded\n");		//DEBUG
				}
				else						//otherwise put process into fg
				{
					give_terminal_to(pline->pgrp, term);	//give the terminal to the child
					pline->status = FOREGROUND;		
					//give_terminal_to(pline->pgrp, term);	//DEBUG
				}
				if(piping == true)				//If piping is necessary, handle them
				{
					if(iterator != list_begin(&pline->commands))	//pipe for second command
					{
						//printf("not begin\n");
						close(pipe_1[1]);
						dup2(pipe_1[0], 0);
						close(pipe_1[0]);
						close(4);
						close(5);
					}
					if(iterator != list_end(&pline->commands))	//pipe for first command
					{
						//printf("not end\n");
						close(pipe_2[0]);
						dup2(pipe_2[1], 1);
						close(pipe_2[1]);
						close(4);
						close(5);
					}	
				}
				if(list_entry(iterator, struct esh_command, elem)->iored_input != 0)	//if input redir is necessary
				{
					//printf("AFASFSFA\n");		//DEBUG
					struct esh_command * com = list_entry(iterator, struct esh_command, elem);
					int input_fd = open(com->iored_input, O_RDONLY, 666);		//open input descriptor
					com->iored_input = NULL;
					dup2(input_fd, 0);
					close(input_fd);	
				}
				if(list_entry(iterator, struct esh_command, elem)->iored_output != 0)	//if output redir is necessary
				{
					//printf("TEST\n");		//DEBUG
					struct esh_command * com = list_entry(iterator, struct esh_command, elem);
					int output_fd;
					if(com->append_to_output)
					{
						//printf("ASFSAFAFSASF\n");	//DEBUG
						//if appending to a file, open descriptor with appropriate tags
						output_fd = open(com->iored_output, O_CREAT | O_APPEND | O_WRONLY, 0666);	
					}
					else
					{	//if not appending, open descriptor with appropriate tags
						output_fd = open(com->iored_output, O_CREAT | O_WRONLY | O_TRUNC, 0666);
					}
					com->iored_input = NULL;
					dup2(output_fd, 1);
					close(output_fd);
				}
				struct esh_command * ex = list_entry(iterator, struct esh_command, elem); //prepare for exec
				if(execvp(ex->argv[0], ex->argv) == -1)					  //EXEC
				{
					esh_sys_fatal_error("esh: command not found\n");	
				}
			}
			else		
			{	
				//PARENT CODE
				if(iterator == list_begin(&pline->commands))
				{
					pline->pgrp = c_pid;					//set pipeline's group pid if first cmd
				}
				setpgid(c_pid, pline->pgrp);					//set commands pid
				list_entry(iterator, struct esh_command, elem)->pid = c_pid;
				if(pline->bg_job == true)					//if job is backgrounded give terminal
				{								//control back to shell
					pline->status = BACKGROUND;				//change status
					printf("[%d] %d\n", pline->jid, pline->pgrp);
					give_terminal_to(getpid(), term);
				}
				else
				{
					pline->status = FOREGROUND;				//if job is fg, change status
					//printf("[%d] %d\n", pline->jid, pline->pgrp);		//DEBUG
				}
				if(piping == true)						
				{	
					if(iterator != list_end(&pline->commands))		//pipe management in parent code
					{
						//printf("not end\n");				//DEBUG
						pipe_1[0] = pipe_2[0];
						pipe_1[1] = pipe_2[1];
					}	
				}
			}
			if(list_empty(&cline->pipes) == false)					//if command was processed, add to
			{									//remove from pline, add to jobs list
				list_push_back(&jList, list_pop_front(&cline->pipes));
			}
			iterator = list_next(iterator);
			//esh_signal_unblock(SIGCHLD);						//DEBUG
		}
		close(4);									//Close pipes
		close(5);
		close(pipe_1[0]);
		close(pipe_1[1]);
		//printf("about to wait\n");				//DEBUG 
		wait_for_job(pline);					//wait on job
        	give_terminal_to(getpid(), term);			//give terminal back to shell once waiting is done
        	esh_signal_unblock(SIGCHLD);				//unblock SIGCHLD
	}	
	//esh_command_line_print(cline);
        esh_command_line_free(cline);
    }
    return 0;
}

