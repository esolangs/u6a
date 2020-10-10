/*
 * vm_stack.h - Unlambda VM segmented stacks definitions
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

#ifndef U6A_VM_STACK_H_
#define U6A_VM_STACK_H_

#include "common.h"
#include "vm_defs.h"

#include <stdint.h>
#include <stdbool.h>
#include <setjmp.h>

struct u6a_vm_stack {
    struct u6a_vm_stack* prev;
    uint32_t             top;
    uint32_t             refcnt;
    struct u6a_vm_var_fn elems[];
};

struct u6a_vm_stack_ctx {
    struct u6a_vm_stack*    active_stack;
    uint32_t                stack_seg_len;
    struct u6a_vm_pool_ctx* pool_ctx;
    jmp_buf*                jmp_ctx;
    const char*             err_stage;
};

bool
u6a_vm_stack_init(struct u6a_vm_stack_ctx* ctx, uint32_t stack_seg_len, jmp_buf* jmp_ctx, const char* err_stage);

static inline struct u6a_vm_var_fn
u6a_vm_stack_top(struct u6a_vm_stack_ctx* ctx) {
    struct u6a_vm_stack* vs = ctx->active_stack;
    if (UNLIKELY(vs->top == UINT32_MAX)) {
        vs = vs->prev;
        if (UNLIKELY(vs == NULL)) {
            U6A_VM_ERR(ctx);
        }
        ctx->active_stack = vs;
    }
    return vs->elems[vs->top];
}

void
u6a_vm_stack_push1_split_(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0);

static inline void
u6a_vm_stack_push1(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0) {
    struct u6a_vm_stack* vs = ctx->active_stack;
    if (LIKELY(vs->top + 1 < ctx->stack_seg_len)) {
        vs->elems[++vs->top] = v0;
    } else {
        u6a_vm_stack_push1_split_(ctx, v0);
    }
}

void
u6a_vm_stack_push2_split_(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1);

static inline void
u6a_vm_stack_push2(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1) {
    struct u6a_vm_stack* vs = ctx->active_stack;
    if (LIKELY(vs->top + 2 < ctx->stack_seg_len)) {
        vs->elems[++vs->top] = v0;
        vs->elems[++vs->top] = v1;
    } else {
        u6a_vm_stack_push2_split_(ctx, v0, v1);
    }
}

void
u6a_vm_stack_push3_split_(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1,
                          struct u6a_vm_var_fn v2);

static inline void
u6a_vm_stack_push3(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1,
                   struct u6a_vm_var_fn v2)
{
    struct u6a_vm_stack* vs = ctx->active_stack;
    if (LIKELY(vs->top + 3 < ctx->stack_seg_len)) {
        vs->elems[++vs->top] = v0;
        vs->elems[++vs->top] = v1;
        vs->elems[++vs->top] = v2;
    } else {
        u6a_vm_stack_push3_split_(ctx, v0, v1, v2);
    }
}

void
u6a_vm_stack_push4_split_(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1,
                          struct u6a_vm_var_fn v2, struct u6a_vm_var_fn v3);

static inline void
u6a_vm_stack_push4(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1,
                   struct u6a_vm_var_fn v2, struct u6a_vm_var_fn v3)
{
    struct u6a_vm_stack* vs = ctx->active_stack;
    if (LIKELY(vs->top + 4 < ctx->stack_seg_len)) {
        vs->elems[++vs->top] = v0;
        vs->elems[++vs->top] = v1;
        vs->elems[++vs->top] = v2;
        vs->elems[++vs->top] = v3;
    } else {
        u6a_vm_stack_push4_split_(ctx, v0, v1, v2, v3);
    }
}

void
u6a_vm_stack_pop_split_(struct u6a_vm_stack_ctx* ctx);

static inline void
u6a_vm_stack_pop(struct u6a_vm_stack_ctx* ctx) {
    struct u6a_vm_stack* vs = ctx->active_stack;
    if (UNLIKELY(vs->top-- == UINT32_MAX)) {
        u6a_vm_stack_pop_split_(ctx);
    }
}

struct u6a_vm_var_fn
u6a_vm_stack_xch_split_(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0);

static inline struct u6a_vm_var_fn
u6a_vm_stack_xch(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0) {
    struct u6a_vm_stack* vs = ctx->active_stack;
    struct u6a_vm_var_fn elem;
    // XCH on segmented stacks is inefficient, perhaps there's a better solution?
    if (LIKELY(vs->top != 0 && vs->top != UINT32_MAX)) {
        elem = vs->elems[vs->top - 1];
        vs->elems[vs->top - 1] = v0;
    } else {
        elem = u6a_vm_stack_xch_split_(ctx, v0);
    }
    return elem;
}

struct u6a_vm_stack*
u6a_vm_stack_dup(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_stack* vs);

static inline struct u6a_vm_stack*
u6a_vm_stack_save(struct u6a_vm_stack_ctx* ctx) {
    return u6a_vm_stack_dup(ctx, ctx->active_stack);
}

void
u6a_vm_stack_discard(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_stack* vs);

void
u6a_vm_stack_destroy(struct u6a_vm_stack_ctx* ctx);

static inline void
u6a_vm_stack_resume(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_stack* vs) {
    u6a_vm_stack_destroy(ctx);
    ctx->active_stack = vs;
}

#endif
