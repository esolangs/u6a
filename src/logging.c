/*
 * logging.c - logging functions
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

#include "logging.h"

#include <stdio.h>
#include <stdarg.h>
#include <inttypes.h>
#include <ctype.h>

#define E_UNEXPECTED_EOF_AFTER "%s: [%s] unexpected end of file after "
#define E_UNRECOGNIZABLE_CHAR  "%s: [%s] unrecognizable character "

const char* prog_name;
bool verbose = false;

void
u6a_logging_init(const char* prog_name_) {
    prog_name = prog_name_;
}

void
u6a_logging_verbose(bool verbose_) {
    verbose = verbose_;
}

U6A_COLD void
u6a_err_bad_alloc(const char* stage, size_t size) {
    fprintf(stderr, "%s: [%s] allocation failed - trying to allocate %zu bytes.\n", prog_name, stage, size);
}

U6A_COLD void
u6a_err_unexpected_eof(const char* stage, int after) {
    fprintf(stderr, E_UNEXPECTED_EOF_AFTER "'%c'.\n", prog_name, stage, (char)after);
}

U6A_COLD void
u6a_err_unprintable_ch(const char* stage, int got) {
    fprintf(stderr, "%s: [%s] printable character or '\\n' expected, 0x%02x given.\n", prog_name, stage, got);
}

U6A_COLD void
u6a_err_bad_ch(const char* stage, int got) {
    if (isprint(got)) {
        fprintf(stderr, E_UNRECOGNIZABLE_CHAR "'%c'.\n", prog_name, stage, (char)got);
    } else if (LIKELY(got == '\n')) {
        fprintf(stderr, E_UNRECOGNIZABLE_CHAR "'\\n'.\n", prog_name, stage);
    } else {
        fprintf(stderr, E_UNRECOGNIZABLE_CHAR "0x%02x.\n", prog_name, stage, got);
    }
}

U6A_COLD void
u6a_err_bad_syntax(const char* stage) {
    fprintf(stderr, "%s: [%s] bad syntax.\n", prog_name, stage);
}

U6A_COLD void
u6a_err_write_failed(const char* stage, size_t bytes, const char* filename) {
    if (bytes > 0) {
        fprintf(stderr, "%s: [%s] failed writing %zu bytes to %s.\n", prog_name, stage, bytes, filename);
    } else {
        fprintf(stderr, "%s: [%s] failed writing data to %s.\n", prog_name, stage, filename);
    }
}

U6A_COLD void
u6a_err_path_too_long(const char* stage, size_t maximum, size_t given) {
    fprintf(stderr, "%s: [%s] file path too long. A maximum of %zu expected, %zu given.\n",
        prog_name, stage, maximum, given);
}

U6A_COLD void
u6a_err_no_input_file(const char* stage) {
    fprintf(stderr, "%s: [%s] no input file specified.\n", prog_name, stage);
}

U6A_COLD void
u6a_err_custom(const char* stage, const char* err_message) {
    fprintf(stderr, "%s: [%s] %s.\n", prog_name, stage, err_message);
}

U6A_COLD void
u6a_err_cannot_open_file(const char* stage, const char* filename) {
    fprintf(stderr, "%s: [%s] failed to open file %s.\n", prog_name, stage, filename);
}

U6A_COLD void
u6a_err_stack_underflow(const char* stage) {
    fprintf(stderr, "%s: [%s] stack underflow.\n", prog_name, stage);
}

U6A_COLD void
u6a_err_invalid_uint(const char* stage, const char* str) {
    fprintf(stderr, "%s: [%s] \"%s\" is not a valid unsigned integer.\n", prog_name, stage, str);
}

U6A_COLD void
u6a_err_uint_not_in_range(const char* stage, uint32_t min_val, uint32_t max_val, uint32_t got) {
    fprintf(stderr, "%s: [%s] Integer out of range - [%" PRIu32 ", %" PRIu32 "] expected, %" PRIu32 " given.\n",
        prog_name, stage, min_val, max_val, got);
}

U6A_COLD void
u6a_err_invalid_bc_file(const char* stage, const char* filename) {
    fprintf(stderr, "%s: [%s] %s is not a valid Unlambda bytecode file.\n", prog_name, stage, filename);
}

U6A_COLD void
u6a_err_bad_bc_ver(const char* stage, const char* filename, int ver_major, int ver_minor) {
    fprintf(stderr, "%s: [%s] bytecode file %s version %d.%d is not compatible.\n",
        prog_name, stage, filename, ver_major, ver_minor);
}

U6A_COLD void
u6a_err_vm_pool_oom(const char* stage) {
    fprintf(stderr, "%s: [%s] VM object pool memory exhausted.\n", prog_name, stage);
}

U6A_COLD void
u6a_err_invalid_opcode(const char* stage, int opcode) {
    fprintf(stderr, "%s: [%s] invalid opcode %d.\n", prog_name, stage, opcode);
}

U6A_COLD void
u6a_err_invalid_ex_opcode(const char* stage, int ex_opcode) {
    fprintf(stderr, "%s: [%s] invalid extended opcode %d.\n", prog_name, stage, ex_opcode);
}

U6A_COLD void
u6a_err_invalid_vm_func(const char* stage, int fn) {
    fprintf(stderr, "%s: [%s] invalid function %d.\n", prog_name, stage, fn);
}

U6A_COLD void
u6a_info_verbose_(const char* format, ...) {
    if (verbose) {
        va_list args;
        va_start(args, format);
        vprintf(format, args);
        va_end(args);
    }
}

U6A_COLD const char*
u6a_logging_get_prog_name_() {
    return prog_name;
}
