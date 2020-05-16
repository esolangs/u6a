/*
 * u6ac.c - Unlambda compiler CLI
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
#include "lexer.h"
#include "parser.h"
#include "codegen.h"

#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <string.h>

#define EC_ERR_OPTIONS  1
#define EC_ERR_LEX      2
#define EC_ERR_PARSE    3
#define EC_ERR_CODEGEN  4

struct arg_options {
    FILE* input_file;
    char* input_file_name;
    FILE* output_file;
    char* output_file_prefix;
    char* output_file_name;
    bool  optimize_const;
    bool  dump_mnemonics;
    bool  print_only;
};

static const char* err_toplevel = "error";
static const char* info_toplevel = "info";

static void
arg_options_destroy(struct arg_options* options, bool delete_output_file) {
    if (options->input_file && options->input_file != stdin) {
        fclose(options->input_file);
    }
    const bool not_using_stdout = options->output_file != stdout;
    if (options->output_file && not_using_stdout) {
        fclose(options->output_file);
    }
    if (delete_output_file && not_using_stdout && options->output_file_name) {
        remove(options->output_file_name);
    }
}

static bool
process_options(struct arg_options* options, int argc, char** argv) {
    static const struct option long_opts[] = {
        { "out-file",    required_argument, NULL, 'o' },
        { "add-prefix",  optional_argument, NULL, 'p' },
        { "verbose",     no_argument,       NULL, 'v' },
        { "syntax-only", no_argument,       NULL, 's' },
        { "help",        no_argument,       NULL, 'H' },
        { "version",     no_argument,       NULL, 'V' },
        { 0, 0, 0, 0 }
    };
    options->optimize_const = false;
    bool syntax_only = false;
    bool verbose = false;
    char optimize_level = '1';
    while (true) {
        int result = getopt_long(argc, argv, "o:O::SvHV", long_opts, NULL);
        if (result == -1) {
            break;
        }
        switch (result) {
            case 'o':
                if (UNLIKELY(options->output_file_name)) {
                    break;
                }
                options->output_file_name = optarg;
                break;
            case 'O':
                optimize_level = optarg ? optarg[0] : '1';
            case 'p':
                if (UNLIKELY(options->output_file_prefix)) {
                    break;
                }
                options->output_file_prefix = optarg ? optarg : "#!/usr/bin/env u6a\n";
                break;
            case 'S':
                options->dump_mnemonics = true;
                break;
            case 'v':
                verbose = true;
                break;
            case 's':
                syntax_only = true;
                break;
            case 'H':
                printf("Usage: u6ac [options] source-file\n\n"
                       "Bytecode compiler for the Unlambda programming language.\n"
                       "See \"man u6ac\" for details.\n");
                options->print_only = true;
                break;
            case 'V':
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
    // Input file
    if (UNLIKELY(optind == argc)) {
        u6a_err_no_input_file(err_toplevel);
        return false;
    }
    options->input_file_name = argv[optind];
    uint32_t file_name_size = strlen(options->input_file_name);
    if (file_name_size == 1 && options->input_file_name[0] == '-') {
        options->input_file = stdin;
        options->input_file_name = "STDIN";
    } else if (UNLIKELY(file_name_size > PATH_MAX - 1)) {
        u6a_err_path_too_long(err_toplevel, PATH_MAX - 1, file_name_size);
        return false;
    } else {
        options->input_file = fopen(options->input_file_name, "r");
        if (options->input_file == NULL) {
            u6a_err_cannot_open_file(err_toplevel, options->input_file_name);
            return false;
        }
    }
    // Output file
    if (syntax_only) {
        if (UNLIKELY(options->output_file_name)) {
            options->output_file_name = NULL;
        }
    } else {
        if (options->output_file_name == NULL) {
            if (options->input_file == stdin) {
                goto write_to_stdout;
            } else {
                if (UNLIKELY(file_name_size + 8 > PATH_MAX - 1)) {
                    u6a_err_path_too_long(err_toplevel, PATH_MAX - 1, file_name_size + 8);
                    return false;
                }
                options->output_file_name = malloc((file_name_size + 9) * sizeof(char));
                strcpy(options->output_file_name, options->input_file_name);
                strcpy(options->output_file_name + file_name_size, options->dump_mnemonics ? ".bc.dump\0" : ".bc\0");
            }
        } else if (strlen(options->output_file_name) == 1 && options->output_file_name[0] == '-') {
            write_to_stdout:
            if (verbose) {
                u6a_err_custom(err_toplevel, "cannot write to STDOUT on verbose mode");
                return false;
            }
            options->output_file = stdout;
            options->output_file_name = "STDOUT";
        }
        if (options->output_file == NULL) {
            options->output_file = fopen(options->output_file_name, "w");
            if (options->output_file == NULL) {
                u6a_err_cannot_open_file(err_toplevel, options->output_file_name);
                return false;
            }
        }
    }
    if (optimize_level > '0') {
        options->optimize_const = true;
    }
    u6a_logging_verbose(verbose);
    return true;
}

int
main(int argc, char** argv) {
    struct arg_options options = { 0 };
    struct u6a_token* token_arr = 0;
    struct u6a_ast_node* ast_arr = 0;
    int exit_code = 0;
    u6a_logging_init(argv[0]);
    if (UNLIKELY(!process_options(&options, argc, argv))) {
        exit_code = EC_ERR_OPTIONS;
        goto terminate;
    }
    if (UNLIKELY(options.print_only)) {
        goto terminate;
    }
    u6a_info_verbose(info_toplevel, "reading source code from %s", options.input_file_name);
    uint32_t token_len;
    if (UNLIKELY(!u6a_lex(options.input_file, &token_arr, &token_len))) {
        exit_code = EC_ERR_LEX;
        goto terminate;
    }
    if (UNLIKELY(!u6a_parse(token_arr, token_len, &ast_arr))) {
        exit_code = EC_ERR_PARSE;
        goto terminate;
    }
    if (UNLIKELY(options.output_file == NULL)) {
        goto terminate;
    }
    u6a_codegen_init(options.output_file, options.output_file_name, options.optimize_const, options.dump_mnemonics);
    if (UNLIKELY(!u6a_write_prefix(options.output_file_prefix))) {
        exit_code = EC_ERR_CODEGEN;
        goto terminate;
    }
    if (UNLIKELY(!u6a_codegen(ast_arr, token_len + 2))) {
        exit_code = EC_ERR_CODEGEN;
        goto terminate;
    }

    terminate:
    arg_options_destroy(&options, exit_code);
    free(token_arr);
    free(ast_arr);
    return exit_code;
}
