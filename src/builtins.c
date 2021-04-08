#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <limits.h>
#include "parser.h"
#include "builtins.h"
#include "buffers.h"


// Checks if the first command line item matches an internal command
void (*try_builtin(parsed_line *pl))(parsed_line *) {

    char *fun = *(pl->cmd->items);
    if (!strcmp(fun, "cd")) {
        return cd;
    } else if (!strcmp(fun, "exit")) {
        return exit_shell;
    } else if (!strcmp(fun, "help")) {
        return help;
    }

    return NULL;

}


// Changes a process' working directory
void cd(parsed_line *pl) {

    char *path = *(++pl->cmd->items); // Advance to cd's arg

    char wd[PATH_MAX];
    getcwd(wd, PATH_MAX);

    if (path == NULL) {
        // Home
        path = getenv("HOME");
    } else if (path[0] == '.' && !path[1]) {
        // Current
        return;
    } else if (path[0] == '.' && path[1] == '.' && !path[2]) {
        // Parent
        char *last_slash = strrchr(wd, '/');
        *(last_slash+1) = '\0';
        // Path is now that of the parent directory
        path = wd;
    } else if (*path != '/') {
        // Relative path: concatenate current directory, /, and argument
        strncat(wd, "/", 1);
        strncat(wd, path, strlen(path));
        path = wd;
    }

    int status = 0;

    status = chdir(path);

    if (status != 0) {
        fprintf(stderr, "Unknown path: %s\n", path);
    }

}


// Exits the shell
void exit_shell(parsed_line *pl) {

    free_buffers(pl);
    exit(EXIT_SUCCESS);

}


// Prints a help message
void help() {

    char *message = "\n Usage:\n\n   command [ | command ]*\n\n"
                    " where command is an absolute path or"
                    " something that can be found through $PATH.\n\n"
                    " Redirections such as '< infile' and '> outfile'"
                    " may appear after a command.\n"
                    " Appending an '&' to a line"
                    " will run the job in the background.\n\n"
                    " Commands defined internally:\n"
                    "  cd [dir]\n  exit\n  help\n\n";

    printf("%s", message);

}
