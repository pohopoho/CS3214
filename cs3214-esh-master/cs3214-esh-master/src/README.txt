Student Information:
--------------------
Robert Andrews <robbiiie>
Michael Cheung <michaelc97>


How to execute the shell:
-------------------------
1.) Navigate to the src folder
2.) Type the "scl enable devtoolset-7 bash" command followed by the "make" command into the terminal
3.) To use the shell without plugins type "./esh"
4.) To use the shell with plugins type "./esh -p <directory/of/plugin>


Important Notes:
----------------
Our shell currently does not provide functionality for multi piping and io functionality when combined with piping.
Other than that our shell functions smoothly.


Description of Base Functionality:
----------------------------------
Jobs Command:
	This command iterates through the jobs list and prints out each job with the job id, job status, and job name
	The format looks like this: [Job ID] <Job Status>		(Job Name)

Kill Command:
	This command will kill a specified job based on the jobs id number. This command accepts a job id as its parameter and uses
	this id to send a SIGKILL signal to the process in order to kill it.

Fg Command:
	This command brings stopped or backgrounded jobs to the forground. This command accepts a job id as its parameter and uses
	this id to change the status to foreground and gives terminal access to the process.

Bg Command:
	This command brings jobs to the background and continues to excute them there. This command accepts a job id as its parameter 
	and uses this id to change the status to background.

Stop Command:
	This command stops a running process by sending a SIGSTOP signal to the job associated with the passed in job id. This command 
	accepts a job id as its parameter and uses this id to determine which process should be stopped.

Ctrl-C Command:
	Ctrl-C exits the shell by sending a SIGINT signal. Our shell monitors for this signal and when it occurs we exit

Ctrl-Z Command:
	Ctrl-Z stops a running process by sending a SIGTSTP signal. Our shell monitors for this signal and when it occurs we update 
	the staus of the affected jobs, reorganize terminal access, and continue the shell


Description of Extended Functionality:
--------------------------------------
Single Piping:
	Our shell handels single piping by changing the stdout of the first instruction to the stdin of the second instruction. 
	It is able to handle commands like "echo hello | rev" by taking the output from the echo command "hello" and feeding 
	it in as the input for the rev command to produce the output "olleh"

I/O:
	Our shell is able to preform operations that involve I/O redirection by either appending text to the end of an existing 
	file or puts the text in a newly created file. This implementation to our shell allows us to handle <, >, <<, and >> 
	symbols by checking if the iored_input/iored_output flags needed to be altered to append or overwrite or send data to or from files. 

Exclusive Access:
	

Multi-Piping and I/O + Piping:
	We do not have our shell to work with more than 2 pipes and I/O in combination with piping either.


List of Plugins Implemented:
----------------------------
Written by us:
	1.) toBin
	    Takes a number in the range of 0 to 15 and converts it to a binary string representation of that number
	2.) triangle
	    Finds the area of a triangle using passed in height and weight values
	3.)

Written by others:
	1.) campg3+ybryan10_genquote (genquote)
	    campg3+ybryan10
	2.) campg3+ybryan10_randNum (randNum)
	    campg3+ybryan10
	3.) campg3+ybryan10_revstring (revstring)
	    campg3+ybryan10
	4.) colinpeppler+gweihao_factorial (factorial)
	    colinpeppler+gweihao
	5.) emmam99+hdavid9_clock (clock)
	    emmam99+hdavid9
	6.) emmam99+hdavid9_d20 (d20)
	    emmam99+hdavid9
	7.) emmam99+hdavid9_fib (fib)
	    emmam99+hdavid9
	8.) jasonv+markb_lowercase (lowercase)
	    jasonv+markb
	    
