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

#ifndef D_PTHREAD_H
#define D_PTHREAD_H

#if defined(_WIN32)
	#include <threads.h>
	#define pthread_mutex_t mtx_t
	#define pthread_t thrd_t
	#define pthread_create(thr, attr, func, arg) thrd_create(thr, (thrd_start_t)(func), arg)
	#define pthread_mutex_lock mtx_lock
	#define pthread_mutex_unlock mtx_unlock
#else
	#include <pthread.h>
#endif

#endif