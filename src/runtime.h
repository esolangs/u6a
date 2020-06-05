/*
 * runtime.h - Unlambda runtime definitions
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

#ifndef U6A_RUNTIME_H_
#define U6A_RUNTIME_H_

#include "common.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

struct u6a_runtime_options {
    FILE*    istream;
    char*    file_name;
    uint32_t stack_segment_size;
    uint32_t pool_size;
    bool     force_exec;
};

bool
u6a_runtime_info(FILE* restrict istream, const char* file_name);

bool
u6a_runtime_init(struct u6a_runtime_options* options);

struct u6a_vm_var_fn
u6a_runtime_execute(FILE* restrict istream, FILE* restrict output_stream);

void
u6a_runtime_destroy();

#endif
