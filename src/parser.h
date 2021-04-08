#ifndef PARSER_H
#define PARSER_H

// The commands in a pipeline are structured as a linked list
typedef struct c {
    // The items of a single command is a list of strings ending with NULL
    char **items;
    size_t length;
    size_t pipe_depth; // How deep in the pipeline this command is
    struct c *next;
} command;

typedef struct cb command_buffer;
typedef struct ib item_buffer;

// The different states a parse structure can be in while parsing
enum parse_state {
    ACCEPTING,
    CMD_EXPECTED,
    IN_EXPECTED,
    OUT_EXPECTED,
    BG_SET
};

// The parse structure, representing a whole command line
typedef struct pl {
    command *cmd;
    char *rstdin;
    char *rstdout;
    int background;
    size_t pipe_count;
    enum parse_state state;
    command_buffer *cmd_buf;
    item_buffer *item_buf;
} parsed_line;

// A simple lexer structure used to feed tokens to the parsing functions
typedef struct ll {
    char *line;
    char *pos;
    char *saved_token;
    int has_next;
} line_lexer;


int parse(char *line, parsed_line *pl);
void determine_size(char *line, size_t *item_count, size_t *pipe_count);
command *parse_spec(char spec, command *cmd, parsed_line *pl);
void init_lexer(line_lexer *lexer, char *line);
char *next_tok(line_lexer *lexer);
const char *get_spec(char c);

#endif
