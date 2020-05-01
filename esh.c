/*
    Erik Safford
    Basic Interactive Shell
    Spring 2020
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

//Max number of space seperated args that will be read from command line
#define MAX_ARGS 6
//Max length of characters each arg can be
#define MAX_ARG_LENGTH 100
//Max total line length that will be parsed from command line
#define MAX_LINE_LENGTH 600
//DEBUG flag, 0 = off, 1 = on
#define DEBUG 0

//Check for space at the beginning of line, or multiple spaces between arguments
int checkCorrectSpacing(char *line) {
    //Return 1 if the line begins with a space
    if(line[0] == ' ') {
        return(1);
    }
    
    //Check for consecutive spaces
    int spaces = 0;
    int prevIndex = 0;
    for(int i = 0; i < strlen(line); i++) {
        if(line[i] == ' ') {
            spaces++;
            if(spaces == 2 && prevIndex == i-1) {
                //Return 1 if consecutive spaces are found
                return(1);
                break;
            }
        }
        else {
            spaces = 0;
        }
        prevIndex = i;
    }
    //Return 0 if no beginning space/consecutive spaces are found and line is valid input
    return(0);
}

//Get the number of space separated arguments within the line read by getline()
int getNumArgs(char *line) {
    int argCount = 0;
    if(strlen(line) > 0 && line[0] != '\n' && line[0] != ' ') {
        argCount++;
        for(int i = 0; i < strlen(line); i++) {
            if(line[i] == ' ') {
                argCount++;
            }
            if(argCount == MAX_ARGS) {
                break;
            }
        }
    }
    //Return number of space separated args in given line
    return(argCount);
}

//Allocate string array to hold user entered command line arguments and parse args into it
char **getArgs(char *line, int argCount) {
    char lineCopy[MAX_LINE_LENGTH] = {0};
    strncpy(lineCopy, line, sizeof(lineCopy));
    //DEBUG, print the line read in from getline()
    if(DEBUG) {
        printf("linecopy is: %s\n", lineCopy);
    }

    //Allocate args array (max of 6 command line args), with last arg as NULL for execv()?
    char **array = malloc(sizeof(char *) * (argCount+1));
    if(!array) {
        return(NULL);
    }

    //Allocate MAX_ARG_LENGTH spaces for each individual arg
    for(int i = 0; i < argCount+1; i++) {
        //Allocate char * MAX_ARG_LENGTH for each arg in array
        if(i != argCount) {
            array[i] = malloc(MAX_ARG_LENGTH + 1);
            if(!array[i]) {
                free(array);
                return(NULL);
            }
            //Intialize each subarray to avoid unconditionalized jumps
            memset(array[i], 0, sizeof(char) * MAX_ARG_LENGTH);    
        }
        //Else dont allocate memory for NULL
        else {
            array[i] = NULL;
        }
    }

    //Buffer to hold each individual arg before being copied into allocated arg array
    char str[MAX_ARG_LENGTH];
    //Index of the str array currently being entered into
    int strIndex = 0;
    //Index of the arg array currently being entered into
    int argIndex = 0;

    //Find each individual arg and copy it into allocated arg array
    for(int i = 0; i < strlen(lineCopy); i++) {
        //If the arg array has been filled with the max number of args, break
        if(argIndex == argCount) {
            break;
        }

        //Break apart the args in lineCopy on every space, or if the end of lineCopy is reached
        if(lineCopy[i] != ' ' && i != strlen(lineCopy)-1) {
            str[strIndex] = lineCopy[i];
            strIndex++;
        }
        //Add each space separated arg into the allocated arg array
        else {
            str[i] = '\0';
            strncpy(array[argIndex], str, sizeof(str));
            argIndex++;
            strIndex = 0;
            memset(str, 0, sizeof(str));
        }
    }
    return array;
}

//Attempt to execute a specified executable filePath with arguments within args
void execute(char *filePath, char **args, int argCount) {
    //If last args argument is not a '&', fork() a process and wait for child to finish
    if(args[argCount-1][0] != '&') {
        //fork() a new process to use execv()
        if(fork()) {
            //Parent process waits for child to finish
            wait(NULL);
            return;
        }
        //Child process executes given args commands at filePath
        else {
            execv(filePath, args);
            return; //return to make compiler happy, execv will never return
        }
    }
    //Else if last args argument is a '&', fork() a process and return automatically without waiting
    //(Run the executable in the background)
    else {
        //Set the '&' argument within args to NULL so it isnt taken as an argument to execv
        free(args[argCount-1]);
        args[argCount-1] = NULL;
        //fork() a new process to use execv()
        if(fork()) {
            return;
        }
        //Child process executes given args commands at filePath
        else {
            execv(filePath, args);
            return; //return to make compiler happy, execv will never return
        }
    }
}

//Check the env PATH/current directory for an executable and execute it
void checkEnvPaths(char *env, char **args, int argCount) {
    //Make a copy of the env PATH string to work with before destructive strtok (so it can be reused)
    //strdup() allocates memory, so it must be freed after
    char *envCopy = NULL;
    envCopy = strdup(env);

    //Use strtok to get separate env paths by breaking up the env PATH string on colons (:)
    char *path = NULL;
    //Flag to print error if specified filePath isn't found/executable
    int fpFlag = 0;

    //Get first colon seperated path
    path = strtok(envCopy, ":");
    while(path != NULL) {
        //Create the filepath/executable to search for in each env PATH directory
        char filePath[MAX_LINE_LENGTH] = {0};
        strncat(filePath, path, sizeof(filePath) - ((unsigned long)strlen(filePath)+1));
        strncat(filePath,"/", sizeof(filePath) - ((unsigned long)strlen(filePath)+1));

        //Append the first arg in args array (executable) to the filePath
        strncat(filePath, args[0], sizeof(filePath) - ((unsigned long)strlen(filePath)+1));

        //Check to see if filePath exists and is executable
        if(access(filePath, X_OK) == 0) {
            //Fork a new process and attempt to execv() filepath
            execute(filePath, args, argCount);
            fpFlag = 0;
            break;
        }
        else {
            fpFlag = 1;
        }

        //Reset filePath buffer
        memset(filePath, 0, sizeof(filePath));

        //Get the next colon seperated path
        path = strtok(NULL, ":");
    }

    //If filePath wasn't found in env PATH/isn't executable by user check the current directory
    if(fpFlag == 1) {
        char currentPath[MAX_LINE_LENGTH] = {0};
        //Get the current working directory filepath
        if(getcwd(currentPath, sizeof(currentPath)) != NULL) {
            strncat(currentPath,"/", sizeof(currentPath) - ((unsigned long)strlen(currentPath)+1));

            //Append the first arg in args array (executable) to the filePath
            strncat(currentPath, args[0], sizeof(currentPath) - ((unsigned long)strlen(currentPath)+1));

            //Check to see if currentPath exists and is executable
            if(access(currentPath, X_OK) == 0) {
                //If the path exists and is executable fork a new process and attempt to execv() filepath
                execute(currentPath, args, argCount);
            }
            else {
                fprintf(stderr, "filepath doesnt exist/isn't executable!\n");
            }      
        }
        else {
            fprintf(stderr, "getcwd() error\n");
        }   
    }
    //Free the buffer used by strdup()
    free(envCopy);
}

int main(int argc, char** argv) {
    //Get the environment PATH variable string
    char *env = NULL;
    env = getenv("PATH");

    //Buffer to hold command read in from user
    char *line = NULL;
    
    //Print terminal promt
    printf("> ");

    //Read contents of file into dynamic line buffer using getline()
    //getline() allocates memory, so it must be freed after
    size_t len = 0, read;
    while((read = getline(&line, &len, stdin) != -1)) {
        //Omit handling the cd command
        if(line[0] == 'c' && line[1] == 'd' && line[2] == '\n') {
            fprintf(stderr, "cd command invalid\n");
        } 
        //Exit the shell if the user types 'exit'
        else if(line[0] == 'e' && line[1] == 'x' && line[2] == 'i' && line[3] == 't' && line[4] == '\n') {
            break;
        }
        //Else attempt to execute the given shell commands
        else {
            //Check to make sure line doesnt start with a space or have consecutive spaces
            int correctSpacing = checkCorrectSpacing(line);

            //If command arg(s) spacing is valid
            if(correctSpacing == 0) {
                //Get the number of entered space separated command line arguments within line
                int argCount = getNumArgs(line);

                //If the user entered argument(s)
                if(argCount > 0) {
                    //Write executable/arguments to search for and execute from the line read in by getline() into string array
                    char **args = getArgs(line, argCount); 

                    //If args was allocated successfully   
                    if(args != NULL) {
                        //DEBUG, check what args are being entered into args array
                        if(DEBUG) {
                            for(int i = 0; i < argCount+1; i++) {
                                printf("args[%d] = %s\n", i, args[i]);
                            }
                        }

                        //Check the env PATH/current directory for an executable and execute it
                        checkEnvPaths(env, args, argCount);

                        //Free the args array
                        for(int i = 0; i < (argCount+1); i++) {
                            free(args[i]);
                        }
                        free(args);
                    }
                    //Else if args was not allocated correctly, print error
                    else {
                        fprintf(stderr, "getArgs() returned NULL\n");
                    }
                }
            }
            //Else if incorrect spacing was found in line input, print error
            else {
                fprintf(stderr, "Incorrect/consecutive spacing detected, input must be single spaced and not begin with a space\n");
            }
        }
        //Print terminal promt (wait a second to prevent command output spilling into input field)
        sleep(1);
        printf("> ");
    }
    //If user has entered 'exit', free the buffer used by getline() and exit program
    free(line);
    return(0);
}