# Basic-Interactive-Shell
* This program consists of an interactive shell (EggShell/esh), a command line environment in which users can execute commands, written in C.
* The executable is called esh and can be ran with: ./esh
* esh runs an infinite loop with getline() that provides the user with a promt ('>'). At this promt, the user can enter commands which the shell will attempt to execute.
* After the input is read in, the shell first verifies that the spacing of the line is correct, i.e. input cannot begin with a space or have multiple consecutive spaces between arguments. Then the shell records the number of space seperated arguments, and allocates a stucture getArgs that contains all of the entered arguments, with a NULL at the last spot in the structure. If errors occur when allocating getArgs an error message is printed to stderr.
* Once the arguments are recorded, the shell verifies that the command entered is a legal command by searching for the command on the path specified by the environment variable PATH.
* The PATH consists of a series of paths delimited by the ':' character (which can be seen by typing 'echo $PATH' on a linux command line).
* To access the directories included on the PATH, functions getenv() and strtok() are used. If esh cannot find the command on the PATH, then it checks the current directory with function getcwd(). If getcwd() fails or a command is still not found, an error message is printed to stderr, and a new promt is presented. esh should not crash.
* If the command is found, esh attempts to execute it using fork() to spawn a new child process, and execv() to execute the commands stored in the getArgs structure. Commands may have up to five optional arguments which are delimited by a single space.
* If the last argument in a command is an '&' character, it indicates that the command is to be run in the background. I.e. when the shell spawns a child process it will not wait for the child to terminate before prompting the user for the next command.
* If the user types 'exit', then esh will terminate.
* The 'cd' command is not handled, and is omitted from the shell. If cd is entered an error is printed to stderr.
* A makefile is included to build esh, and can be run with 'make'.
* #define's are included to change the maximum number of arguments read by esh, the maximum length of the arguments, and the maximum line length to be parsed from getline().
* A DEBUG flag is included to see the contents being read in by getline(), and the recorded arguments within the getArgs structure. 0 = off (no DEBUG statements), 1 = on (DEBUG statements)
## Screenshot
![Alt text](/screenshot/sc.png?raw=true "sc")