/*
 * dump.h - dump utility definitions
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

#ifndef U6A_DUMP_H_
#define U6A_DUMP_H_

#include "vm_defs.h"

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#define U6A_HEXDUMP_BYTES_PER_LINE 16

bool
u6a_dump_mnemonics(FILE* restrict output_stream, struct u6a_vm_ins* data, uint32_t length);

bool
u6a_dump_data(FILE* restrict output_stream, const char* data, uint32_t length);

#endif
