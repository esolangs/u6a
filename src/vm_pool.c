/*
 * vm_pool.c - Unlambda VM object pool
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

#include "vm_pool.h"
#include "logging.h"

#include <stddef.h>
#include <stdlib.h>

bool
u6a_vm_pool_init(struct u6a_vm_pool_ctx* ctx, uint32_t pool_len, uint32_t ins_len, jmp_buf* jmp_ctx, const char* err_stage) {
    const uint32_t pool_size = sizeof(struct u6a_vm_pool) + pool_len * sizeof(struct u6a_vm_pool_elem);
    ctx->active_pool = malloc(pool_size);
    if (UNLIKELY(ctx->active_pool == NULL)) {
        u6a_err_bad_alloc(err_stage, pool_size);
        return false;
    }
    const uint32_t holes_size = sizeof(struct u6a_vm_pool_elem_ptrs) + pool_len * sizeof(struct u6a_vm_pool_elem*);
    ctx->holes = malloc(holes_size);
    if (UNLIKELY(ctx->holes == NULL)) {
        u6a_err_bad_alloc(err_stage, holes_size);
        free(ctx->active_pool);
        return false;
    }
    const uint32_t free_stack_size = ins_len * sizeof(struct vm_pool_elem*);
    ctx->fstack = malloc(free_stack_size);
    if (UNLIKELY(ctx->fstack == NULL)) {
        u6a_err_bad_alloc(err_stage, free_stack_size);
        free(ctx->active_pool);
        free(ctx->holes);
        return false;
    }
    ctx->active_pool->pos = UINT32_MAX;
    ctx->holes->pos = UINT32_MAX;
    ctx->pool_len = pool_len;
    ctx->jmp_ctx = jmp_ctx;
    ctx->err_stage = err_stage;
    return true;
}

void
u6a_vm_pool_destroy(struct u6a_vm_pool_ctx* ctx) {
    free(ctx->active_pool);
    free(ctx->holes);
    free(ctx->fstack);
}
