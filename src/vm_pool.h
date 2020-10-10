/*
 * vm_pool.h - Unlambda VM object pool definitions
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

#ifndef U6A_VM_POOL_H_
#define U6A_VM_POOL_H_

#include "common.h"
#include "vm_defs.h"

#include <stdint.h>
#include <stdbool.h>
#include <setjmp.h>

struct u6a_vm_pool_elem {
    struct u6a_vm_var_tuple values;
    uint32_t                refcnt;
    uint32_t                flags;
};

#define U6A_VM_POOL_ELEM_HOLDS_PTR ( 1 << 0 )

struct u6a_vm_pool {
    uint32_t pos;
    struct u6a_vm_pool_elem elems[];
};

struct u6a_vm_pool_elem_ptrs {
    uint32_t pos;
    struct u6a_vm_pool_elem* elems[];
};

struct u6a_vm_pool_ctx {
    struct u6a_vm_pool*           active_pool;
    struct u6a_vm_pool_elem_ptrs* holes;
    struct u6a_vm_pool_elem**     fstack;
    struct u6a_vm_stack_ctx*      stack_ctx;
    uint32_t                      pool_len;
    uint32_t                      fstack_top;
    jmp_buf*                      jmp_ctx;
    const char*                   err_stage;
};

// Forward declarations

struct u6a_vm_stack*
u6a_vm_stack_dup(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_stack* vs);

void
u6a_vm_stack_discard(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_stack* vs);

void
u6a_err_vm_pool_oom(const char* stage);

static inline void
u6a_free_stack_push_(struct u6a_vm_pool_ctx* ctx, struct u6a_vm_var_fn fn) {
    if (fn.token.fn & U6A_VM_FN_REF) {
        ctx->fstack[++ctx->fstack_top] = ctx->active_pool->elems + fn.ref;
    }
}

static inline struct u6a_vm_pool_elem*
u6a_free_stack_pop_(struct u6a_vm_pool_ctx* ctx) {
    if (ctx->fstack_top == UINT32_MAX) {
        return NULL;
    }
    return ctx->fstack[ctx->fstack_top--];
}

static inline struct u6a_vm_pool_elem*
u6a_vm_pool_elem_alloc_(struct u6a_vm_pool_ctx* ctx) {
    struct u6a_vm_pool* pool = ctx->active_pool;
    struct u6a_vm_pool_elem_ptrs* holes = ctx->holes;
    struct u6a_vm_pool_elem* new_elem;
    if (ctx->holes->pos == UINT32_MAX) {
        if (UNLIKELY(++pool->pos == ctx->pool_len)) {
            u6a_err_vm_pool_oom(ctx->err_stage);
            U6A_VM_ERR(ctx);
        }
        new_elem = pool->elems + pool->pos;
    } else {
        new_elem = holes->elems[holes->pos--];
    }
    new_elem->refcnt = 1;
    return new_elem;
}

static inline struct u6a_vm_pool_elem*
u6a_vm_pool_elem_dup_(struct u6a_vm_pool_ctx* ctx, struct u6a_vm_pool_elem* elem) {
    struct u6a_vm_pool_elem* new_elem = u6a_vm_pool_elem_alloc_(ctx);
    *new_elem = *elem;
    return new_elem;
}

bool
u6a_vm_pool_init(struct u6a_vm_pool_ctx* ctx, uint32_t pool_len, uint32_t ins_len, jmp_buf* jmp_ctx, const char* err_stage);

static inline uint32_t
u6a_vm_pool_alloc1(struct u6a_vm_pool_ctx* ctx, struct u6a_vm_var_fn v1) {
    struct u6a_vm_pool_elem* elem = u6a_vm_pool_elem_alloc_(ctx);
    elem->values = (struct u6a_vm_var_tuple) { .v1.fn = v1, .v2.ptr = NULL };
    elem->flags = 0;
    return elem - ctx->active_pool->elems;
}

static inline uint32_t
u6a_vm_pool_alloc2(struct u6a_vm_pool_ctx* ctx, struct u6a_vm_var_fn v1, struct u6a_vm_var_fn v2) {
    struct u6a_vm_pool_elem* elem = u6a_vm_pool_elem_alloc_(ctx);
    elem->values = (struct u6a_vm_var_tuple) { .v1.fn = v1, .v2.fn = v2 };
    elem->flags = 0;
    return elem - ctx->active_pool->elems;
}

static inline uint32_t
u6a_vm_pool_alloc2_ptr(struct u6a_vm_pool_ctx* ctx, void* v1, void* v2) {
    struct u6a_vm_pool_elem* elem = u6a_vm_pool_elem_alloc_(ctx);
    elem->values = (struct u6a_vm_var_tuple) { .v1.ptr = v1, .v2.ptr = v2 };
    elem->flags = U6A_VM_POOL_ELEM_HOLDS_PTR;
    return elem - ctx->active_pool->elems;
}

static inline union u6a_vm_var
u6a_vm_pool_get1(struct u6a_vm_pool* pool, uint32_t offset) {
    return pool->elems[offset].values.v1;
}

static inline struct u6a_vm_var_tuple
u6a_vm_pool_get2(struct u6a_vm_pool* pool, uint32_t offset) {
    return pool->elems[offset].values;
}

static inline struct u6a_vm_var_tuple
u6a_vm_pool_get2_separate(struct u6a_vm_pool_ctx* ctx, uint32_t offset) {
    struct u6a_vm_pool_elem* elem = ctx->active_pool->elems + offset;
    struct u6a_vm_var_tuple values = elem->values;
    if (elem->refcnt > 1) {
        // Continuation having more than 1 reference should be separated before reinstatement
        values.v1.ptr = u6a_vm_stack_dup(ctx->stack_ctx, values.v1.ptr);
    }
    return values;
}

static inline void
u6a_vm_pool_addref(struct u6a_vm_pool* pool, uint32_t offset) {
    ++pool->elems[offset].refcnt;
}

static inline void
u6a_vm_pool_free(struct u6a_vm_pool_ctx* ctx, uint32_t offset) {
    struct u6a_vm_pool_elem* elem = ctx->active_pool->elems + offset;
    struct u6a_vm_pool_elem_ptrs* holes = ctx->holes;
    ctx->fstack_top = UINT32_MAX;
    do {
        if (--elem->refcnt == 0) {
            holes->elems[++holes->pos] = elem;
            if (elem->flags & U6A_VM_POOL_ELEM_HOLDS_PTR) {
                // Continuation destroyed before used
                u6a_vm_stack_discard(ctx->stack_ctx, elem->values.v1.ptr);
            } else {
                u6a_free_stack_push_(ctx, elem->values.v2.fn);
                u6a_free_stack_push_(ctx, elem->values.v1.fn);
            }
        }
    } while ((elem = u6a_free_stack_pop_(ctx)));
}

void
u6a_vm_pool_destroy(struct u6a_vm_pool_ctx* ctx);

#endif
