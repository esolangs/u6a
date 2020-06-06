/*
 * defs.h - miscellaneous definitions
 * 
 * Copyright (C) 2020  CismonX <admin@cismon.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef U6A_DEFS_H_
#define U6A_DEFS_H_

#include "common.h"

#include <stdint.h>
#include <stddef.h>

#define U6A_TOKEN_FN_CHAR ( 1 << 4 )
#define U6A_TOKEN_FN_APP  ( 1 << 5 )

enum u6a_token_fn {
    u6a_tf_placeholder_,
    u6a_tf_k, u6a_tf_s, u6a_tf_i, u6a_tf_v, u6a_tf_c, u6a_tf_d, u6a_tf_e,
    u6a_tf_in,                       /* @ */
    u6a_tf_pipe,                     /* | */
    u6a_tf_out = U6A_TOKEN_FN_CHAR,  /* . */
    u6a_tf_cmp,                      /* ? */
    u6a_tf_app = U6A_TOKEN_FN_APP    /* ` */
};

struct u6a_token {
    uint8_t fn;
    uint8_t ch;
};

#define U6A_TOKEN(fn_, ch_) (struct u6a_token) { .fn = (fn_), .ch = (ch_) }
#define U6A_TOKEN_INIT_LEN  ( 4 * 1024 )
#define U6A_TOKEN_MAX_LEN   ( U6A_TOKEN_INIT_LEN * 1024 )

struct u6a_ast_node {
    struct u6a_token value;
    uint16_t         reserved_;
    uint32_t         sibling;        /* index of sibling when this is left child, otherwise 0 */
};

#define U6A_AN_FN(node)             (node)->value.fn
#define U6A_AN_CH(node)             (node)->value.ch
#define U6A_AN_LEFT(node)         ( (node) + 1 )
#define U6A_AN_RIGHT(node, head)  ( (head) + U6A_AN_LEFT(node)->sibling )

struct u6a_bc_header {
    struct {
        uint8_t  magic;              /* the "magic byte" for Unlambda bytecode files */
        uint8_t  ver_major;
        uint8_t  ver_minor;
        uint8_t  prog_header_size;   
    } file;
    struct {
        uint32_t text_size;          /* length of text segment (Bytes) */
        uint32_t rodata_size;        /* length of rodata segment (Bytes) */
    } prog;
};

#define U6A_BC_FILE_HEADER_SIZE sizeof(((struct u6a_bc_header*)NULL)->file)
#define U6A_BC_PROG_HEADER_SIZE sizeof(((struct u6a_bc_header*)NULL)->prog)

#endif
