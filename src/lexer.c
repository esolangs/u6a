/*
 * lexer.c - Unlambda lexer
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

#include "lexer.h"
#include "logging.h"

#include <stdlib.h>
#include <stddef.h>
#include <inttypes.h>
#include <ctype.h>

#define READ_CH()                              \
    ch = fgetc(input_stream);                  \
    if (UNLIKELY(ch == EOF)) {                 \
        u6a_err_unexpected_eof(err_lex, fn);   \
        goto lex_failed;                       \
    }                                          \
    if (LIKELY(isprint(ch) || ch == '\n')) {   \
        tokens[len].ch = ch;                   \
        break;                                 \
    }                                          \
    u6a_err_unprintable_ch(err_lex, ch);       \
    goto lex_failed

static const char* err_lex = "lex error";
static const char* info_lex = "lex";

bool
u6a_lex(FILE* restrict input_stream, struct u6a_token** token_arr, uint32_t* token_len) {
    uint32_t token_arr_size = U6A_TOKEN_INIT_LEN;
    struct u6a_token* tokens = malloc(token_arr_size * sizeof(struct u6a_token));
    if (UNLIKELY(tokens == NULL)) {
        u6a_err_bad_alloc(err_lex, token_arr_size * sizeof(struct u6a_token));
        return false;
    }
    int fn, ch;
    uint32_t len = 0;
    while (true) {
        fn = fgetc(input_stream);
        if (fn == '#') {
            do {
                fn = fgetc(input_stream);
            } while (fn != EOF && fn != '\n');
        }
        if (UNLIKELY(fn == EOF)) {
            break;
        }
        if (isspace(fn)) {
            continue;
        }
        if (UNLIKELY(len >= token_arr_size)) {
            token_arr_size *= 2;
            if (token_arr_size > U6A_TOKEN_MAX_LEN) {
                u6a_err_bad_alloc(err_lex, token_arr_size);
                goto lex_failed;
            }
            struct u6a_token* new_tokens = realloc(tokens, token_arr_size * sizeof(struct u6a_token));
            if (new_tokens == NULL) {
                u6a_err_bad_alloc(err_lex, token_arr_size * sizeof(struct u6a_token));
                goto lex_failed;
            }
            tokens = new_tokens;
        }
        switch (fn) {
            case '`':
                tokens[len].fn = u6a_tf_app;
                break;
            case 'S':
            case 's':
                tokens[len].fn = u6a_tf_s;
                break;
            case 'K':
            case 'k':
                tokens[len].fn = u6a_tf_k;
                break;
            case 'I':
            case 'i':
                tokens[len].fn = u6a_tf_i;
                break;
            case 'V':
            case 'v':
                tokens[len].fn = u6a_tf_v;
                break;
            case '.':
                tokens[len].fn = u6a_tf_out;
                READ_CH();
            case 'R':
            case 'r':
                tokens[len] = U6A_TOKEN(u6a_tf_out, '\n');
                break;
            case 'C':
            case 'c':
                tokens[len].fn = u6a_tf_c;
                break;
            case 'D':
            case 'd':
                tokens[len].fn = u6a_tf_d;
                break;
            case '?':
                tokens[len].fn = u6a_tf_cmp;
                READ_CH();
            case '@':
                tokens[len].fn = u6a_tf_in;
                break;
            case '|':
                tokens[len].fn = u6a_tf_pipe;
                break;
            case 'E':
            case 'e':
                tokens[len].fn = u6a_tf_e;
                break;
            default:
                u6a_err_bad_ch(err_lex, fn);
                lex_failed:
                free(tokens);
                return false;
        }
        ++len;
    }
    *token_arr = tokens;
    *token_len = len;
    u6a_info_verbose(info_lex, "completed, %" PRIu32 " tokens total", len);
    return true;
}
