/*
 * u6a.c - Unlambda runtime CLI
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
#include "vm_defs.h"
#include "runtime.h"

#include <string.h>
#include <stdlib.h>
#include <getopt.h>
#include <errno.h>

#define EC_ERR_OPTIONS  1
#define EC_ERR_INIT     2
#define EC_ERR_RUNTIME  3

#define PARSE_UINT_OPT(opt, min_val, max_val)                            \
    errno = 0;                                                           \
    (opt) = strtoul(optarg, NULL, 10);                                   \
    if (UNLIKELY(errno)) {                                               \
        u6a_err_invalid_uint(err_toplevel, optarg);                      \
        return false;                                                    \
    }                                                                    \
    if (UNLIKELY((opt) < (min_val) || (opt) > (max_val))) {              \
        u6a_err_uint_not_in_range(err_toplevel, min_val, max_val, opt);  \
        return false;                                                    \
    }

struct arg_options {
    struct u6a_runtime_options runtime;
    bool                       print_info;
    bool                       print_only;
};

static const char* err_toplevel = "error";
static const char* info_toplevel = "info";

static void
arg_options_destroy(struct arg_options* options) {
    if (options->runtime.istream && options->runtime.istream != stdin) {
        fclose(options->runtime.istream);
    }
}

static bool
process_options(struct arg_options* options, int argc, char** argv) {
    static const struct option long_opts[] = {
        { "stack-segment-size", required_argument, NULL, 's' },
        { "pool-size",          required_argument, NULL, 'p' },
        { "info",               no_argument,       NULL, 'i' },
        { "force",              no_argument,       NULL, 'f' },
        { "help",               no_argument,       NULL, 'H' },
        { "version",            no_argument,       NULL, 'V' },
        { 0, 0, 0, 0 }
    };
    options->runtime.stack_segment_size = U6A_VM_DEFAULT_STACK_SEGMENT_SIZE;
    options->runtime.pool_size = U6A_VM_DEFAULT_POOL_SIZE;
    options->print_info = false;
    while (true) {
        int result = getopt_long(argc, argv, "s:p:ifHV", long_opts, NULL);
        if (result == -1) {
            break;
        }
        switch (result) {
            case 's':
                PARSE_UINT_OPT(options->runtime.stack_segment_size,
                    U6A_VM_MIN_STACK_SEGMENT_SIZE, U6A_VM_MAX_STACK_SEGMENT_SIZE);
                break;
            case 'p':
                PARSE_UINT_OPT(options->runtime.pool_size, U6A_VM_MIN_POOL_SIZE, U6A_VM_MAX_POOL_SIZE);
                break;
            case 'i':
                options->print_info = true;
                break;
            case 'f':
                options->runtime.force_exec = true;
                break;
            case 'H':
                printf("Usage: u6a [options] bytecode-file\n\n"
                       "Runtime for the Unlambda programming language.\n"
                       "See \"man u6a\" for details.\n");
                options->print_only = true;
                break;
            case 'v':
                printf("%d.%d.%d\n", U6A_VER_MAJOR, U6A_VER_MINOR, U6A_VER_PATCH);
                options->print_only = true;
                break;
            case '?':
                return false;
            default:
                U6A_NOT_REACHED();
        }
    }
    if (UNLIKELY(options->print_only)) {
        return true;
    }
    if (UNLIKELY(optind == argc)) {
        u6a_err_no_input_file(err_toplevel);
        return false;
    }
    options->runtime.file_name = argv[optind];
    uint32_t file_name_size = strlen(options->runtime.file_name);
    if (file_name_size == 1 && options->runtime.file_name[0] == '-') {
        options->runtime.istream = stdin;
        options->runtime.file_name = "STDIN";
    } else if (UNLIKELY(file_name_size > PATH_MAX - 1)) {
        u6a_err_path_too_long(err_toplevel, PATH_MAX - 1, file_name_size);
        return false;
    } else {
        options->runtime.istream = fopen(options->runtime.file_name, "r");
        if (options->runtime.istream == NULL) {
            u6a_err_cannot_open_file(err_toplevel, options->runtime.file_name);
            return false;
        }
    }
    return true;
}

int main(int argc, char** argv) {
    struct arg_options options = { 0 };
    int exit_code = 0;
    u6a_logging_init(argv[0]);
    if (UNLIKELY(!process_options(&options, argc, argv))) {
        exit_code = EC_ERR_OPTIONS;
        goto terminate;
    }
    if (options.print_only) {
        goto terminate;
    }
    if (options.print_info) {
        if (UNLIKELY(!u6a_runtime_info(options.runtime.istream, options.runtime.file_name))) {
            exit_code = EC_ERR_INIT;
        }
        goto terminate;
    }
    if (UNLIKELY(!u6a_runtime_init(&options.runtime))) {
        exit_code = EC_ERR_INIT;
        goto terminate;
    }
    struct u6a_vm_var_fn exec_result = u6a_runtime_execute(stdin, stdout);
    if (UNLIKELY(U6A_VM_VAR_FN_IS_EMPTY(exec_result))) {
        exit_code = EC_ERR_RUNTIME;
        goto terminate;
    }

    terminate:
    u6a_runtime_destroy();
    arg_options_destroy(&options);
    return exit_code;
}
