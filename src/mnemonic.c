/*
 * mnemonic.c - Unlambda mnemonics
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

#include "mnemonic.h"
#include "vm_defs.h"

const char*
u6a_mnemonic_op(uint8_t op) {
    switch (op) {
        case u6a_vo_app:
            return "APP";
        case u6a_vo_la:
            return "LA";
        case u6a_vo_sa:
            return "SA";
        case u6a_vo_del:
            return "DEL";
        case u6a_vo_lc:
            return "LC";
        case u6a_vo_xch:
            return "XCH";
        default:
            U6A_NOT_REACHED();
    }
}

const char*
u6a_mnemonic_op_ex(uint8_t op_ex) {
    switch (op_ex) {
        case u6a_vo_ex_print:
            return "print";
        default:
            U6A_NOT_REACHED();
    }
}

const char*
u6a_mnemonic_fn(uint8_t fn) {
    switch (fn) {
        case u6a_vf_placeholder_:
            return "acc";
        case u6a_vf_k:
            return "k";
        case u6a_vf_s:
            return "s";
        case u6a_vf_i:
            return "i";
        case u6a_vf_v:
            return "v";
        case u6a_vf_c:
            return "c";
        case u6a_vf_d:
            return "d";
        case u6a_vf_e:
            return "e";
        case u6a_vf_in:
            return "@";
        case u6a_vf_pipe:
            return "|";
        case u6a_vf_out:
            return ".";
        case u6a_vf_cmp:
            return "?";
        case u6a_vf_k1:
            return "`k";
        case u6a_vf_s1:
            return "`s";
        case u6a_vf_s2:
            return "``s";
        case u6a_vf_c1:
            return "`c";
        case u6a_vf_d1_s:
        case u6a_vf_d1_c:
        case u6a_vf_d1_d:
            return "`d";
        case u6a_vf_j:
            return "~j";
        case u6a_vf_f:
            return "~f";
        case u6a_vf_p:
            return "~p";
        default:
            U6A_NOT_REACHED();
    }
}

const char*
u6a_mnemonic_ch(uint8_t ch) {
    static const char* ascii_table =
        "!\0\"\0#\0$\0%\0&\0'\0(\0)\0*\0+\0,\0-\0.\0/\0000\0001\0002\0003\0004\0005\0006\0007\0008\0009\0"
        ":\0;\0<\0=\0>\0?\0@\0A\0B\0C\0D\0E\0F\0G\0H\0I\0J\0K\0L\0M\0N\0O\0P\0Q\0R\0S\0T\0U\0V\0W\0X\0Y\0Z\0"
        "[\0\\\0]\0^\0_\0`\0a\0b\0c\0d\0e\0f\0g\0h\0i\0j\0k\0l\0m\0n\0o\0p\0q\0r\0s\0t\0u\0v\0w\0x\0y\0z\0{\0|\0}\0~";
    if (ch == ' ') {
        return "<SP>";
    } else if (ch == '\n') {
        return "<LF>";
    } else if (ch > 32 && ch < 127) {
        return ascii_table + ((ch - 33) << 1);
    } else {
        U6A_NOT_REACHED();
    }
}
