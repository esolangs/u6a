/*
 * logging.h - logging functions definitions
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

#ifndef U6A_LOGGING_H_
#define U6A_LOGGING_H_

#include "common.h"

#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

void
u6a_logging_init(const char* prog_name);

void
u6a_logging_verbose(bool verbose);

void
u6a_err_bad_alloc(const char* stage, size_t size);

void
u6a_err_unexpected_eof(const char* stage, int after);

void
u6a_err_unprintable_ch(const char* stage, int got);

void
u6a_err_bad_ch(const char* stage, int got);

void
u6a_err_bad_syntax(const char* stage);

void
u6a_err_write_failed(const char* stage, size_t bytes, const char* filename);

void
u6a_err_path_too_long(const char* stage, size_t maximum, size_t given);

void
u6a_err_no_input_file(const char* stage);

void
u6a_err_custom(const char* stage, const char* err_message);

void
u6a_err_cannot_open_file(const char* stage, const char* filename);

void
u6a_err_stack_underflow(const char* stage);

void
u6a_err_invalid_uint(const char* stage, const char* str);

void
u6a_err_uint_not_in_range(const char* stage, uint32_t min_val, uint32_t max_val, uint32_t got);

void
u6a_err_invalid_bc_file(const char* stage, const char* filename);

void
u6a_err_bad_bc_ver(const char* stage, const char* filename, int ver_major, int ver_minor);

void
u6a_err_vm_pool_oom(const char* stage);

void
u6a_err_invalid_opcode(const char* stage, int opcode);

void
u6a_err_invalid_ex_opcode(const char* stage, int ex_opcode);

void
u6a_err_invalid_vm_func(const char* stage, int fn);

void
u6a_info_verbose_(const char* format, ...);

const char*
u6a_logging_get_prog_name_();

#define u6a_info_verbose(stage, info_message, ...) \
    u6a_info_verbose_("%s: [%s] " info_message ".\n", u6a_logging_get_prog_name_(), stage, ##__VA_ARGS__)

#endif
