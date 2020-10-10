/*
 * mnemonic.h - Unlambda mnemonics definitions
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

#ifndef U6A_MNEMONIC_H_
#define U6A_MNEMONIC_H_

#include <stdint.h>

const char*
u6a_mnemonic_op(uint8_t op);

const char*
u6a_mnemonic_op_ex(uint8_t op_ex);

const char*
u6a_mnemonic_fn(uint8_t fn);

const char*
u6a_mnemonic_ch(uint8_t ch);

#endif
