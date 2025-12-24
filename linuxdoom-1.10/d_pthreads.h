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