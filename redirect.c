//
//  redirect.c
//  myShell
//
//  Created by Connor Crawford on 2/4/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

/*
 * Contains functions responsible for redirection and piping
 */

#include "redirect.h"
#include "shell.h"
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>

/*
 * Checks if user wants to redirect stdin and opens output file if necessary
 * Parameters:      char** - array of C strings containing arguments
 * Returns:         FILE* - output file
 */
FILE* redirectInput(char **args) {
    FILE *fp;
    int i = 1;
    
    if (!args || !*args)
        return NULL;
    
    while (args[i] != NULL) {
        if (args[i][0] == '<') {
            if (args[++i] != NULL) {
                fp = fopen(args[i], "r");
                open(args[i], O_RDONLY);
                if (!fp)
                    perror("fopen");
                args[i-1] = NULL;
                return fp;
            }
        }
        i++;
    }
    return NULL;
}

/*
 * Checks if user wants to redirect stdout and opens output file if necessary
 * Parameters:      char** - array of C strings containing arguments
 * Returns:         FILE* - output file
 */
FILE* redirectOutput(char **args) {
    FILE *fp;
    char mode[] = "x";
    int i = 1;
    
    if (!args || !*args)
        return NULL;
    
    while (args[i] != NULL) {
        if (strcmp(args[i], ">") == 0)
            *mode = 'w'; // single arrow indicates truncate mode
        else if (strcmp(args[i], ">>") == 0)
            *mode = 'a';
        
        if (*mode != 'x') {
            if (args[++i] != NULL) {
                fp = fopen(args[i], mode);
                if (!fp)
                    perror("fopen");
                args[i-1] = NULL;
                return fp;
            }
        }
        i++;
    }
    return NULL;
}

/*
 * Checks if user wants to redirect stdout and opens output file if necessary
 * Parameters:      char** - array of C strings containing arguments
 * Returns:         int - 1 if user entered | in command, 0 if not
 */
int shouldPipe(char **args) {
    int i;
    
    if (!args || !*args)
        return 0;
    
    for (i = 0; args[i] != NULL; i++) {
        if (strcmp(args[i], "|") == 0) {
            i++;
            if (args[i] != NULL) {
                return i;
            }
        }
    }
    return 0;
}

/*
 * Creates child process and executes first part of command if user wants to pipe
 * Parameters:      char** - array of C strings containing arguments
 *                  int[] - file descriptors for pipe
 */
void myPipe(char **args, int fd[2]) {
    if (fork() == 0) {
        // child process
        close(fd[0]);
        while ((dup2(fd[1], STDOUT_FILENO) == -1)  && (errno == EINTR));
        close(fd[1]);
        
        myExecute(args);
    }
}

/*
 * Puts stdin/stdout/stderr back where they belong if they were redirected
 * Parameters:      FILE* - the batchfile if it exists
 *                  FILE* - the file with which stdin was replaced, if it exists
 *                  FILE* - the file with which stdout was replaced, if it exists
 *                  int - the file descriptor for the copy of stdin
 *                  int - the file descriptor for the copy of stdout
 *                  int - the file descriptor for the copy of stderr
 */
void restoreIO(FILE *batchfile, FILE* inFile, FILE* outFile, int stdinCopy, int stdoutCopy, int stderrCopy) {
    // restore I/O streams
    if (batchfile)
        dup2(fileno(batchfile), STDIN_FILENO);
    else
        dup2(stdinCopy, STDIN_FILENO);
    if (inFile) fclose(inFile);
    if (outFile) fclose(outFile);
    dup2(stdoutCopy, STDOUT_FILENO);
    dup2(stderrCopy, STDERR_FILENO);
    fflush(stdin);
}