/*
 ** Fichero: Cliente.c
 ** Autores:
 ** Alejandro Hernández de la Iglesia 70900084P
 ** Usuario: i0900084
 */



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


int sock;
struct sockaddr_in6	servaddr;
struct ipv6_mreq ipv6mreq;
void leave(int);

int main(int arguments, char **argv)
{
    char buffer[MAXLINE];
    int port;
    char ifaz[4];
    char multicast[10];
    char tiempo[128];
    time_t t;
    struct tm *tiempoLocal;
    
    
    
    signal(SIGINT,&leave);
    
    if (arguments==1)
    {
        port=PUERTO;
        strcpy(ifaz,INTERFAZ);
        strcpy(multicast,MULTICAST);
        printf("\n\nSocket con argumentos por defecto\n");
        printf("\n\n\nMulticas:%s\nInterfaz: %s\nPuerto:%d\n",multicast,ifaz,port);
    }
    else if(arguments==4)//fe80::a10d:4fd4:22ca:3e1d%eth0%port
    {
        strcpy(multicast,argv[1]);
        strcpy(ifaz, argv[2]);
        port=atoi(argv[3]);
        printf("\n\n Argumentos del socket:\n");
        printf("\n\n\nMulticas:%s\nInterfaz: %s\nPuerto:%d\n",multicast,ifaz,port);
    }else{
        printf("\n\n Argumentos [Opcionales]: multicas, interfaz, puerto\n\n");
        return 0;
    }
    //CREAR UDP SOCKET (SOCK_DGRAM)
    if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        perror ("\n\nERROR EN LA CREACION DEL SOCKET\n");
        exit (-1);
    }
    
    /* Unirse al grupo multicast */
    if(setsockopt(sock,IPPROTO_IPV6,SO_REUSEADDR,&ipv6mreq,sizeof(ipv6mreq))<0)
    {
        perror("Llamada setsockopt para multicast\n");
        exit(1);
        
    }
    
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port   = htons(PUERTO);	/* daytime server */
    servaddr.sin6_addr = in6addr_any; //recibir datagramas
    inet_pton(AF_INET6,multicast,&ipv6mreq.ipv6mr_multiaddr); //para convertir las dir IPv4 e IPv6
    ipv6mreq.ipv6mr_interface=if_nametoindex(ifaz);
    if(bind(sock, (struct sockaddr*)&servaddr, sizeof(servaddr)))
    {
        perror("Binding datagram socket error");
        close(sock);
        exit(1);
    }
    
    //con IP_ADD_MEMBERSHIP peta...
    if(setsockopt(sock,IPPROTO_IPV6,IPV6_JOIN_GROUP,&ipv6mreq,sizeof(ipv6mreq))<0)
    {
        printf("\n\ERROR SETSOCKOPT\n\n");
        exit(1);
        
    }
    for(;;)
    {
        if (read(sock, buffer, sizeof(buffer))<0) {
            perror("Error leer scoket");
            exit(1);
        }else{
            t=time(0);
            tiempoLocal = localtime(&t);
            strftime(tiempo, 128, "%d/%m/%y %H:%M:%S", tiempoLocal);
            printf("\n%s\n",tiempo);
            printf("\nMeNSAJE: %s\n",buffer);
        }
    }
    
    return 0;
}

void leave(int signal)
{
    int t;
    printf("\nExit....\n");
    if(setsockopt(sock, IPPROTO_IPV6, IPV6_LEAVE_GROUP, &ipv6mreq, sizeof(ipv6mreq)) < 0)
    {
        perror("\nError socket\n\n");
        exit(1);
    }
    printf("Closing client...\n");
    close(sock);
    exit(0);
    
}