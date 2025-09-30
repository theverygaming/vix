/* SPDX-License-Identifier: GPL-2.0 WITH Linux-syscall-note */
#pragma once

// https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/errno.h
// https://github.com/torvalds/linux/blob/master/include/uapi/asm-generic/errno-base.h
// modified to prefix all macros with LINUX_

// #include <asm-generic/errno-base.h>

#define LINUX_EPERM   1  /* Operation not permitted */
#define LINUX_ENOENT  2  /* No such file or directory */
#define LINUX_ESRCH   3  /* No such process */
#define LINUX_EINTR   4  /* Interrupted system call */
#define LINUX_EIO     5  /* I/O error */
#define LINUX_ENXIO   6  /* No such device or address */
#define LINUX_E2BIG   7  /* Argument list too long */
#define LINUX_ENOEXEC 8  /* Exec format error */
#define LINUX_EBADF   9  /* Bad file number */
#define LINUX_ECHILD  10 /* No child processes */
#define LINUX_EAGAIN  11 /* Try again */
#define LINUX_ENOMEM  12 /* Out of memory */
#define LINUX_EACCES  13 /* Permission denied */
#define LINUX_EFAULT  14 /* Bad address */
#define LINUX_ENOTBLK 15 /* Block device required */
#define LINUX_EBUSY   16 /* Device or resource busy */
#define LINUX_EEXIST  17 /* File exists */
#define LINUX_EXDEV   18 /* Cross-device link */
#define LINUX_ENODEV  19 /* No such device */
#define LINUX_ENOTDIR 20 /* Not a directory */
#define LINUX_EISDIR  21 /* Is a directory */
#define LINUX_EINVAL  22 /* Invalid argument */
#define LINUX_ENFILE  23 /* File table overflow */
#define LINUX_EMFILE  24 /* Too many open files */
#define LINUX_ENOTTY  25 /* Not a typewriter */
#define LINUX_ETXTBSY 26 /* Text file busy */
#define LINUX_EFBIG   27 /* File too large */
#define LINUX_ENOSPC  28 /* No space left on device */
#define LINUX_ESPIPE  29 /* Illegal seek */
#define LINUX_EROFS   30 /* Read-only file system */
#define LINUX_EMLINK  31 /* Too many links */
#define LINUX_EPIPE   32 /* Broken pipe */
#define LINUX_EDOM    33 /* Math argument out of domain of func */
#define LINUX_ERANGE  34 /* Math result not representable */

#define LINUX_EDEADLK      35 /* Resource deadlock would occur */
#define LINUX_ENAMETOOLONG 36 /* File name too long */
#define LINUX_ENOLCK       37 /* No record locks available */

/*
 * This error code is special: arch syscall entry code will return
 * -ENOSYS if users try to call a syscall that doesn't exist.  To keep
 * failures of syscalls that really do exist distinguishable from
 * failures due to attempts to use a nonexistent syscall, syscall
 * implementations should refrain from returning -ENOSYS.
 */
#define LINUX_ENOSYS 38 /* Invalid system call number */

#define LINUX_ENOTEMPTY   39     /* Directory not empty */
#define LINUX_ELOOP       40     /* Too many symbolic links encountered */
#define LINUX_EWOULDBLOCK EAGAIN /* Operation would block */
#define LINUX_ENOMSG      42     /* No message of desired type */
#define LINUX_EIDRM       43     /* Identifier removed */
#define LINUX_ECHRNG      44     /* Channel number out of range */
#define LINUX_EL2NSYNC    45     /* Level 2 not synchronized */
#define LINUX_EL3HLT      46     /* Level 3 halted */
#define LINUX_EL3RST      47     /* Level 3 reset */
#define LINUX_ELNRNG      48     /* Link number out of range */
#define LINUX_EUNATCH     49     /* Protocol driver not attached */
#define LINUX_ENOCSI      50     /* No CSI structure available */
#define LINUX_EL2HLT      51     /* Level 2 halted */
#define LINUX_EBADE       52     /* Invalid exchange */
#define LINUX_EBADR       53     /* Invalid request descriptor */
#define LINUX_EXFULL      54     /* Exchange full */
#define LINUX_ENOANO      55     /* No anode */
#define LINUX_EBADRQC     56     /* Invalid request code */
#define LINUX_EBADSLT     57     /* Invalid slot */

#define LINUX_EDEADLOCK EDEADLK

