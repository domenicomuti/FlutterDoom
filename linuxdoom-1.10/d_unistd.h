#ifndef D_UNISTD_H
#define D_UNISTD_H

#if defined(_WIN32)
    #include <io.h>
    #include <fcntl.h>
    #define open _open
    #define close _close
    #define read _read
    #define write _write
    #define lseek _lseek
    #define access _access
    #define O_RDONLY _O_RDONLY
    #define O_BINARY _O_BINARY
    #define R_OK 4

    #include "synchapi.h"
    #define sleep(x) Sleep((x) * 1000)
	#define usleep(x) Sleep((x) / 1000)
#else
    #include <unistd.h>
#endif

#endif