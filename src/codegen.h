/*
 * codegen.h - Unlambda VM bytecode generator definitions
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

#ifndef U6A_CODEGEN_H_
#define U6A_CODEGEN_H_

#include "common.h"
#include "defs.h"

#include <stdbool.h>
#include <stdio.h>

struct u6a_codegen_options {
    FILE* output_stream;
    char* file_name;
    bool  optimize_const;
    bool  dump_mnemonics;
};

bool
u6a_write_prefix(const struct u6a_codegen_options* options, const char* prefix_string);

bool
u6a_codegen(const struct u6a_codegen_options* options, struct u6a_ast_node* ast_arr, uint32_t ast_len);

#endif
