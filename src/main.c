#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>
#include <limits.h>
#include "buffers.h"
#include "parser.h"
#include "builtins.h"

void sigint_handler(int);
void interpret_command_line(parsed_line *pl);
void prepare_pipeline(parsed_line *pl);
void prepare_command_node(command *cmd, char *in);
void execute_command(char **items);

// Foreground processes running commands are interrupted on SIGINT,
// but the shell process ignores it
void sigint_handler(int signal) {
}


int main(void) {

    signal(SIGINT, sigint_handler);
    parsed_line pl;
    char *prompt;
    char work_dir[PATH_MAX];
    char *line = NULL;
    int res;

    res = init_buffers(&pl);

    if (res < 0) {
        fprintf(stderr, "Could not initialize buffers\n");
        exit(EXIT_FAILURE);
    }

    int done = 0;

    // Shell loop
    do {
        getcwd(work_dir, PATH_MAX);
        prompt = strncat(work_dir, "> ", 2);
        line = readline(prompt);
        add_history(line);

        if (!line) { // EOF
            done = 1;
        } else if (!*line) {
            ; // Do nothing on empty string
        } else {
            res = parse(line, &pl);
            if (res < 0) {
                fprintf(stderr, "Parse error\n");
            } else {
                interpret_command_line(&pl);
            }
        }

        if (line) {
            free(line);
        }

    } while (!done);

    free_buffers(&pl);
    return 0;

}


void interpret_command_line(parsed_line *pl) {

    // Built-in commands relate to the shell process,
    // so just execute and return to loop
    builtin_fun_ptr f;
    f = try_builtin(pl);
    if (f) {
        (*f)(pl);
        return;
    }

    // Otherwise, run the commands in a child process
    pid_t pid = fork();

    if (pid == -1) {
        fprintf(stderr,"Fork error\n");
    } else if (pid != 0) {
        // Parent (shell)
        waitpid(pid, NULL, 0);
    } else {
        // Child
        if (pl->background) {
            // Background processes fork again and their children
            // (that do exec()) are inherited by init
            pid_t pidbg = fork();
            if (pidbg != 0) {
                exit(0);
            }
            setpgid(0, 0); // Makes it not receive SIGINT from Ctrl-C
        }
        prepare_pipeline(pl);
    }

}


// Sets up overall output and calls another function to process
// the individual command nodes of a parsed command line
void prepare_pipeline(parsed_line *pl) {

    if (pl->rstdout) {
        int fd = open(pl->rstdout, O_CREAT|O_WRONLY, S_IRUSR|S_IWUSR);
        if (fd == -1) {
            fprintf(stderr, "Unable to open file %s\n", pl->rstdout);
            exit(EXIT_FAILURE);
        }
        dup2(fd, 1); // Sets stdout for this process to specified file
    }
    prepare_command_node(pl->cmd, pl->rstdin);

}


// If cmd is not last in the pipeline, a fork is performed where the parent
// executes the current command with the child's output as input,
// and the child continues recursively to the next node.
// The specified redirection of stdin is included as a parameter, because the
// last forked process is the one that will be using it.
void prepare_command_node(command *cmd, char *in) {

    if (cmd->next == NULL) { // Last in the chain should not fork further
        if (in) {
            int fd = open(in, O_RDONLY);
            struct stat buffer;
            if (stat(in, &buffer) == -1 || fd == -1) {
                fprintf(stderr, "Unable to open file %s\n", in);
                exit(EXIT_FAILURE);
            }
            dup2(fd, 0); // Sets stdin for this process to specified file
        }
        execute_command(cmd->items);
    }

   // Sets up pipe through which the process with the current command and
   // the process with the next can communicate
    int p[2], pid;
    pipe(p);
    pid = fork();

    if (pid == -1) {
        fprintf(stderr, "Fork error\n");
        exit(EXIT_FAILURE);

    } else if (pid != 0) {
        // Parent
        dup2(p[0], 0); // Read from child's output
        close(p[0]);
        close(p[1]);
        waitpid(pid, NULL, 0);
        execute_command(cmd->items);

    } else {
        // Child
        dup2(p[1], 1); // Write to parent's input
        close(p[0]);
        close(p[1]);
        prepare_command_node(cmd->next, in);
    }

}


// Executes a single command in the calling process
void execute_command(char **items) {

    size_t argc = 0;
    while (*items != NULL) {
        argc++;
        items++;
    }
    items -= argc;

    char *argv[argc+1];
    argv[argc] = NULL;
    for (size_t i = 0; i < argc; ++i, ++items) {
        argv[i] = *items;
    }

    execvp(argv[0], argv);

    // execvp failed
    fprintf(stderr, "Unknown or malformatted command: %s\n", argv[0]);
    exit(EXIT_FAILURE);

}
