/*
 * parser.c - Unlambda parser
 * 
 * Copyright (C) 2020  CismonX <admin@cismon.net>
 *
 * This file is part of U6a.
 *
 * U6a is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * U6a is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with U6a.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "parser.h"
#include "logging.h"

#include <stdlib.h>

static const char* err_parse = "parse error";
static const char* info_parse = "parse";

bool
u6a_parse(struct u6a_token* token_arr, uint32_t token_len, struct u6a_ast_node** ast_arr) {
    const uint32_t ast_size = (token_len + 2) * sizeof(struct u6a_ast_node);
    // The array stores the pre-order traversal of AST nodes
    struct u6a_ast_node* ast = calloc(1, ast_size);
    if (UNLIKELY(ast == NULL)) {
        u6a_err_bad_alloc(err_parse, ast_size);
        return false;
    }
    // Add an explicit `e` function as a guard, to ensure proper exit of the program.
    U6A_AN_FN(ast) = u6a_tf_app;
    U6A_AN_FN(U6A_AN_LEFT(ast)) = u6a_tf_e;
    U6A_AN_LEFT(ast)->sibling = 2;
    const uint32_t pstack_size = (token_len + 1) * sizeof(struct u6a_ast_node*);
    struct u6a_ast_node** pstack = malloc(pstack_size);
    if (UNLIKELY(pstack == NULL)) {
        u6a_err_bad_alloc(err_parse, pstack_size);
        goto parse_failed;
    }
    pstack[0] = ast;
    uint32_t pstack_idx = 0;
    // LL(0) parser
    for (uint32_t token_idx = 0; token_idx < token_len; ++token_idx) {
        struct u6a_token* current_token = token_arr + token_idx;
        struct u6a_ast_node* child;
        if (UNLIKELY(pstack_idx == UINT32_MAX)) {
            u6a_err_bad_syntax(err_parse);
            goto parse_failed;
        }
        struct u6a_ast_node* top = pstack[pstack_idx];
        if (!U6A_AN_FN(U6A_AN_LEFT(top))) {
            child = U6A_AN_LEFT(top);
        } else {
            U6A_AN_LEFT(top)->sibling = token_idx + 2;
            child = U6A_AN_RIGHT(top, ast);
            --pstack_idx;
        }
        child->value = *current_token;
        if (current_token->fn == u6a_tf_app) {
            pstack[++pstack_idx] = child;
        }
    }
    if (UNLIKELY(pstack_idx != UINT32_MAX)) {
        u6a_err_bad_syntax(err_parse);
        goto parse_failed;
    }
    free(pstack);
    *ast_arr = ast;
    u6a_info_verbose(info_parse, "%s", "completed");
    return true;

    parse_failed:
    free(pstack);
    free(ast);
    return false;
}
