#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include "parser.h"
#include "buffers.h"

#define PIPE  ('|')
#define RDOUT ('>')
#define RDIN  ('<')
#define BG    ('&')

#define is_pipe(c)   ((c) == PIPE)
#define is_rdin(c)   ((c) == RDIN)
#define is_rdout(c)  ((c) == RDOUT)
#define is_bg(c)     ((c) == BG)

#define is_spec(c)   (is_pipe(c) || is_rdin(c) || is_rdout(c) || is_bg(c))
#define is_id(c)     (!is_spec(c) && !isspace(c))


// Parses a command line string given by the first parameter
// into an intermediate parse structure given by the second,
// which the shell can easily process
int parse(char *line, parsed_line *pl) {

    size_t item_count = 0;
    size_t pipe_count = 0;

    determine_size(line, &item_count, &pipe_count);
    int res = check_buffers(pl, item_count, pipe_count);

    if (res < 0) {
        return -1;
    }

    // Initializes parse structure
    pl->rstdin     = NULL;
    pl->rstdout    = NULL;
    pl->pipe_count = pipe_count;
    pl->background = 0;
    pl->state      = CMD_EXPECTED;

    // Initializes current command
    command *cmd = pl->cmd_buf->start;

    cmd->items      = pl->item_buf->start;
    cmd->length     = 0;
    cmd->pipe_depth = 0;
    cmd->next       = NULL;

    line_lexer lex;
    init_lexer(&lex, line);
    char *token;
    char first; // Tokens with special meaning can be id'd by first char

    while (lex.has_next) {
        token = next_tok(&lex);
        first = token[0];
        if (is_spec(first)) {
            cmd = parse_spec(first, cmd, pl);
            if (cmd == NULL) {
                return -1;
            }
            if (is_bg(first) && !lex.has_next) {
                pl->state = ACCEPTING;
            }
            continue;
        }
        // Parses regular token
        switch (pl->state) {
            case IN_EXPECTED:
                pl->rstdin = token;
                break;
            case OUT_EXPECTED:
                pl->rstdout = token;
                break;
            // & has to appear last on the command line
            case BG_SET:
                fprintf(stderr, "Illegal background\n");
                return -1;
            case ACCEPTING: // Fallthrough
            case CMD_EXPECTED:
                // Append to command item list
                *cmd->items++ = token;
                cmd->length++;
                break;
        }
        pl->state = ACCEPTING;
    }

    if (pl->state != ACCEPTING) {
        return -1;
    }

    *cmd->items = NULL;        // Marks this command as done
    cmd->items -= cmd->length; // Returns command item list to start
    pl->cmd = cmd;             // Last command in pipeline is executed first
    return 0;

}


// Parses one of the special tokens.
// Called by parsing loop when it encounters one of them.
command *parse_spec(char spec, command *cmd, parsed_line *pl) {

     // The syntax never allows for two special tokens in a row
     if (pl->state != ACCEPTING) {
         fprintf(stderr, "Unexpected %c\n", spec);
         return NULL;
    }

    switch(spec) {
        case PIPE:
            // Continue at next command
            *cmd->items = NULL;
            char **item_buf_pos = cmd->items + 1;
            cmd->items -= cmd->length;
            cmd++;
            // Pipeline should appear in reverse order when executing
            cmd->next = cmd - 1;
            cmd->pipe_depth = cmd->next->pipe_depth + 1;
            cmd->items = item_buf_pos;
            cmd->length = 0;
            pl->state = CMD_EXPECTED;
            break;
        case RDIN:
            // stdin only makes sense to redirect first in pipeline...
            if (cmd->pipe_depth != 0) {
                fprintf(stderr, "Cannot redirect stdin\n");
                return NULL;
            }
            pl->state = IN_EXPECTED;
            break;
        case RDOUT:
            // ...and stdout last
            if (cmd->pipe_depth != pl->pipe_count) {
                fprintf(stderr, "Cannot redirect stdout\n");
                return NULL;
            }
            pl->state = OUT_EXPECTED;
            break;
        case BG:
            pl->background = 1;
            pl->state = BG_SET;
            break;
        default:
            return NULL;

    }
    return cmd;

}


// Calculates the number of items on
// and the pipeline depth of a command line
void determine_size(char *line, size_t *item_count, size_t *pipe_count) {

    char *c = line;
    while (*c != '\0') {

        while (isspace(*c)) {
            c++;
        }

        if (is_spec(*c)) {
            *pipe_count += *c++ == PIPE;
        } else if (*c != '\0') {
            (*item_count)++;
            while (is_id(*c) && *c != '\0') {
                c++;
            }
        }
    }

}


// Prepares lexer for use with function next_tok
void init_lexer(line_lexer *lexer, char *line) {

    lexer->line        = line;
    lexer->pos         = line;
    lexer->saved_token = NULL;
    lexer->has_next    = *line != '\0';

}


// Returns the next token from the lexer, or NULL when done. Does not allocate
// memory, but instead null terminates as appropriate
char *next_tok(line_lexer *lexer) {

    if (!lexer->has_next) {
        return NULL;
    }

    // A token may have been saved if it had
    // to be overwritten during previous call
    if (lexer->saved_token) {
        char *ret = lexer->saved_token;
        lexer->saved_token = NULL;
        lexer->has_next = *lexer->pos != '\0';
        return ret;
    }

    while (isspace(*lexer->pos)) {
        lexer->pos++;
    }

    while (*lexer->pos != '\0') {
        // Cannot null terminate special token because first char of next
        // token may be adjacent. Instead, return a constant
        if (is_spec(*lexer->pos)) {
            char *spec = (char *) get_spec(*lexer->pos++);
            lexer->has_next = *lexer->pos != '\0';
            return spec;
        }
        // Found start of id string
        char *str_start = lexer->pos;
        while (is_id(*lexer->pos) && *lexer->pos != '\0') {
            lexer->pos++;
        }
        if (isspace(*lexer->pos)) {
            // Whitespace after id string: null terminate at first space
            *lexer->pos++ = '\0';
        } else if (is_spec(*lexer->pos)) {
            // Special token after id string: save it
            lexer->saved_token = (char *) get_spec(*lexer->pos);
            lexer->has_next = 1;
            *lexer->pos++ = '\0';
        }

        // Last token of the line
        if (*lexer->pos == '\0' && lexer->saved_token == NULL) {
            lexer->has_next = 0;
        }

        return str_start;

     }

    return NULL;

}


// Returns a constant copy of one of the tokens with special meaning
// so the original safely can be overwritten with null when lexing
const char *get_spec(char c) {

    static const char PIPE_CONST[]  = { '|', '\0' };
    static const char RDIN_CONST[]  = { '<', '\0' };
    static const char RDOUT_CONST[] = { '>', '\0' };
    static const char BG_CONST[]    = { '&', '\0' };

    switch (c) {
        case PIPE:
            return PIPE_CONST;
            break;
        case RDIN:
            return RDIN_CONST;
            break;
        case RDOUT:
            return RDOUT_CONST;
            break;
        case BG:
            return BG_CONST;
            break;
        default:
            return NULL;
    }

}
