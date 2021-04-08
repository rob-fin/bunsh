#include <stdlib.h>
#include "buffers.h"
#include "parser.h"

#define CMD_BUF_SIZE    8 * sizeof(command)
#define ITEM_BUF_SIZE  32 * sizeof(char *)


// Allocates memory for the parse structure's buffers
int init_buffers(parsed_line *pl) {

    static command_buffer cmd_buf;
    cmd_buf.start = malloc(CMD_BUF_SIZE);

    static item_buffer item_buf;
    item_buf.start = malloc(ITEM_BUF_SIZE);

    if (cmd_buf.start == NULL || item_buf.start == NULL) {
        return -1;
    }

    cmd_buf.size = CMD_BUF_SIZE;
    pl->cmd_buf = &cmd_buf;
    item_buf.size = ITEM_BUF_SIZE;
    pl->item_buf = &item_buf;

    return 0;

}


// Given a command line with item_count items and a pipeline depth of
// pipe_count, make sure the buffers can accommodate the parse structure
int check_buffers(parsed_line *pl, size_t item_count, size_t pipe_count) {
    // Command buffer
    size_t curr_size = pl->cmd_buf->size;
    // The number of commands is one more than the number of pipes
    size_t want_size  = (pipe_count + 1) * sizeof(command);
    if (want_size > curr_size) {
        pl->cmd_buf->start = realloc(pl->cmd_buf->start,
                                     want_size);

        if (pl->cmd_buf->start == NULL) {
            return -1;
        }
        pl->cmd_buf->size = want_size;
    }

    // Item buffer
    curr_size = pl->item_buf->size;
    // Each item list of each command ends with NULL
    want_size = (item_count + pipe_count) * sizeof(char *);
    if (want_size > curr_size) {
        pl->item_buf->start = realloc(pl->item_buf->start,
                                      want_size);

        if (pl->item_buf->start == NULL) {
            return -1;
        }
        pl->item_buf->size = want_size;
    }

    return 0;
}


void free_buffers(parsed_line *pl) {

    free(pl->item_buf->start);
    free(pl->cmd_buf->start);

}
