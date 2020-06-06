/*
 * vm_defs.h - Unlambda VM definitions
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

#ifndef U6A_VM_DEFS_H_
#define U6A_VM_DEFS_H_

#include "common.h"
#include "defs.h"

#define U6A_VM_OP_APPLY    ( 1 << 4 )
#define U6A_VM_OP_OFFSET   ( 1 << 5 )
#define U6A_VM_OP_EXTENTED ( 1 << 6 )
#define U6A_VM_OP_INTERNAL ( 1 << 7 )

enum u6a_vm_opcode {
    u6a_vo_placeholder_,
    u6a_vo_app = U6A_VM_OP_APPLY,
    u6a_vo_la,
    u6a_vo_sa = U6A_VM_OP_OFFSET,
    u6a_vo_del,
    u6a_vo_lc = U6A_VM_OP_OFFSET | U6A_VM_OP_EXTENTED,
    u6a_vo_xch = U6A_VM_OP_INTERNAL
};

#define U6A_VM_OP_EX_LC ( 1 << 4 )

enum u6a_vm_opcode_ex {
    u6a_vo_ex_placeholder_,
    u6a_vo_ex_print = U6A_VM_OP_EX_LC
};

#define U6A_VM_FN_CHAR     ( 1 << 4 )
#define U6A_VM_FN_REF      ( 1 << 5 )
#define U6A_VM_FN_PROMISE  ( 1 << 6 )
#define U6A_VM_FN_INTERNAL ( 1 << 7 )

enum u6a_vm_fn {
    u6a_vf_placeholder_,
    u6a_vf_k, u6a_vf_s, u6a_vf_i, u6a_vf_v, u6a_vf_c, u6a_vf_d, u6a_vf_e,
    u6a_vf_in,                                        /* @          */
    u6a_vf_pipe,                                      /* |          */
    u6a_vf_out = U6A_VM_FN_CHAR,                      /* .X         */
    u6a_vf_cmp,                                       /* ?X         */
    u6a_vf_k1 = U6A_VM_FN_REF,                        /* `kX        */
    u6a_vf_s1,                                        /* `sX        */
    u6a_vf_s2,                                        /* ``sXY      */
    u6a_vf_c1,                                        /* `cX        */
    u6a_vf_d1_s = U6A_VM_FN_PROMISE | U6A_VM_FN_REF,  /* `d`XZ      */
    u6a_vf_d1_c,                                      /* `dX        */
    u6a_vf_d1_d = U6A_VM_FN_PROMISE,                  /* `dF        */
    u6a_vf_j = U6A_VM_FN_INTERNAL,                    /* (jump)     */
    u6a_vf_f,                                         /* (finalize) */
    u6a_vf_p                                          /* (print)    */
};

struct u6a_vm_ins {
    uint8_t  opcode;
    uint8_t  opcode_ex;
    uint16_t reserved_;
    union {
        uint32_t offset;
        struct {
            struct u6a_token first;
            struct u6a_token second;
        } fn;
    } operand;
};

struct u6a_vm_var_fn {
    struct u6a_token token;
    uint16_t         reserved_;
    uint32_t         ref;
};

#define U6A_VM_VAR_FN_REF(fn_, ref_) (struct u6a_vm_var_fn) { .token.fn = (fn_), .ref = (ref_) }
#define U6A_VM_VAR_FN_EMPTY          (struct u6a_vm_var_fn) { 0 }
#define U6A_VM_VAR_FN_IS_EMPTY(fn_)  ( ((union u6a_vm_var)(fn_)).ptr == NULL )

union u6a_vm_var {
    struct u6a_vm_var_fn fn;
    void*                ptr;
};

struct u6a_vm_var_tuple {
    union u6a_vm_var v1;
    union u6a_vm_var v2;
};

#define U6A_VM_DEFAULT_STACK_SEGMENT_SIZE   256
#define U6A_VM_MIN_STACK_SEGMENT_SIZE       64
#define U6A_VM_MAX_STACK_SEGMENT_SIZE     ( 1024 * 1024 )

#define U6A_VM_DEFAULT_POOL_SIZE          ( 1024 * 1024 )
#define U6A_VM_MIN_POOL_SIZE                16
#define U6A_VM_MAX_POOL_SIZE              ( 16 * 1024 * 1024 )

#endif
