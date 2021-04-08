#include <stdlib.h>
#include "CUnit/Basic.h"
#include "src/parser.h"
#include "src/buffers.h"

size_t item_count;
size_t pipe_count;
line_lexer lex;
parsed_line pl;
command cmd;

void reset_fixtures() {
    item_count = 0;
    pipe_count = 0;
    pl.cmd = NULL;
    pl.rstdin = NULL;
    pl.background = 0;
    pl.pipe_count = 0;
    pl.state = CMD_EXPECTED;
    pl.cmd_buf = NULL;
    pl.item_buf = NULL;
    cmd.items = NULL;
    cmd.length = 0;
    cmd.pipe_depth = 0;
    cmd.next = NULL;
}

void test_determine_size_empty() {
    reset_fixtures();
    determine_size("", &item_count, &pipe_count);
    CU_ASSERT_EQUAL(item_count, 0);
    CU_ASSERT_EQUAL(pipe_count, 0);
}

void test_determine_size_only_whitespace() {
    reset_fixtures();
    char *line = "     \t    \n         ";
    determine_size(line, &item_count, &pipe_count);
    CU_ASSERT_EQUAL(item_count, 0);
    CU_ASSERT_EQUAL(pipe_count, 0);
}

void test_determine_size_deep() {
    reset_fixtures();
    char *line = "a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a|a";
    determine_size(line, &item_count, &pipe_count);
    CU_ASSERT_EQUAL(item_count, 30);
    CU_ASSERT_EQUAL(pipe_count, 29);
}

void test_determine_size_varying_item_whitespace_length() {
    reset_fixtures();
    char *line = "a |bb  |ccc  | dddd  | eeeee|       ffffff";
    determine_size(line, &item_count, &pipe_count);
    CU_ASSERT_EQUAL(item_count, 6);
    CU_ASSERT_EQUAL(pipe_count, 5);
}

void test_parse_spec_rejecting_state() {
    reset_fixtures();
    pl.state = CMD_EXPECTED;
    CU_ASSERT_PTR_NULL(parse_spec('x', NULL, &pl));
}

void test_parse_spec_pipe() {
    reset_fixtures();
    init_buffers(&pl);
    pl.state = ACCEPTING;
    command *cmd_before = pl.cmd_buf->start;
    cmd_before->pipe_depth = 4;
    cmd_before->items = pl.item_buf->start;
    char **buf_pos = cmd_before->items;
    command *cmd_after = parse_spec('|', cmd_before, &pl);
    CU_ASSERT_EQUAL(cmd_after, cmd_before + 1);
    CU_ASSERT_PTR_NULL(*buf_pos);
    CU_ASSERT_EQUAL(cmd_after->pipe_depth, 5);
    CU_ASSERT_EQUAL(cmd_after->items, buf_pos + 1);
    CU_ASSERT_EQUAL(cmd_after->length, 0);
    CU_ASSERT_EQUAL(pl.state, CMD_EXPECTED);
    free_buffers(&pl);
}

void test_parse_spec_rdin_legal() {
    reset_fixtures();
    pl.state = ACCEPTING;
    cmd.pipe_depth = 0;
    parse_spec('<', &cmd, &pl);
    CU_ASSERT_EQUAL(pl.state, IN_EXPECTED);
}

void test_parse_spec_rdin_illegal() {
    reset_fixtures();
    pl.state = ACCEPTING;
    cmd.pipe_depth = 1;
    CU_ASSERT_PTR_NULL(parse_spec('<', &cmd, &pl));
}

void test_parse_spec_rdout_legal() {
    reset_fixtures();
    pl.pipe_count = 11;
    pl.state = ACCEPTING;
    cmd.pipe_depth = 11;
    parse_spec('>', &cmd, &pl);
    CU_ASSERT_EQUAL(pl.state, OUT_EXPECTED);
}

void test_parse_spec_rdout_illegal() {
    reset_fixtures();
    pl.pipe_count = 15;
    pl.state = ACCEPTING;
    cmd.pipe_depth = 9;
    parse_spec('>', &cmd, &pl);
    CU_ASSERT_PTR_NULL(parse_spec('>', &cmd, &pl));
}

void test_parse_spec_bg() {
    reset_fixtures();
    pl.state = ACCEPTING;
    parse_spec('&', NULL, &pl);
    CU_ASSERT_EQUAL(pl.background, 1);
    CU_ASSERT_EQUAL(pl.state, BG_SET);
}

void test_lexer_empty_string() {
    reset_fixtures();
    char line[] = "";
    init_lexer(&lex, line);
    CU_ASSERT_PTR_NULL(next_tok(&lex));
    CU_ASSERT_EQUAL(lex.has_next, 0);
}

void test_lexer_leading_spaces() {
    reset_fixtures();
    char line[] = "                 z";
    init_lexer(&lex, line);
    CU_ASSERT_STRING_EQUAL(next_tok(&lex), "z");
    CU_ASSERT_EQUAL(lex.has_next, 0);
}

void test_lexer_spec() {
    reset_fixtures();
    char line[] = "|<>& |  <  >   &";
    init_lexer(&lex, line);
    CU_ASSERT_STRING_EQUAL(next_tok(&lex), "|");
    CU_ASSERT_STRING_EQUAL(next_tok(&lex), "<");
    CU_ASSERT_STRING_EQUAL(next_tok(&lex), ">");
    CU_ASSERT_STRING_EQUAL(next_tok(&lex), "&");
    CU_ASSERT_STRING_EQUAL(next_tok(&lex), "|");
    CU_ASSERT_STRING_EQUAL(next_tok(&lex), "<");
    CU_ASSERT_STRING_EQUAL(next_tok(&lex), ">");
    CU_ASSERT_STRING_EQUAL(next_tok(&lex), "&");
}

void test_lexer_spec_after_id() {
    reset_fixtures();
    char line[] = "abc|";
    init_lexer(&lex, line);
    CU_ASSERT_STRING_EQUAL(next_tok(&lex), "abc");
    CU_ASSERT_STRING_EQUAL(lex.saved_token, "|");
    CU_ASSERT_EQUAL(lex.has_next, 1);
    CU_ASSERT_STRING_EQUAL(next_tok(&lex), "|");
    CU_ASSERT_EQUAL(lex.has_next, 0);
}

void test_get_spec_normal() {
    reset_fixtures();
    const char expected[] = { '|', '\0' };
    const char *actual = get_spec('|');
    CU_ASSERT_STRING_EQUAL(actual, expected);
}

void test_get_spec_non_special_token() {
    reset_fixtures();
    const char *result = get_spec('x');
    CU_ASSERT_PTR_NULL(result);
}
