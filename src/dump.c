/*
 * dump.c - dump utility
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

#include "common.h"
#include "dump.h"
#include "mnemonic.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <arpa/inet.h>

#define D_INC        data[idx + __COUNTER__]
#define D_INC_DUP_4  D_INC, D_INC, D_INC, D_INC
#define D_INC_DUP_16 D_INC_DUP_4, D_INC_DUP_4, D_INC_DUP_4, D_INC_DUP_4
#define F_2B_DUP_2   "%02x%02x %02x%02x "
#define F_2B_DUP_8   F_2B_DUP_2 F_2B_DUP_2 F_2B_DUP_2 F_2B_DUP_2

#define fprintf_check(os, format, ...)           \
    if (fprintf(os, format, __VA_ARGS__) < 0) {  \
        return false;                            \
    }

static inline bool
write_mnemonic_ins(FILE* restrict output_stream, uint32_t offset, struct u6a_vm_ins ins) {
    fprintf_check(output_stream, "%08x:  ", offset);
    const char* op = u6a_mnemonic_op(ins.opcode);
    if (ins.opcode & U6A_VM_OP_EXTENTED) {
        const char* op_ex = u6a_mnemonic_op_ex(ins.opcode_ex);
        int op_len = strlen(op);
        fprintf_check(output_stream, "%-*s<%-5s>%-*s", op_len, op, op_ex, 3 - op_len, "");
    } else {
        fprintf_check(output_stream, "%-10s", op);
    }
    if (ins.opcode & U6A_VM_OP_OFFSET) {
        fprintf_check(output_stream, " 0x%08x\n", ntohl(ins.operand.offset));
    } else if (ins.opcode == u6a_vo_app) {
        const char* fn_1 = u6a_mnemonic_fn(ins.operand.fn.first.fn);
        int fn_1_len = strlen(fn_1);
        if (ins.operand.fn.first.fn & U6A_VM_FN_CHAR) {
            const char* ch_1 = u6a_mnemonic_ch(ins.operand.fn.first.ch);
            int fn_ch_1_len = fn_1_len + strlen(ch_1);
            fprintf_check(output_stream, " %-*s%s,%-*s", fn_1_len, fn_1, ch_1, 5 - fn_ch_1_len, "");
        } else {
            fprintf_check(output_stream, " %-*s,%-*s", fn_1_len, fn_1, 5 - fn_1_len, "");
        }
        const char* fn_2 = u6a_mnemonic_fn(ins.operand.fn.second.fn);
        if (ins.operand.fn.second.fn & U6A_VM_FN_CHAR) {
            const char* ch_2 = u6a_mnemonic_ch(ins.operand.fn.second.ch);
            int fn_2_len = strlen(fn_2);
            fprintf_check(output_stream, " %-*s%-4s\n", fn_2_len, fn_2, ch_2);
        } else {
            fprintf_check(output_stream, " %s\n", fn_2);
        }
    } else {
        fprintf_check(output_stream, " %c", '\n');
    }
    return true;
}

static inline bool
u6a_hexdump(FILE* restrict output_stream, const char* data, const char* formatted, uint32_t length) {
    static const char* format = "%08x:  " F_2B_DUP_8 " %.16s\n";
    for (uint32_t idx = 0; idx < length; idx += U6A_HEXDUMP_BYTES_PER_LINE) {
        uint32_t remaining = length - idx;
        if (UNLIKELY(remaining < U6A_HEXDUMP_BYTES_PER_LINE)) {
            fprintf_check(output_stream, "%08x:  ", idx);
            for (; idx < length - 1; idx += 2) {
                fprintf_check(output_stream, "%02x%02x ", data[idx], data[idx + 1]);
            }
            if (idx == length - 1) {
                fprintf_check(output_stream, "%02x   ", data[idx]);
            }
            int blanks = (U6A_HEXDUMP_BYTES_PER_LINE - remaining) / 2 * 5;
            fprintf_check(output_stream, " %*s%.*s\n", blanks, "", remaining, formatted + length - remaining);
        } else {
            fprintf_check(output_stream, format, idx, D_INC_DUP_16, formatted + idx);
        }
    }
    return true;
}

bool
u6a_dump_mnemonics(FILE* restrict output_stream, struct u6a_vm_ins* data, uint32_t length) {
    fprintf_check(output_stream, "%s\n", ".text");
    for (uint32_t idx = 0; idx < length; ++idx) {
        if (UNLIKELY(!write_mnemonic_ins(output_stream, idx, data[idx]))) {
            return false;
        }
    }
    fprintf_check(output_stream, "%c", '\n');
    return true;
}

bool
u6a_dump_data(FILE* restrict output_stream, const char* data, uint32_t length) {
    fprintf_check(output_stream, "%s\n", ".rodata");
    char* formatted_data = malloc(length);
    for (uint32_t idx = 0; idx < length; ++idx) {
        formatted_data[idx] = isprint(data[idx]) ? data[idx] : '.';
    }
    bool result = u6a_hexdump(output_stream, data, formatted_data, length);
    free(formatted_data);
    return result;
}
