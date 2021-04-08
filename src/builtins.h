#ifndef BUILTINS_H
#define BUILTINS_H

typedef void (*builtin_fun_ptr)(parsed_line *);

void (*try_builtin(parsed_line *pl))(parsed_line *);
void exit_shell(parsed_line *pl);
void cd(parsed_line *pl);
void help();

#endif
