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

struct vm_stack {
    struct vm_stack*     prev;
    uint32_t             top;
    uint32_t             refcnt;
    struct u6a_vm_var_fn elems[];
};

static struct vm_stack* active_stack;
static        uint32_t  stack_seg_len;

const char* err_stage;

static inline struct vm_stack*
vm_stack_create(struct vm_stack* prev, uint32_t top) {
    const uint32_t size = sizeof(struct vm_stack) + stack_seg_len * sizeof(struct u6a_vm_var_fn);
    struct vm_stack* vs = malloc(size);
    if (UNLIKELY(vs == NULL)) {
        u6a_err_bad_alloc(err_stage, size);
        return NULL;
    }
    vs->prev = prev;
    vs->top = top;
    vs->refcnt = 0;
    return vs;
}

static inline struct vm_stack*
vm_stack_dup(struct vm_stack* vs) {
    const uint32_t size = sizeof(struct vm_stack) + stack_seg_len * sizeof(struct u6a_vm_var_fn);
    struct vm_stack* dup_stack = malloc(size);
    if (UNLIKELY(dup_stack == NULL)) {
        u6a_err_bad_alloc(err_stage, size);
        return NULL;
    }
    memcpy(dup_stack, vs, sizeof(struct vm_stack) + (vs->top + 1) * sizeof(struct u6a_vm_var_fn));
    dup_stack->refcnt = 0;
    for (uint32_t idx = vs->top; idx < UINT32_MAX; --idx) {
        struct u6a_vm_var_fn elem = vs->elems[idx];
        if (elem.token.fn & U6A_VM_FN_REF) {
            u6a_vm_pool_addref(elem.ref);
        }
    }
    if (vs->prev) {
        ++vs->prev->refcnt;
    }
    return dup_stack;
}

