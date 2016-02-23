//
//  shell.h
//  myShell
//
//  Created by Connor Crawford on 2/3/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

#ifndef shell_h
#define shell_h

#include <stdio.h>

void shell(FILE *inFile);
void myExecute(char **args);
void setEnv(char *name);

#endif /* shell_h */
