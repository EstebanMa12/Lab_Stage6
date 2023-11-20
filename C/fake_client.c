#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

// struct tcpheader{
//     unsigned short int tcph_srcport;
//     unsigned short int tcph_destport;
//     unsigned int tcph_seqnum;
//     unsigned int tcph_acknum;
//     unsigned char tcph_reserved:4, tcph_offset:4;
//     unsigned char tcph_fin:1, tcph_syn:1, tcph_rst:1, tcph_psh:1, tcph_ack:1, tcph_urg:1, tcph_res2:2;
//     unsigned short int tcph_win;
//     unsigned short int tcph_chksum;
//     unsigned short int tcph_urgptr;
// };

struct pseudo_header
{
	u_int32_t source_address;
	u_int32_t dest_address;
	u_int8_t placeholder;
	u_int8_t protocol;
	u_int16_t tcp_length;
};

#define DATAGRAM_LEN 4096
#define OPT_SIZE 20

unsigned short checksum( const char *buf, unsigned size){
    unsigned sum = 0, i;
    //Accumulate checksum
    for (i = 0; i < size - 1; i += 2){
        unsigned short word16 = *(unsigned short *) &buf[i];
        sum += word16;
    }
    //Handle odd-sized case
    if (size & 1){
        unsigned short word16 = (unsigned char) buf[i];
        sum += word16;
    }
    //Fold to get the ones-complement result
    while (sum >> 16)
        sum = (sum & 0xFFFF) + (sum >> 16);
    //Invert to get the negative in ones-complement arithmetic
    return (unsigned short) ~sum;
}

void create_data_packet(struct sockaddr_in* src, struct sockaddr_in* dst, int32_t seq, int32_t ack_seq, char* data, int data_len, char** out_packet, int* out_packet_len, int id)
{
	// datagram to represent the packet
	char *datagram = calloc(DATAGRAM_LEN, sizeof(char));

	// required structs for IP and TCP header
	struct iphdr *iph = (struct iphdr*)datagram;
	struct tcphdr *tcph = (struct tcphdr*)(datagram + sizeof(struct iphdr));
	struct pseudo_header psh;

	// set payload
	char* payload = datagram + sizeof(struct iphdr) + sizeof(struct tcphdr) + OPT_SIZE;
	memcpy(payload, data, data_len);

	// IP header configuration
	iph->ihl = 5;
	iph->version = 4;
	iph->tos = 0;
	iph->tot_len = sizeof(struct iphdr) + sizeof(struct tcphdr) + OPT_SIZE + data_len;
	iph->id = htons(id); // id of this packet
	iph->frag_off = 0;
	iph->ttl = 64;
	iph->protocol = IPPROTO_TCP;
	iph->check = 0; // correct calculation follows later
	iph->saddr = src->sin_addr.s_addr;
	iph->daddr = dst->sin_addr.s_addr;

	// TCP header configuration
	tcph->source = src->sin_port;
	tcph->dest = dst->sin_port;
	tcph->seq = htonl(seq);
	tcph->ack_seq = htonl(ack_seq);
	tcph->doff = 10; // tcp header size
	tcph->fin = 0;
	tcph->syn = 0;
	tcph->rst = 0;
	tcph->psh = 1;
	tcph->ack = 1;
	tcph->urg = 0;
	tcph->check = 0; // correct calculation follows later
	tcph->window = htons(5840); // window size
	tcph->urg_ptr = 0;

	// TCP pseudo header for checksum calculation
	psh.source_address = src->sin_addr.s_addr;
	psh.dest_address = dst->sin_addr.s_addr;
	psh.placeholder = 0;
	psh.protocol = IPPROTO_TCP;
	psh.tcp_length = htons(sizeof(struct tcphdr) + OPT_SIZE + data_len);
	int psize = sizeof(struct pseudo_header) + sizeof(struct tcphdr) + OPT_SIZE + data_len;
	// fill pseudo packet
	char* pseudogram = malloc(psize);
	memcpy(pseudogram, (char*)&psh, sizeof(struct pseudo_header));
	memcpy(pseudogram + sizeof(struct pseudo_header), tcph, sizeof(struct tcphdr) + OPT_SIZE + data_len);

	tcph->check = checksum((const char*)pseudogram, psize);
	iph->check = checksum((const char*)datagram, iph->tot_len);

	*out_packet = datagram;
	*out_packet_len = iph->tot_len;
	free(pseudogram);
  free(datagram);
  free(*out_packet);

}

int main (int argc, char** argv){
  //Configuration source
  int source_port = 443;
  int identification = 53826;

  if (argc != 4){
    printf("Invalid parameters. \n");
    printf("Usage: %s <src_ip> <dst_ip> <dst_port>\n", argv[0]);
    return 1;
  }

  srand(time(NULL));
  int sock = socket(AF_INET, SOCK_RAW, IPPROTO_TCP);
  if (sock < 0){
    perror("socket() error");
    return 1;
  }

  //Destination IP address configuration
  struct sockaddr_in dst;
  dst.sin_family = AF_INET;
  dst.sin_port = htons(atoi(argv[3]));
  
  // if (inet_pton(AF_INET, SOCK_RAW, IPPROTO_TCP) !=1)
  if (inet_pton(AF_INET, argv[2], &dst.sin_addr) != 1)
  {
    fprintf(stderr,"Error in converting IP to network order\n");
    perror("inet_pton() error");
    return 1;
  }

  //Source IP address configuration
  struct sockaddr_in src;
  src.sin_family = AF_INET;
  src.sin_port = htons(source_port); // COnfiguration of client
  if (inet_pton(AF_INET, argv[1], &src.sin_addr) != 1){
    perror("inet_pton() error");
    return 1;
  }

  printf("Selected source port number: %d\n", ntohs(src.sin_port));

  int one =1;
  const int *val = &one;
  if (setsockopt(sock, IPPROTO_IP, IP_HDRINCL, val, sizeof(one)) < 0){
    perror("setsockopt() error");
    return 1;
  }

  char* data = "Hello world!";
  int data_len = strlen(data);

  char* packet;
  int packet_len;
  int sent;

  create_data_packet(&src, &dst, 0, 0, data, data_len, &packet, &packet_len,identification);
  if ((sent = sendto(sock, packet, packet_len,0, (struct sockaddr*)&dst, sizeof(struct sockaddr)))==-1){
    perror("sendto() error");
    return 1;
  }
  else{
    printf("Sent %d bytes.\n", sent);
  }



  close(sock);
  return 0;

}