static inline void
vm_stack_free(struct vm_stack* vs) {
    struct vm_stack* prev;
    do {
        prev = vs->prev;
        if (--vs->refcnt == 0) {
            for (uint32_t idx = vs->top; idx < UINT32_MAX; --idx) {
                struct u6a_vm_var_fn elem = vs->elems[idx];
                if (elem.token.fn & U6A_VM_FN_REF) {
                    u6a_vm_pool_free(elem.ref);
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
u6a_vm_stack_init(uint32_t stack_seg_len_, const char* err_stage_) {
    stack_seg_len = stack_seg_len_;
    err_stage = err_stage_;
    active_stack = vm_stack_create(NULL, UINT32_MAX);
    return active_stack != NULL;
}

U6A_HOT struct u6a_vm_var_fn
u6a_vm_stack_top() {
    struct vm_stack* vs = active_stack;
    if (UNLIKELY(vs->top == UINT32_MAX)) {
        vs = vs->prev;
        if (UNLIKELY(vs == NULL)) {
            return (struct u6a_vm_var_fn) { 0 };
        }
        active_stack = vs;
    }
    return vs->elems[vs->top];
}

// Boilerplates below. If only we have C++ templates here... (macros just make things nastier)

U6A_HOT bool
u6a_vm_stack_push1(struct u6a_vm_var_fn v0) {
    struct vm_stack* vs = active_stack;
    if (LIKELY(vs->top + 1 < stack_seg_len)) {
        vs->elems[++vs->top] = v0;
        return true;
    }
    active_stack = vm_stack_create(vs, 0);
    if (UNLIKELY(active_stack == NULL)) {
        active_stack = vs;
        return false;
    }
    ++vs->refcnt;
    active_stack->elems[0] = v0;
    return true;
}

U6A_HOT bool
u6a_vm_stack_push2(struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1) {
    struct vm_stack* vs = active_stack;
    if (LIKELY(vs->top + 2 < stack_seg_len)) {
        vs->elems[++vs->top] = v0;
        vs->elems[++vs->top] = v1;
        return true;
    }
    active_stack = vm_stack_create(vs, 1);
    if (UNLIKELY(active_stack == NULL)) {
        active_stack = vs;
        return false;
    }
    ++vs->refcnt;
    active_stack->elems[0] = v0;
    active_stack->elems[1] = v1;
    return true;
}

U6A_HOT bool
u6a_vm_stack_push3(struct u6a_vm_var_fn v0, struct u6a_vm_var_tuple v12) {
    struct vm_stack* vs = active_stack;
    if (LIKELY(vs->top + 3 < stack_seg_len)) {
        vs->elems[++vs->top] = v0;
        vs->elems[++vs->top] = v12.v2.fn;
        vs->elems[++vs->top] = v12.v1.fn;
        return true;
    }
    active_stack = vm_stack_create(vs, 2);
    if (UNLIKELY(active_stack == NULL)) {
        active_stack = vs;
        return false;
    }
    ++vs->refcnt;
    active_stack->elems[0] = v0;
    active_stack->elems[1] = v12.v2.fn;
    active_stack->elems[2] = v12.v1.fn;
    return true;
}

U6A_HOT bool
u6a_vm_stack_push4(struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1, struct u6a_vm_var_tuple v23) {
    struct vm_stack* vs = active_stack;
    if (LIKELY(vs->top + 4 < stack_seg_len)) {
        vs->elems[++vs->top] = v0;
        vs->elems[++vs->top] = v1;
        vs->elems[++vs->top] = v23.v2.fn;
        vs->elems[++vs->top] = v23.v1.fn;
        return true;
    }
    active_stack = vm_stack_create(vs, 3);
    if (UNLIKELY(active_stack == NULL)) {
        active_stack = vs;
        return false;
    }
    ++vs->refcnt;
    active_stack->elems[0] = v0;
    active_stack->elems[1] = v1;
    active_stack->elems[2] = v23.v2.fn;
    active_stack->elems[3] = v23.v1.fn;
    return true;
}

U6A_HOT bool
u6a_vm_stack_pop() {
    struct vm_stack* vs = active_stack;
    if (LIKELY(vs->top-- != UINT32_MAX)) {
        return true;
    }
    active_stack = vs->prev;
    if (UNLIKELY(active_stack == NULL)) {
        u6a_err_stack_underflow(err_stage);
        active_stack = vs;
        return false;
    }
    if (--active_stack->refcnt > 0) {
        active_stack = vm_stack_dup(active_stack);
    }
    if (UNLIKELY(active_stack == NULL)) {
        active_stack = vs;
        return false;
    };
    free(vs);
    --active_stack->top;
    return true;
}

struct u6a_vm_var_fn
u6a_vm_stack_xch(struct u6a_vm_var_fn v0) {
    struct vm_stack* vs = active_stack;
    if (UNLIKELY(vs->top == UINT32_MAX)) {
        active_stack = vs->prev;
        if (UNLIKELY(active_stack == NULL)) {
            active_stack = vs;
            return (struct u6a_vm_var_fn) { 0 };
        }
        if (--active_stack->refcnt > 0) {
            active_stack = vm_stack_dup(active_stack);
        }
        if (UNLIKELY(active_stack == NULL)) {
            active_stack = vs;
            return (struct u6a_vm_var_fn) { 0 };
        };
        free(vs);
        vs = active_stack;
    }
    struct u6a_vm_var_fn elem = vs->elems[vs->top - 1];
    vs->elems[vs->top - 1] = v0;
    return elem;
}

void*
u6a_vm_stack_save() {
    return vm_stack_dup(active_stack);
}

void*
u6a_vm_stack_dup(void* ptr) {
    return vm_stack_dup(ptr);
}

void
u6a_vm_stack_resume(void* ptr) {
    u6a_vm_stack_destroy();
    active_stack = ptr;
}

void
u6a_vm_stack_discard(void* ptr) {
    vm_stack_free(ptr);
}

void
u6a_vm_stack_destroy() {
    vm_stack_free(active_stack);
}
