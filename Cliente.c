//
//  Cliente.c
//  socketsMulticas
//
//  Created by Alejandro Hernández de la Iglesia on 13/5/15.
//  Copyright (c) 2015 Alejandro Hernandez. All rights reserved.
//

#include <sys/types.h>      /* basic system data types */
#include <sys/socket.h>     /* basic socket definitions */
#include <sys/time.h>       /* timeval{} for select() */
#include <time.h>           /* timespec{} for pselect() */
#include <netinet/in.h>     /* sockaddr_in{} and other Internet defns */
#include <arpa/inet.h>      /* inet(3) functions */
#include <errno.h>
#include <fcntl.h>          /* for nonblocking */
#include <netdb.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>       /* for S_xxx file mode constants */
#include <sys/uio.h>        /* for iovec{} and readv/writev */
#include <unistd.h>
#include <sys/wait.h>
#include <stdio.h>
#include <sys/sem.h>

#define MAXLINE 4096        /* max text line length */
/* Following shortens all the type casts of pointer arguments */
#define SA      struct sockaddr

#define INTERFAZ "eth0"
#define PUERTO 7090
#define MULTICAST "ff02::0084"

