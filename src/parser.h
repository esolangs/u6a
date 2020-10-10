/*
 * parser.h - Unlambda parser definitions
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

#ifndef U6A_PARSER_H_
#define U6A_PARSER_H_

#include "common.h"
#include "defs.h"

#include <stdbool.h>

bool
u6a_parse(struct u6a_token* token_arr, uint32_t token_len, struct u6a_ast_node** ast_arr);

#endif
