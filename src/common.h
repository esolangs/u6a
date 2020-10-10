/*
 * common.h - common includes and definitions
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

#ifndef U6A_COMMON_H_
#define U6A_COMMON_H_

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#ifdef __GNUC__
#define LIKELY(expr)      __builtin_expect(!!(expr), 1)
#define UNLIKELY(expr)    __builtin_expect(!!(expr), 0)
#define U6A_COLD          __attribute__((cold))
#define U6A_HOT           __attribute__((hot))
#define U6A_NOT_REACHED() __builtin_unreachable()
#else
#define LIKELY(expr)      (expr)
#define UNLIKELY(expr)    (expr)
#define U6A_COLD
#define U6A_HOT
#define U6A_NOT_REACHED()
#endif

#ifdef __linux__
#include <linux/limits.h>
#else
#include <limits.h>
#endif

#define U6A_MAGIC     0xDC  /* Latin 'U' with diaeresis */
#define U6A_VER_MAJOR 0x00
#define U6A_VER_MINOR 0x01
#define U6A_VER_PATCH 0x01

#endif
