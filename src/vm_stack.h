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

bool
u6a_vm_stack_init(uint32_t stack_seg_len, const char* err_stage);

struct u6a_vm_var_fn
u6a_vm_stack_top();

bool
u6a_vm_stack_push1(struct u6a_vm_var_fn v0);

bool
u6a_vm_stack_push2(struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1);

// Functions push3 and push4 are made for the s2 function to alleviate overhead caused by hot split

bool
u6a_vm_stack_push3(struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1, struct u6a_vm_var_fn v2);

bool
u6a_vm_stack_push4(struct u6a_vm_var_fn v0, struct u6a_vm_var_fn v1,
                   struct u6a_vm_var_fn v2, struct u6a_vm_var_fn v3);

bool
u6a_vm_stack_pop();

struct u6a_vm_var_fn
u6a_vm_stack_xch(struct u6a_vm_var_fn v1);

void*
u6a_vm_stack_save();

void*
u6a_vm_stack_dup(void* ptr);

void
u6a_vm_stack_resume(void* ptr);

void
u6a_vm_stack_discard(void* ptr);

void
u6a_vm_stack_destroy();

#endif
