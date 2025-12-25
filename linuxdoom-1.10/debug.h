/*
 * Copyright (C) 2025 Domenico Muti
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 31 Milk St # 960789 Boston, MA 02196 USA.
 */

#ifndef DEBUG_H
#define DEBUG_H

#if defined(__ANDROID__)
    #include <android/log.h>

    #ifdef NDEBUG
        #define LOG(...)
    #else
        #define LOG(...) __android_log_print(ANDROID_LOG_DEBUG, "flutter", __VA_ARGS__)
    #endif
#else
    #ifdef NDEBUG
        #define LOG(...)
    #else
        #define LOG(...) printf(__VA_ARGS__)
    #endif
#endif

#endif