#define LINUX_EBFONT          59  /* Bad font file format */
#define LINUX_ENOSTR          60  /* Device not a stream */
#define LINUX_ENODATA         61  /* No data available */
#define LINUX_ETIME           62  /* Timer expired */
#define LINUX_ENOSR           63  /* Out of streams resources */
#define LINUX_ENONET          64  /* Machine is not on the network */
#define LINUX_ENOPKG          65  /* Package not installed */
#define LINUX_EREMOTE         66  /* Object is remote */
#define LINUX_ENOLINK         67  /* Link has been severed */
#define LINUX_EADV            68  /* Advertise error */
#define LINUX_ESRMNT          69  /* Srmount error */
#define LINUX_ECOMM           70  /* Communication error on send */
#define LINUX_EPROTO          71  /* Protocol error */
#define LINUX_EMULTIHOP       72  /* Multihop attempted */
#define LINUX_EDOTDOT         73  /* RFS specific error */
#define LINUX_EBADMSG         74  /* Not a data message */
#define LINUX_EOVERFLOW       75  /* Value too large for defined data type */
#define LINUX_ENOTUNIQ        76  /* Name not unique on network */
#define LINUX_EBADFD          77  /* File descriptor in bad state */
#define LINUX_EREMCHG         78  /* Remote address changed */
#define LINUX_ELIBACC         79  /* Can not access a needed shared library */
#define LINUX_ELIBBAD         80  /* Accessing a corrupted shared library */
#define LINUX_ELIBSCN         81  /* .lib section in a.out corrupted */
#define LINUX_ELIBMAX         82  /* Attempting to link in too many shared libraries */
#define LINUX_ELIBEXEC        83  /* Cannot exec a shared library directly */
#define LINUX_EILSEQ          84  /* Illegal byte sequence */
#define LINUX_ERESTART        85  /* Interrupted system call should be restarted */
#define LINUX_ESTRPIPE        86  /* Streams pipe error */
#define LINUX_EUSERS          87  /* Too many users */
#define LINUX_ENOTSOCK        88  /* Socket operation on non-socket */
#define LINUX_EDESTADDRREQ    89  /* Destination address required */
#define LINUX_EMSGSIZE        90  /* Message too long */
#define LINUX_EPROTOTYPE      91  /* Protocol wrong type for socket */
#define LINUX_ENOPROTOOPT     92  /* Protocol not available */
#define LINUX_EPROTONOSUPPORT 93  /* Protocol not supported */
#define LINUX_ESOCKTNOSUPPORT 94  /* Socket type not supported */
#define LINUX_EOPNOTSUPP      95  /* Operation not supported on transport endpoint */
#define LINUX_EPFNOSUPPORT    96  /* Protocol family not supported */
#define LINUX_EAFNOSUPPORT    97  /* Address family not supported by protocol */
#define LINUX_EADDRINUSE      98  /* Address already in use */
#define LINUX_EADDRNOTAVAIL   99  /* Cannot assign requested address */
#define LINUX_ENETDOWN        100 /* Network is down */
#define LINUX_ENETUNREACH     101 /* Network is unreachable */
#define LINUX_ENETRESET       102 /* Network dropped connection because of reset */
#define LINUX_ECONNABORTED    103 /* Software caused connection abort */
#define LINUX_ECONNRESET      104 /* Connection reset by peer */
#define LINUX_ENOBUFS         105 /* No buffer space available */
#define LINUX_EISCONN         106 /* Transport endpoint is already connected */
#define LINUX_ENOTCONN        107 /* Transport endpoint is not connected */
#define LINUX_ESHUTDOWN       108 /* Cannot send after transport endpoint shutdown */
#define LINUX_ETOOMANYREFS    109 /* Too many references: cannot splice */
#define LINUX_ETIMEDOUT       110 /* Connection timed out */
#define LINUX_ECONNREFUSED    111 /* Connection refused */
#define LINUX_EHOSTDOWN       112 /* Host is down */
#define LINUX_EHOSTUNREACH    113 /* No route to host */
#define LINUX_EALREADY        114 /* Operation already in progress */
#define LINUX_EINPROGRESS     115 /* Operation now in progress */
#define LINUX_ESTALE          116 /* Stale file handle */
#define LINUX_EUCLEAN         117 /* Structure needs cleaning */
#define LINUX_ENOTNAM         118 /* Not a XENIX named type file */
#define LINUX_ENAVAIL         119 /* No XENIX semaphores available */
#define LINUX_EISNAM          120 /* Is a named type file */
#define LINUX_EREMOTEIO       121 /* Remote I/O error */
#define LINUX_EDQUOT          122 /* Quota exceeded */

#define LINUX_ENOMEDIUM    123 /* No medium found */
#define LINUX_EMEDIUMTYPE  124 /* Wrong medium type */
#define LINUX_ECANCELED    125 /* Operation Canceled */
#define LINUX_ENOKEY       126 /* Required key not available */
#define LINUX_EKEYEXPIRED  127 /* Key has expired */
#define LINUX_EKEYREVOKED  128 /* Key has been revoked */
#define LINUX_EKEYREJECTED 129 /* Key was rejected by service */

/* for robust mutexes */
#define LINUX_EOWNERDEAD      130 /* Owner died */
#define LINUX_ENOTRECOVERABLE 131 /* State not recoverable */

#define LINUX_ERFKILL 132 /* Operation not possible due to RF-kill */

#define LINUX_EHWPOISON 133 /* Memory page has hardware error */
