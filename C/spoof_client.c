#include <stdio.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <netinet/ip.h>

// Definir las estructuras de los encabezados
struct ipheader {
 unsigned char      iph_ihl:4, iph_ver:4;
 unsigned char      iph_tos;
 unsigned short int iph_len;
 unsigned short int iph_ident;
 unsigned char      iph_flag:3, iph_offset:13;
 unsigned char      iph_ttl;
 unsigned char      iph_protocol;
 unsigned short int iph_chksum;
 unsigned int       iph_sourceip;
 unsigned int       iph_destip;
};

struct tcpheader {
 unsigned short int tcph_srcport;
 unsigned short int tcph_destport;
 unsigned int       tcph_seqnum;
 unsigned int       tcph_acknum;
 unsigned char      tcph_reserved:4, tcph_offset:4;
 // byte split
 unsigned char      tcph_fin:1, tcph_syn:1, tcph_rst:1, tcph_psh:1, tcph_ack:1, tcph_urg:1, tcph_res2:2;
 unsigned short int tcph_win;
 unsigned short int tcph_chksum;
 unsigned short int tcph_urgptr;
};

// Función para calcular el checksum
unsigned short csum(unsigned short *buf, int len) {
    unsigned long sum;
    for(sum=0; len>0; len--)
        sum += *buf++;
    sum = (sum >> 16) + (sum &0xffff);
    sum += (sum >> 16);
    return (unsigned short)(~sum);
}

// Función principal
int main() {
    // Crear un socket
    int sock = socket (PF_INET, SOCK_RAW, IPPROTO_TCP);
    
    // Construir el encabezado IP
    struct ipheader iph;
    iph.iph_ihl = 5;
    iph.iph_ver = 4;
    iph.iph_tos = 16;
    iph.iph_len = sizeof(struct ipheader) + sizeof(struct tcpheader);
    iph.iph_ident = htons(54321);
    iph.iph_ttl = 64;
    iph.iph_protocol = 6;
    iph.iph_chksum = 0;
    
    // Construir el encabezado TCP
    struct tcpheader tcph;
    tcph.tcph_srcport = htons(12345);
    tcph.tcph_destport = htons(80);
    tcph.tcph_seqnum = htonl(1);
    tcph.tcph_acknum = 0;
    tcph.tcph_offset = 5;
    tcph.tcph_syn = 1;
    tcph.tcph_ack = 0;
    tcph.tcph_win = htons(32767);
    tcph.tcph_chksum = 0;
    tcph.tcph_urgptr = 0;
    
    // Calcular el checksum
    iph.iph_chksum = csum((unsigned short *)&iph, sizeof(iph));
    tcph.tcph_chksum = csum((unsigned short *)&tcph, sizeof(tcph));
    
    // Enviar el paquete
    send(sock, &iph, sizeof(iph), 0);
    send(sock, &tcph, sizeof(tcph), 0);
    
    // Cerrar el socket
    close(sock);
    
    return 0;
}

// Para establecer la dirección IP y el puerto de origen a los del primer cliente, necesitarás conocer estos valores. Supongamos que la dirección IP del primer cliente es "192.168.1.1" y el puerto es "12345". Aquí está el pseudocódigo:

// Convertir la dirección IP del cliente y el puerto a formato de red utilizando las funciones inet_addr y htons.
// Asignar estos valores a los campos correspondientes en las estructuras de encabezado IP y TCP.
// Enviar el paquete a través del socket.
// Aquí está el código en C:

// Por favor, ten en cuenta que este código es solo para fines educativos y no debe usarse para actividades maliciosas.
// #include <arpa/inet.h>

// // ...

// // En la función principal
// int main() {
//     // ...

//     // Establecer la dirección IP y el puerto de origen a los del primer cliente
//     char *client_ip = "192.168.1.1";
//     unsigned short client_port = 12345;

//     iph.iph_sourceip = inet_addr(client_ip);
//     tcph.tcph_srcport = htons(client_port);

//     // ...

//     // Enviar el paquete
//     struct sockaddr_in sin;
//     sin.sin_family = AF_INET;
//     sin.sin_port = tcph.tcph_destport;
//     sin.sin_addr.s_addr = iph.iph_destip;

//     sendto(sock, &iph, sizeof(iph), 0, (struct sockaddr *)&sin, sizeof(sin));
//     sendto(sock, &tcph, sizeof(tcph), 0, (struct sockaddr *)&sin, sizeof(sin));

//     // ...

//     return 0;
// }