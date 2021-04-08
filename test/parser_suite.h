#ifndef PARSER_SUITE_H
#define PARSER_SUITE_H

#include "CUnit/Basic.h"
#include "src/parser.h"

int init_suite_parser();
int clean_suite_parser();

void test_determine_size_empty();
void test_determine_size_only_whitespace();
void test_determine_size_deep();
void test_determine_size_varying_item_whitespace_length();
void test_parse_spec_rejecting_state();
void test_parse_spec_pipe();
void test_parse_spec_rdin_legal();
void test_parse_spec_rdin_illegal();
void test_parse_spec_rdout_legal();
void test_parse_spec_rdout_illegal();
void test_parse_spec_bg();
void test_lexer_empty_string();
void test_lexer_leading_spaces();
void test_lexer_spec();
void test_lexer_spec_after_id();
void test_get_spec_normal();
void test_get_spec_non_special_token();

#endif
