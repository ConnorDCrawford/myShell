//
//  main.c
//  myShell
//
//  Created by Connor Crawford on 2/3/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

/*
 * Contains only the main function, which simply executes the shell function
 */

#include "shell.h"
#include <stdio.h>

int main(int argc, const char * argv[]) {
    
    FILE * batchfile = NULL;
    
    if (argv[1])
        batchfile = fopen(argv[1], "r");
    
    setEnv("SHELL");
    
    shell(batchfile); // myShell starts with user input, not a batchfile, so use NULL as parameter
    
    return 0;
}