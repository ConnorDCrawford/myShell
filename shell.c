//
//  shell.c
//  myShell
//
//  Created by Connor Crawford on 2/3/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//


/*
 * Contains functions pertaining to the fundamental operation of the shell,
 * including the logic of the shell, getting the command line, parsing the
 * command line, and executing programs.
 */

#include "shell.h"
#include "builtins.h"
#include "redirect.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

void printPrompt();
char* getCmd(FILE *inFile);
void parseCmd(char *cmd, char **args);
int isBuiltIn(char **args);
int isBackgroundProcess(char **args);
void createChild(char **args);

/*
 * Main logic of the shell. Will process batch commands if batchfile is not NULL.
 * Parameters:      FILE* - pointer to a file containing batch commands, can be NULL
 */
void shell(FILE *batchfile) {
    char *cmd = NULL;
    char *args[128];
    FILE *inFile = NULL, *outFile = NULL;
    
    // copy file descriptors of I/O streams for later restoration
    int stdinCopy = dup(STDIN_FILENO),
        stdoutCopy = dup(STDOUT_FILENO),
        stderrCopy = dup(STDERR_FILENO);
    
    while (1) {
        
        if (!batchfile)
            printPrompt(); // Don't print prompt if recieving commands from a batchfile
        
        cmd = getCmd(batchfile);
        if (*cmd == '\0')
            break; // stop instance of shell if command is null character, or if using a batchfile and reach EOF
        
        parseCmd(cmd, args);
        if (!*args)
            continue; // no arguments, so go back to top of loop to get a new command
        
        inFile = redirectInput(args); // check if user wants to recieve input from a file, open file if so
        outFile = redirectOutput(args); // check if user wants to redirect output to a file, open file if so
        
        if (inFile) {
            dup2(fileno(inFile), STDIN_FILENO); // replace stdin with input file if it exist
        }
        if (outFile) {
            dup2(fileno(outFile), STDOUT_FILENO); // replace stdout with output file if it exist
            dup2(fileno(outFile), STDERR_FILENO); // replace stderr with output file if it exist
        }
        
        if (!executeBuiltIn(args)) // command was not internal, try to execute command
            createChild(args);
        
        restoreIO(batchfile, inFile, outFile, stdinCopy, stdoutCopy, stderrCopy); // restore I/O streams if redirection occured
        free(cmd);
    }
}

/*
 * Gets a command line from stdin or a batchfile
 * Parameters:      FILE* - pointer to a file containing batch commands, can be NULL
 * Returns:         char* - a C string containing the command
 */
char* getCmd(FILE *inFile) {
    char *cmd = NULL;
    size_t nBytes = 0;
    
    if (!inFile) {
        inFile = stdin; // if no batchfile, read from stdin
        fflush(stdin);
    }
    
    while (!cmd)
        getline(&cmd, &nBytes, inFile); // get line from either stdin or batchfile
    
    return cmd;
}

/*
 * Separates commands into an array of arguments
 * Parameters:      char* - pointer to a file containing batch commands, can be NULL
 *                  char** - array of C strings containing arguments
 */
void parseCmd(char *cmd, char **args) {
    int i;
    char delim[] = {'\t', '\n', '\v', '\f', '\r', ' '}; // separate arguments by whitespace
    
    if (!cmd)
        return;
    
    if (args) {
        char *token = strtok(cmd, delim);
        
        i = 0;
        while (token) {
            args[i++] = token;
            token = strtok(NULL, delim);
        }
    }
    
    args[i] = NULL; // set element after last argument to NULL
}

/*
 * Prints prompt contianing the machine's name and the current working directory
 */
void printPrompt() {
    char name[512];
    char cwd[512];
    
    if (!gethostname(name, sizeof(name))) // get name of machine
        printf("%s: ", name);
    else
        perror("gethostname");
    
    if (getcwd(cwd, sizeof(cwd))) // get current working directory
        printf("%s> ", cwd);
    else
        perror("getcwd");
}

/*
 * Creates child process that will execute the command
 * Parameters:      char** - array of C strings containing arguments
 */
void createChild(char **args) {
    pid_t cPID;
    int fd[2];
    pipe(fd);
    int pipeNo;
    
    pipeNo = shouldPipe(args); // returns index of | if it exists, otherwise returns 0
    if (pipeNo) {
        *(args+pipeNo-1) = NULL; // replace | in args with NULL so command after won't be executed
        myPipe(args, fd);
    }
    
    if ((cPID = fork()) != -1) { // check for fork failure
        if (cPID == 0) {
            // child process
            close(fd[1]); // close write end of pipe, not needed
            if (pipeNo) while ((dup2(fd[0], STDIN_FILENO) == -1)  && (errno == EINTR)); // Redirect stdin if piping
            close(fd[0]); // close read end of pipe, no longer needed
            
            myExecute(args+pipeNo); // jump to command after |, if it exists
            exit(EXIT_FAILURE); // exec failed, kill child
        } else {
            // parent process
            if (!isBackgroundProcess(args))
                waitpid(cPID, NULL, 0);
        }
    }
    
}

/*
 * Executes the specified arguments
 * Parameters:      char** - array of C strings containing arguments
 */
void myExecute(char **args) {
    if (args) {
        setEnv("PARENT");
        execvp(*args, args);
        printf("-myShell: %s: command not found\n", *args); // exec failed
    }
}

/*
 * Determines if user wants program to execute in background
 * Parameters:      char** - array of C strings containing arguments
 */
int isBackgroundProcess(char **args) {
    int i = 0;
    
    if (!args || !*args)
        return -1;
    
    while (args[i++] != NULL);
    if (strcmp(args[i-2], "&") == 0)
        return 1; // last argument is &, so it is a background process
    
    return 0;
}

/*
 * Sets the specified environment variable to the directory of the myShell executable
 * Parameters:      char* - the environment vairbale name
 */
void setEnv(char *name) {
    char cwd[512];
    
    if (!getcwd(cwd, sizeof(cwd))) { // get current working directory
        perror("getcwd");
        return;
    }
    
    strcat(cwd, "/myShell"); // add name of myShell executable
    
    if (name) setenv(name, cwd, 1); // set environment variable to the pathname
}