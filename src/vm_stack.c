/*
 * vm_stack.c - Unlambda VM segmented stacks
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

#include "vm_stack.h"
#include "vm_pool.h"
#include "logging.h"

#include <stddef.h>
#include <stdlib.h>
#include <string.h>

static inline struct u6a_vm_stack*
vm_stack_create(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_stack* prev, uint32_t top) {
    const uint32_t size = sizeof(struct u6a_vm_stack) + ctx->stack_seg_len * sizeof(struct u6a_vm_var_fn);
    struct u6a_vm_stack* vs = malloc(size);
    if (UNLIKELY(vs == NULL)) {
        u6a_err_bad_alloc(ctx->err_stage, size);
        return NULL;
    }
    vs->prev = prev;
    vs->top = top;
    vs->refcnt = 0;
    return vs;
}

static inline struct u6a_vm_stack*
vm_stack_dup(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_stack* vs) {
    const uint32_t size = sizeof(struct u6a_vm_stack) + ctx->stack_seg_len * sizeof(struct u6a_vm_var_fn);
    struct u6a_vm_stack* dup_stack = malloc(size);
    if (UNLIKELY(dup_stack == NULL)) {
        u6a_err_bad_alloc(ctx->err_stage, size);
        return NULL;
    }
    memcpy(dup_stack, vs, sizeof(struct u6a_vm_stack) + (vs->top + 1) * sizeof(struct u6a_vm_var_fn));
    dup_stack->refcnt = 0;
    for (uint32_t idx = vs->top; idx < UINT32_MAX; --idx) {
        struct u6a_vm_var_fn elem = vs->elems[idx];
        if (elem.token.fn & U6A_VM_FN_REF) {
            u6a_vm_pool_addref(ctx->pool_ctx->active_pool, elem.ref);
        }
    }
    if (vs->prev) {
        ++vs->prev->refcnt;
    }
    return dup_stack;
}

static inline void
vm_stack_free(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_stack* vs) {
    struct u6a_vm_stack* prev;
    vs->refcnt = 1;
    do {
        prev = vs->prev;
        if (--vs->refcnt == 0) {
            for (uint32_t idx = vs->top; idx < UINT32_MAX; --idx) {
                struct u6a_vm_var_fn elem = vs->elems[idx];
                if (elem.token.fn & U6A_VM_FN_REF) {
                    u6a_vm_pool_free(ctx->pool_ctx, elem.ref);
                }
            }
            free(vs);
            vs = prev;
        } else {
            break;
        }
    } while (vs);
}

bool
u6a_vm_stack_init(struct u6a_vm_stack_ctx* ctx, uint32_t stack_seg_len, const char* err_stage) {
    ctx->stack_seg_len = stack_seg_len;
    ctx->err_stage = err_stage;
    ctx->active_stack = vm_stack_create(ctx, NULL, UINT32_MAX);
    return ctx->active_stack != NULL;
}

// Boilerplates below. If only we have C++ templates here... (macros just make things nastier)

U6A_HOT bool
u6a_vm_stack_push1_split_(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0) {
    struct u6a_vm_stack* vs = ctx->active_stack;
    ctx->active_stack = vm_stack_create(ctx, vs, 0);
    if (UNLIKELY(ctx->active_stack == NULL)) {
        ctx->active_stack = vs;
        return false;
    }
    ++vs->refcnt;
    ctx->active_stack->elems[0] = v0;
    return true;
}

U6A_HOT bool
u6a_vm_stack_push2_split_(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1) {
    struct u6a_vm_stack* vs = ctx->active_stack;
    ctx->active_stack = vm_stack_create(ctx, vs, 1);
    if (UNLIKELY(ctx->active_stack == NULL)) {
        ctx->active_stack = vs;
        return false;
    }
    ++vs->refcnt;
    ctx->active_stack->elems[0] = v0;
    ctx->active_stack->elems[1] = v1;
    return true;
}

U6A_HOT bool
u6a_vm_stack_push3_split_(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1,
                          struct u6a_vm_var_fn v2)
{
    struct u6a_vm_stack* vs = ctx->active_stack;
    ctx->active_stack = vm_stack_create(ctx, vs, 2);
    if (UNLIKELY(ctx->active_stack == NULL)) {
        ctx->active_stack = vs;
        return false;
    }
    ++vs->refcnt;
    ctx->active_stack->elems[0] = v0;
    ctx->active_stack->elems[1] = v1;
    ctx->active_stack->elems[2] = v2;
    return true;
}

U6A_HOT bool
u6a_vm_stack_push4_split_(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1,
                          struct u6a_vm_var_fn v2, struct u6a_vm_var_fn v3)
{
    struct u6a_vm_stack* vs = ctx->active_stack;
    ctx->active_stack = vm_stack_create(ctx, vs, 3);
    if (UNLIKELY(ctx->active_stack == NULL)) {
        ctx->active_stack = vs;
        return false;
    }
    ++vs->refcnt;
    ctx->active_stack->elems[0] = v0;
    ctx->active_stack->elems[1] = v1;
    ctx->active_stack->elems[2] = v2;
    ctx->active_stack->elems[3] = v3;
    return true;
}

U6A_HOT bool
u6a_vm_stack_pop_split_(struct u6a_vm_stack_ctx* ctx) {
    struct u6a_vm_stack* vs = ctx->active_stack;
    ctx->active_stack = vs->prev;
    if (UNLIKELY(ctx->active_stack == NULL)) {
        u6a_err_stack_underflow(ctx->err_stage);
        ctx->active_stack = vs;
        return false;
    }
    if (--ctx->active_stack->refcnt > 0) {
        ctx->active_stack = vm_stack_dup(ctx, ctx->active_stack);
    }
    if (UNLIKELY(ctx->active_stack == NULL)) {
        ctx->active_stack = vs;
        return false;
    }
    free(vs);
    --ctx->active_stack->top;
    return true;
}

U6A_HOT struct u6a_vm_var_fn
u6a_vm_stack_xch_split_(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_var_fn v0) {
    struct u6a_vm_stack* vs = ctx->active_stack;
    struct u6a_vm_var_fn elem;
    // XCH on segmented stacks is inefficient, perhaps there's a better solution?
    struct u6a_vm_stack* prev = vs->prev;
    if (UNLIKELY(prev == NULL)) {
        u6a_err_stack_underflow(ctx->err_stage);
        return U6A_VM_VAR_FN_EMPTY;
    }
    if (--prev->refcnt > 0) {
        prev = vm_stack_dup(ctx, prev);
        if (UNLIKELY(prev == NULL)) {
            return U6A_VM_VAR_FN_EMPTY;
        }
    }
    if (vs->top == 0) {
        ++prev->refcnt;
        vs->prev = prev;
        elem = prev->elems[prev->top];
        prev->elems[prev->top] = v0;
    } else {
        free(vs);
        ctx->active_stack = prev;
        elem = prev->elems[prev->top - 1];
        prev->elems[prev->top - 1] = v0;
    }
    return elem;
}

struct u6a_vm_stack*
u6a_vm_stack_dup(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_stack* vs) {
    return vm_stack_dup(ctx, vs);
}

void
u6a_vm_stack_discard(struct u6a_vm_stack_ctx* ctx, struct u6a_vm_stack* vs) {
    vm_stack_free(ctx, vs);
}

void
u6a_vm_stack_destroy(struct u6a_vm_stack_ctx* ctx) {
    vm_stack_free(ctx, ctx->active_stack);
}
