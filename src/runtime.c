/*
 * runtime.c - Unlambda runtime
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

#include "runtime.h"
#include "logging.h"
#include "vm_defs.h"
#include "vm_stack.h"
#include "vm_pool.h"

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <setjmp.h>

static struct u6a_vm_ins*      text;
static        uint32_t         text_len;
static        char*            rodata;
static        uint32_t         rodata_len;
static        bool             force_exec;
static struct u6a_vm_stack_ctx stack_ctx;
static struct u6a_vm_pool_ctx  pool_ctx;
static        jmp_buf          jmp_ctx;

static const struct u6a_vm_ins text_subst[] = {
    { .opcode = u6a_vo_la  },
    { .opcode = u6a_vo_xch },
    { .opcode = u6a_vo_la  },
    { .opcode = u6a_vo_la  },
    { .opcode = u6a_vo_la  }
};
static const uint32_t text_subst_len = sizeof(text_subst) / sizeof(struct u6a_vm_ins);

static const char* err_runtime = "runtime error";

#define CHECK_BC_HEADER_VER(file_header)       \
    ( (file_header).ver_major == U6A_VER_MAJOR && (file_header).ver_minor == U6A_VER_MINOR )

#define ACC_FN_REF(fn_, ref_)                  \
    acc = U6A_VM_VAR_FN_REF(fn_, ref_)
#define VM_JMP(dest)                           \
    ins = text + (dest);                       \
    continue
#define CHECK_FORCE(log_func, err_val)         \
    if (!force_exec) {                         \
        log_func(err_runtime, err_val);        \
        goto runtime_error;                    \
    }

#define VM_VAR_JMP       U6A_VM_VAR_FN_REF(u6a_vf_j, ins - text)
#define VM_VAR_FINALIZE  U6A_VM_VAR_FN_REF(u6a_vf_f, ins - text)

#define STACK_PUSH1(fn_0)                    u6a_vm_stack_push1(&stack_ctx, fn_0)
#define STACK_PUSH2(fn_0, fn_1)              u6a_vm_stack_push2(&stack_ctx, fn_0, fn_1)
#define STACK_PUSH3(fn_0, fn_1, fn_2)        u6a_vm_stack_push3(&stack_ctx, fn_0, fn_1, fn_2)
#define STACK_PUSH4(fn_0, fn_1, fn_2, fn_3)  u6a_vm_stack_push4(&stack_ctx, fn_0, fn_1, fn_2, fn_3)
#define STACK_XCH(fn_0)                      u6a_vm_stack_xch(&stack_ctx, fn_0)
#define STACK_POP(var)                         \
    vm_var_fn_free(top);                       \
    var = top = u6a_vm_stack_top(&stack_ctx);  \
    u6a_vm_stack_pop(&stack_ctx)

#define POOL_ALLOC1(v1)             u6a_vm_pool_alloc1(&pool_ctx, v1)
#define POOL_ALLOC2(v1, v2)         u6a_vm_pool_alloc2(&pool_ctx, v1, v2)
#define POOL_ALLOC2_PTR(v1, v2)     u6a_vm_pool_alloc2_ptr(&pool_ctx, v1, v2)
#define POOL_GET1(offset)           u6a_vm_pool_get1(pool_ctx.active_pool, offset)
#define POOL_GET2(offset)           u6a_vm_pool_get2(pool_ctx.active_pool, offset)
#define POOL_GET2_SEPARATE(offset)  u6a_vm_pool_get2_separate(&pool_ctx, offset)

static inline bool
read_bc_header(struct u6a_bc_header* restrict header, FILE* restrict input_stream) {
    int ch;
    do {
        ch = fgetc(input_stream);
        if (UNLIKELY(ch == EOF)) {
            return false;
        }
    } while (ch != U6A_MAGIC);
    if (UNLIKELY(ch != ungetc(ch, input_stream))) {
        return false;
    }
    if (UNLIKELY(1 != fread(&header->file, U6A_BC_FILE_HEADER_SIZE, 1, input_stream))) {
        return false;
    }
    if (LIKELY(header->file.prog_header_size >= U6A_BC_FILE_HEADER_SIZE)) {
        return 1 == fread(&header->prog, header->file.prog_header_size, 1, input_stream);
    }
    return true;
}

static inline struct u6a_vm_var_fn
vm_var_fn_addref(struct u6a_vm_var_fn var) {
    if (var.token.fn & U6A_VM_FN_REF) {
        u6a_vm_pool_addref(pool_ctx.active_pool, var.ref);
    }
    return var;
}

static inline void
vm_var_fn_free(struct u6a_vm_var_fn var) {
    if (var.token.fn & U6A_VM_FN_REF) {
        u6a_vm_pool_free(&pool_ctx, var.ref);
    }
}

bool
u6a_runtime_info(FILE* restrict input_stream, const char* file_name) {
    struct u6a_bc_header header;
    if (UNLIKELY(!read_bc_header(&header, input_stream))) {
        u6a_err_invalid_bc_file(err_runtime, file_name);
        return false;
    }
    printf("Version: %d.%d.*\n", header.file.ver_major, header.file.ver_minor);
    if (LIKELY(CHECK_BC_HEADER_VER(header.file))) {
        if (LIKELY(header.file.prog_header_size == U6A_BC_PROG_HEADER_SIZE)) {
            printf("Size of section .text   (bytes): %" PRIu32 "\n", ntohl(header.prog.text_size));
            printf("Size of section .rodata (bytes): %" PRIu32 "\n", ntohl(header.prog.rodata_size));
        } else {
            printf("Program header unrecognizable (%d bytes)\n", header.file.prog_header_size);
        }
    }
    return true;
}

bool
u6a_runtime_init(struct u6a_runtime_options* options) {
    struct u6a_bc_header header;
    if (UNLIKELY(!read_bc_header(&header, options->istream))) {
        u6a_err_invalid_bc_file(err_runtime, options->file_name);
        return false;
    }
    if (UNLIKELY(!CHECK_BC_HEADER_VER(header.file))) {
        if (!options->force_exec || header.file.prog_header_size != U6A_BC_FILE_HEADER_SIZE) {
            u6a_err_bad_bc_ver(err_runtime, options->file_name, header.file.ver_major, header.file.ver_minor);
            return false;
        }
    }
    header.prog.text_size = ntohl(header.prog.text_size);
    header.prog.rodata_size = ntohl(header.prog.rodata_size);
    text = malloc(header.prog.text_size + sizeof(text_subst));
    if (UNLIKELY(text == NULL)) {
        u6a_err_bad_alloc(err_runtime, header.prog.text_size + sizeof(text_subst));
        return false;
    }
    text_len = header.prog.text_size / sizeof(struct u6a_vm_ins);
    rodata = malloc(header.prog.rodata_size);
    if (UNLIKELY(rodata == NULL)) {
        u6a_err_bad_alloc(err_runtime, header.prog.rodata_size);
        free(text);
        return false;
    }
    rodata_len = header.prog.rodata_size / sizeof(char);
    memcpy(text, text_subst, sizeof(text_subst));
    if (UNLIKELY(text_len != fread(text + text_subst_len, sizeof(struct u6a_vm_ins), text_len, options->istream))) {
        goto runtime_init_failed;
    }
    if (UNLIKELY(rodata_len != fread(rodata, sizeof(char), rodata_len, options->istream))) {
        goto runtime_init_failed;
    }
    if (UNLIKELY(!u6a_vm_stack_init(&stack_ctx, options->stack_segment_size, &jmp_ctx, err_runtime))) {
        goto runtime_init_failed;
    }
    if (UNLIKELY(!u6a_vm_pool_init(&pool_ctx, options->pool_size, text_len, &jmp_ctx, err_runtime))) {
        goto runtime_init_failed;
    }
    stack_ctx.pool_ctx = &pool_ctx;
    pool_ctx.stack_ctx = &stack_ctx;
    for (struct u6a_vm_ins* ins = text + text_subst_len; ins < text + text_len; ++ins) {
        if (ins->opcode & U6A_VM_OP_OFFSET) {
            ins->operand.offset = ntohl(ins->operand.offset);
        }
    }
    force_exec = options->force_exec;
    return true;

    runtime_init_failed:
    u6a_runtime_destroy();
    return false;
}

U6A_HOT struct u6a_vm_var_fn
u6a_runtime_execute(FILE* restrict istream, FILE* restrict ostream) {
    struct u6a_vm_var_fn acc = { 0 }, top = { 0 }, func = { 0 }, arg = { 0 };
    struct u6a_vm_ins* ins = text + text_subst_len;
    int current_char = EOF;
    struct u6a_vm_var_tuple tuple;
    void* cont;
    if (setjmp(jmp_ctx)) {
        goto runtime_error;
    }
    while (true) {
        switch (ins->opcode) {
            case u6a_vo_app:
                if (ins->operand.fn.first.fn) {
                    func.token = ins->operand.fn.first;
                } else {
                    func = acc;
                    goto arg_from_ins;
                }
                if (ins->operand.fn.second.fn) {
                    arg_from_ins:
                    arg.token = ins->operand.fn.second;
                } else {
                    arg = acc;
                }
                goto do_apply;
            case u6a_vo_la:
                STACK_POP(func);
                arg = acc;
                do_apply:
                switch (func.token.fn) {
                    case u6a_vf_s:
                        ACC_FN_REF(u6a_vf_s1, POOL_ALLOC1(vm_var_fn_addref(arg)));
                        break;
                    case u6a_vf_s1:
                        vm_var_fn_addref(arg);
                        ACC_FN_REF(u6a_vf_s2, POOL_ALLOC2(vm_var_fn_addref(POOL_GET1(func.ref).fn), arg));
                        break;
                    case u6a_vf_s2:
                        tuple = POOL_GET2(func.ref);
                        vm_var_fn_addref(tuple.v1.fn);
                        vm_var_fn_addref(tuple.v2.fn);
                        vm_var_fn_addref(arg);
                        // Tail call elimination
                        if (ins - text == 0x03) {
                            STACK_PUSH3(arg, tuple.v2.fn, tuple.v1.fn);
                        } else {
                            STACK_PUSH4(VM_VAR_JMP, arg, tuple.v2.fn, tuple.v1.fn);
                        }
                        acc = arg;
                        VM_JMP(0x00);
                    case u6a_vf_k:
                        ACC_FN_REF(u6a_vf_k1, POOL_ALLOC1(vm_var_fn_addref(arg)));
                        break;
                    case u6a_vf_k1:
                        acc = vm_var_fn_addref(POOL_GET1(func.ref).fn);
                        break;
                    case u6a_vf_i:
                        acc = arg;
                        break;
                    case u6a_vf_out:
                        acc = arg;
                        fputc(func.token.ch, ostream);
                        break;
                    case u6a_vf_j:
                        acc = arg;
                        ins = text + func.ref;
                        break;
                    case u6a_vf_f:
                        ins = text + func.ref;
                        STACK_POP(acc);
                        STACK_PUSH2(U6A_VM_VAR_FN_REF(u6a_vf_j, func.ref), vm_var_fn_addref(arg));
                        VM_JMP(0x03);
                    case u6a_vf_c:
                        cont = u6a_vm_stack_save(&stack_ctx);
                        STACK_PUSH2(VM_VAR_JMP, vm_var_fn_addref(arg));
                        ACC_FN_REF(u6a_vf_c1, POOL_ALLOC2_PTR(cont, ins));
                        VM_JMP(0x03);
                    case u6a_vf_d:
                        ACC_FN_REF(u6a_vf_d1_c, POOL_ALLOC1(vm_var_fn_addref(arg)));
                        break;
                    case u6a_vf_c1:
                        tuple = POOL_GET2_SEPARATE(func.ref);
                        u6a_vm_stack_resume(&stack_ctx, tuple.v1.ptr);
                        ins = tuple.v2.ptr;
                        acc = arg;
                        break;
                    case u6a_vf_d1_c:
                        STACK_PUSH2(VM_VAR_JMP, vm_var_fn_addref(POOL_GET1(func.ref).fn));
                        acc = arg;
                        VM_JMP(0x03);
                    case u6a_vf_d1_s:
                        tuple = POOL_GET2(func.ref);
                        STACK_PUSH3(vm_var_fn_addref(arg), VM_VAR_FINALIZE, vm_var_fn_addref(tuple.v1.fn));
                        acc = tuple.v2.fn;
                        VM_JMP(0x03);
                    case u6a_vf_d1_d:
                        STACK_PUSH2(vm_var_fn_addref(arg), VM_VAR_FINALIZE);
                        VM_JMP(func.ref);
                    case u6a_vf_v:
                        acc.token.fn = u6a_vf_v;
                        break;
                    case u6a_vf_p:
                        acc = arg;
                        fputs(rodata + func.ref, ostream);
                        break;
                    case u6a_vf_in:
                        current_char = fgetc(istream);
                        STACK_PUSH2(VM_VAR_JMP, vm_var_fn_addref(arg));
                        if (UNLIKELY(current_char == EOF)) {
                            arg.token.fn = u6a_vf_v;
                        } else {
                            arg.token.fn = u6a_vf_i;
                        }
                        acc = arg;
                        VM_JMP(0x03);
                    case u6a_vf_cmp:
                        STACK_PUSH2(VM_VAR_JMP, vm_var_fn_addref(arg));
                        arg.token.fn = func.token.ch == current_char ? u6a_vf_i : u6a_vf_v;
                        acc = arg;
                        VM_JMP(0x03);
                    case u6a_vf_pipe:
                        STACK_PUSH2(VM_VAR_JMP, vm_var_fn_addref(arg));
                        if (UNLIKELY(current_char == EOF)) {
                            arg.token.fn = u6a_vf_v;
                        } else {
                            arg.token = U6A_TOKEN(u6a_vf_out, current_char);
                        }
                        acc = arg;
                        VM_JMP(0x03);
                    case u6a_vf_e:
                        // Every program should terminate with explicit `e` function
                        return arg;
                    default:
                        CHECK_FORCE(u6a_err_invalid_vm_func, func.token.fn);
                }
                break;
            case u6a_vo_sa:
                if (UNLIKELY(acc.token.fn == u6a_vf_d)) {
                    goto delay;
                }
                STACK_PUSH1(vm_var_fn_addref(acc));
                break;
            case u6a_vo_xch:
                if (UNLIKELY(acc.token.fn == u6a_vf_d)) {
                    STACK_POP(func);
                    vm_var_fn_addref(func);
                    STACK_POP(arg);
                    ACC_FN_REF(u6a_vf_d1_s, POOL_ALLOC2(func, vm_var_fn_addref(arg)));
                } else {
                    acc = STACK_XCH(acc);
                }
                break;
            case u6a_vo_del:
                delay:
                acc = U6A_VM_VAR_FN_REF(u6a_vf_d1_d, ins + 1 - text);
                VM_JMP(text_subst_len + ins->operand.offset);
            case u6a_vo_lc:
                switch (ins->opcode_ex) {
                    case u6a_vo_ex_print:
                        acc = U6A_VM_VAR_FN_REF(u6a_vf_p, ins->operand.offset);
                        break;
                    default:
                        CHECK_FORCE(u6a_err_invalid_ex_opcode, ins->opcode_ex);
                }
                break;
            default:
                CHECK_FORCE(u6a_err_invalid_opcode, ins->opcode);
        }
        ++ins;
    }

    runtime_error:
    return U6A_VM_VAR_FN_EMPTY;
}

void
u6a_runtime_destroy() {
    free(text);
    free(rodata);
    text = NULL;
    rodata = NULL;
}
