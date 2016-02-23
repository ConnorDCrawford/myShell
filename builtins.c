//
//  builtins.c
//  myShell
//
//  Created by Connor Crawford on 2/3/16.
//  Copyright © 2016 Connor Crawford. All rights reserved.
//

/*
 * Contains mechanism to check if a command is a builtin function, and the builtin functions
 */

#include "builtins.h"
#include "shell.h"
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <dirent.h>

void myShell(char **args);
void my_cd(char **args);
void my_clr(char **args);
void listContents(DIR *d);
void my_dir(char **args);
void my_environ(char **args);
void my_echo(char **args);
void my_help(char **args);
void my_echo(char **args);
void my_pause(char **args);
void my_quit(char **args);

char *builtin_str[] = { "myshell", "cd", "clr", "dir", "environ", "echo", "help", "echo", "pause", "quit", NULL };
void (*builtin_func[]) (char **) = { &myShell, &my_cd, &my_clr, &my_dir, &my_environ, &my_echo, &my_help,
    &my_echo, &my_pause, &my_quit};

/*
 * Searches user's command from list of internal commands, executes command if it exists
 * Parameters:      char** - char** - array of C strings containing arguments
 * Returns:         int - 1 if command was a builtin function, 0 if it was not
 */
int executeBuiltIn(char **args) {
    int i;
    
    if (!args || !*args)
        return 0;
    
    for (i = 0; builtin_str[i] != NULL; i++) {
        if (strcmp(args[0], builtin_str[i]) == 0) { // compare first argument against list of internal commands
            builtin_func[i](args); // execute that corresponding function
            return 1;
        }
    }
    
    return 0;
}

/*
 * Opens specified batchfile, executes batch commands
 * Parameters:      char** - array of C strings containing arguments
 */
void myShell(char **args) {
    FILE *batchfile;
    if (*++args) {
        batchfile = fopen(*args, "r");
        if (!batchfile) {
            printf("myshell: %s: No such file or directory\n", *args);
        } else {
            shell(batchfile); // execute another instance of shell with the batchfile
            fclose(batchfile);
            fflush(stdout);
        }
    }
}

void my_cd(char **args) {
    if (!*(args+1)) {
        if (chdir(getenv("HOME"))) // user only entered "cd," which goes to home directory
            perror("cd");
    } else {
        if (chdir(*(args+1))) // user entered a pathname
            perror("cd");
    }
}

void my_clr(char **args) {
    printf("\033[2J\033[1;1H"); // this code will clear terminal and bring cursor to top in POSIX systems
}

void listContents(DIR *d) {
    struct dirent *dir;
    
    if (!d)
        perror("opendir");
    
    while ((dir = readdir(d)) != NULL)
        printf ("%s\n", dir->d_name); // print the name of each directory/file until there are none left
    
    closedir(d);
}

void my_dir(char **args) {
    DIR *d;
    char *dirName;
    
    if (!args[1])
        dirName = "."; // user only entered "dir," meaning they want to print contents of current directory
    else
        dirName = args[1];
    
    d = opendir(dirName);
    listContents(d);
}

void my_environ(char **args) {
    extern char **environ;
    int i;
    
    for (i = 0; environ[i] != NULL; i++)
        printf("%s\n", environ[i]); // print all environment strings
}

void my_echo(char **args) {
    int i;
    
    for (i = 1; args[i] != NULL; i++)
        printf("%s ", args[i]); // print every element in args after "echo"
    printf("\n");
}

void my_help(char **args) {
    char pwd[512];
    pid_t cPID;
    
    strcat(pwd, getenv("PWD")); // get parent directory of executable
    strcat(pwd, "/readme"); // add name of readme file
    
    cPID = fork();
    if (cPID == 0) {
        execlp("more", "more", pwd, NULL); // execute more filter with readme file
        perror("execlp"); // exec failed
    } else {
        waitpid(cPID, NULL, 0); // wait for process to complete
    }
}

void my_pause(char **args) {
    while (getchar() != '\n'); // wait unti 'Enter' is pressed
}

void my_quit(char **args) {
    exit(EXIT_SUCCESS);
}