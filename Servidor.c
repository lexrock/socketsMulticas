/*
 ** Fichero: Servidor.c
 ** Autores:
 ** Alejandro Hernández de la Iglesia 70900084P
 ** Usuario: i0900084
 */

//
//  Servidor.c
//  socketsMulticas
//
//  Created by Alejandro Hernandez on 10/5/15.
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
#include <syslog.h>

#define MAXLINE 4096        /* max text line length */
/* Following shortens all the type casts of pointer arguments */
#define SA      struct sockaddr

#define INTERFAZ "eth0"
#define PUERTO 7090
#define MULTICAST "ff02::0084"
FILE  *flog;
int semaforo;
struct sembuf entro, salgo;
int sock;
struct sockaddr_in6	servaddr;
struct ipv6_mreq ipv6mreq;
pthread_t thread[10];
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;
typedef struct MENSAJE
{
    int cada;
    int durante;
    char texto[100];
}MENSAJE;
MENSAJE mensaje[20];
void leave(int);
void *createThread(void*);

int main(int arguments, char **argv)
{
    
    int i=0,j=0;
    int port;
    char ifaz[4];
    char multicast[10];
    char separador[2]="#";
    char buffer[100];
    char *ptr;
    FILE *f;
    char texto[30] ="\nFin de transmisión...\n";
    
    semaforo = semget(IPC_PRIVATE, 1, IPC_CREAT | 0600);
    if(semctl(semaforo, 0, SETVAL,1)==-1)
    {
        perror("Error SETVAL semaforo\n");
        exit(1);
    }
    //signal
    entro.sem_num=0;
    entro.sem_op=1;
    entro.sem_flg=0;
    //wait
    salgo.sem_num=0;
    salgo.sem_op=-1;
    salgo.sem_flg=0;
    
    signal(SIGINT,&leave);
    switch (fork()) {
        case -1:
            syslog(LOG_ERR, "Error Daemond\n");
            exit(1);
            break;
        case 0:
            break;
            
        default:
            return 0;
    }
    if (arguments==2)
    {
        port=PUERTO;
        strcpy(ifaz,INTERFAZ);
        strcpy(multicast,MULTICAST);
        printf("\n\nSocket con argumentos por defecto\n");
        printf("\n\n\nMulticas:%s\nInterfaz: %s\nPuerto:%d\n",multicast,ifaz,port);
    }
    else if(arguments==5)//fe80::a10d:4fd4:22ca:3e1d%eth0%port
    {
        strcpy(multicast,argv[2]);
        strcpy(ifaz, argv[3]);
        port=atoi(argv[4]);
        printf("\n\n Argumentos del socket:\n");
        printf("\n\n\nMulticas:%s\nInterfaz: %s\nPuerto:%d\n",multicast,ifaz,port);
    }else
        printf("\n\n Argumentos: 'Fichero.txt' [Opcionales]: multicas, interfaz, puerto\n\n");
    f=fopen(argv[1], "r");
    flog=fopen("socketMulticas.log", "w");
    //CREAR UDP SOCKET (SOCK_DGRAM)
    if ((sock = socket(AF_INET6, SOCK_DGRAM, 0)) < 0) {
        syslog(LOG_ERR, "\n\nERROR EN LA CREACION DEL SOCKET\n");
        exit (-1);
    }
    //INICIALIZACION
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin6_family = AF_INET6;
    servaddr.sin6_port   = htons(port);	/* daytime server */
    inet_pton(AF_INET6,multicast,&servaddr.sin6_addr); //para convertir las dir IPv4 e IPv6
    ipv6mreq.ipv6mr_interface=if_nametoindex(ifaz);
    
    //ESPECIFICAR INTERFAZ DE DIFUCION POR DEFECTO
    
    if(setsockopt(sock,IPPROTO_IPV6,IPV6_MULTICAST_IF,&ipv6mreq,sizeof(ipv6mreq))<0)
    {
        syslog(LOG_ERR, "\n\ERROR SETSOCKOPT\n\n");
        exit(1);
        
    }
    
    while (fgets(buffer, 100, f)) {
        ptr=buffer;
        ptr=strtok(buffer,separador);
        strcpy(mensaje[i].texto, ptr);
        ptr=strtok(NULL,separador);
        mensaje[i].cada=atoi(ptr);
        ptr=strtok(NULL,separador);
        mensaje[i].durante=atoi(ptr);
        
        if(pthread_create(&thread[i], NULL, createThread, &mensaje[i])){
            syslog(LOG_ERR, "ERROR AL CREAR THREAD\n");
            exit(1);
        }
        if (i>=20) {
            syslog(LOG_ERR,"Demasiadas lineas para leer(limite de threads 20)\n\n");
            exit(1);
        }
        i++;
    }

    if (sendto(sock, texto, sizeof(texto), 0, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
        syslog(LOG_ERR, "Error sen socket\n");
    
    for (j=0; j<i; i++) {
        pthread_join(thread[j], NULL);
    }

    fclose(f);
    close(sock);
    return 0;
    
    
}

void leave(int signal)
{
    int t;
    char texto[30] ="\nEl Socket se ha cerrado\n";
    
    if (sendto(sock, texto, sizeof(texto), 0, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
        syslog(LOG_ERR, "Error sen socket\n");
        for (t=0; t<10; t++) {
            pthread_cancel(thread[t]);
        }
    if (semctl(semaforo, 0, IPC_RMID)==-1) {
        syslog(LOG_ERR, "Error liberar semaforos\n");
    }

    close(sock);
    exit(0);
    
}

void *createThread(void *msg)
{
    
    int i;
    time_t tEnvio, tRecp;
    MENSAJE *m = (MENSAJE *)msg;
    
    
    
    pthread_mutex_lock( &mutex1 );
    tEnvio=time(NULL);
    
    do {
        
        
        /* if (semop(semaforo, &entro, 1)==-1) {
         perror("Error wait");
         exit(1);
         }*/
        if (sendto(sock, m->texto, sizeof(m->texto), 0, (struct sockaddr*)&servaddr, sizeof(servaddr))<0)
            syslog(LOG_ERR, "Error al mandar mensaje");

        
        else
            syslog(LOG_INFO, "Mensaje: %s\n",m->texto);

        
        sleep(m->cada);
        tRecp=time(NULL);
        /* if (semop(semaforo, &salgo, 1)==-1) {
         perror("Error signal\n");
         exit(1);
         }*/
        
    } while ((tRecp-tEnvio)<m->durante);
    pthread_mutex_unlock( &mutex1 );
    return NULL;
}