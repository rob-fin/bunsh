#include <stdio.h>
#include "CUnit/Basic.h"
#include "src/parser.h"
#include "test/parser_suite.h"

int main() {

    if (CU_initialize_registry() != CUE_SUCCESS) {
        return CU_get_error();
    }

    CU_pSuite pSuite_parser = NULL;
    pSuite_parser = CU_add_suite("PARSER", NULL, NULL);

    if (!pSuite_parser) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    if (!CU_add_test(pSuite_parser, "determine_size, empty",
                     test_determine_size_empty) ||
        !CU_add_test(pSuite_parser, "determine_size, only whitespace",
                     test_determine_size_only_whitespace) ||
        !CU_add_test(pSuite_parser, "determine_size, deep",
                     test_determine_size_deep) ||
        !CU_add_test(pSuite_parser, "determine_size, varying item length",
                     test_determine_size_varying_item_whitespace_length) ||
        !CU_add_test(pSuite_parser, "parse_spec, rejecting state",
                     test_parse_spec_rejecting_state) ||
        !CU_add_test(pSuite_parser, "parse_spec, pipe",
                     test_parse_spec_pipe) ||
        !CU_add_test(pSuite_parser, "parse_spec, rdin legal",
                     test_parse_spec_rdin_legal) ||
        !CU_add_test(pSuite_parser, "parse_spec, rdin illegal",
                     test_parse_spec_rdin_illegal) ||
        !CU_add_test(pSuite_parser, "parse_spec, rdout legal",
                     test_parse_spec_rdout_legal) ||
        !CU_add_test(pSuite_parser, "parse_spec, rdout illegal",
                     test_parse_spec_rdout_illegal) ||
        !CU_add_test(pSuite_parser, "parse_spec, bg",
                     test_parse_spec_bg) ||
        !CU_add_test(pSuite_parser, "lexer, empty string",
                     test_lexer_empty_string) ||
        !CU_add_test(pSuite_parser, "lexer, leading spaces",
                     test_lexer_leading_spaces) ||
        !CU_add_test(pSuite_parser, "lexer, spec",
                     test_lexer_spec) ||
        !CU_add_test(pSuite_parser, "lexer, spec after id",
                     test_lexer_spec_after_id) ||
        !CU_add_test(pSuite_parser, "get_spec, normal",
                     test_get_spec_normal) ||
        !CU_add_test(pSuite_parser, "get_spec, non-special char",
                     test_get_spec_non_special_token)) {
        CU_cleanup_registry();
        return CU_get_error();
    }

    CU_basic_set_mode(CU_BRM_VERBOSE);
    CU_basic_run_tests();
    CU_cleanup_registry();

    if (CU_get_error() == CUE_SUCCESS) {
        return CU_get_number_of_tests_failed();
    } else {
        fprintf(stderr, "%s\n", CU_get_error_msg());
        return CU_get_error();
    }

}
