//
//  redirect.h
//  myShell
//
//  Created by Connor Crawford on 2/4/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#ifndef redirect_h
#define redirect_h

#include <stdio.h>


void createPipes(int fd_in[2], int fd_out[2]);
int redirectIO(int fd_in[2], int fd_out[2]);
FILE* redirectInput(char **args);
FILE* redirectOutput(char **args);
int shouldPipe(char **args);
void myPipe(char **args, int fd[2]);
void restoreIO(FILE *batchfile, FILE* inFile, FILE* outFile, int stdinCopy, int stdoutCopy, int stderrCopy);

#endif /* redirect_h */
