#ifndef BUFFERS_H
#define BUFFERS_H

typedef struct c command;
typedef struct pl parsed_line;

// Buffer for individual commands
typedef struct cb {
    command *start;
    size_t size;
} command_buffer;

// Buffer for all items of all commands of a line
typedef struct ib {
    char **start;
    size_t size;
} item_buffer;

int init_buffers(parsed_line *pl);
int check_buffers(parsed_line  *pl, size_t item_count, size_t pipe_count);
void free_buffers(parsed_line *pl);

#endif
