#include <pcap.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/if_ether.h> 
#include <net/ethernet.h>
#include <linux/tcp.h>
#include <netinet/ether.h> 
#include <netinet/ip.h> 
#include <string.h>
#include <vector>
#include <multimap>

#define LOOP_PACKETS		100
using namespace std;

enum transport_protocol {TCP, UDP};
enum type_of_service {TORRENT, VIDEO, DEFAULT};

class Session {
	private:
		struct in_addr ip_src, ip_dst;
		u_short port_src, port_dst;
		transport_protocol t_prot;
		type_of_service t_service;
		unsigned long packets;
		// Add timestamp - date
	public:
		Session() : packets(0) {}
		~Session() {}
};

class Packet_dispatcher {
	public:
		char *dev; 
		char errbuf[PCAP_ERRBUF_SIZE];
		pcap_t* descr;
		Packet_dispatcher(){
			this->dev = NULL;
			this->descr = NULL;
		}
		//---------------------------------------------------------------
		void open_dev() {
			this->dev = pcap_lookupdev(errbuf);
			if(dev == NULL){
				printf("%s\n",errbuf); exit(1);
			}
			/* open device for reading */
			descr = pcap_open_live(dev,BUFSIZ,0,-1,errbuf);
			if(descr == NULL){
				printf("pcap_open_live(): %s\n",errbuf);
				exit(1);
			}
		}
		//----------------------------------------------------------------
		u_int16_t handle_ethernet (u_char *args,const struct pcap_pkthdr* pkthdr,
						const u_char* packet)
		{
		    u_int caplen = pkthdr->caplen;
		    u_int length = pkthdr->len;
		    struct ether_header *eptr;  /* net/ethernet.h */
		    u_short ether_type;

		    if (caplen < ETHER_HDRLEN)
		    {
			fprintf(stdout,"Packet length less than ethernet header length\n");
			return -1;
		    }

		    /* lets start with the ether header... */
		    eptr = (struct ether_header *) packet;
		    return ntohs(eptr->ether_type);
		}
		//------------------------------------------------------------------
		void handle_tcp (u_char *args, const struct pcap_pkthdr* pkthdr,
					const u_char* packet)
		{
		    const struct my_tcp *tcp;
		    u_int length = pkthdr->len;
		    u_int hlen,off,version;
		    int i;

		    int len;

		    /* jump pass the ethernet header */
		    tcp = (struct my_tcp*)(packet + sizeof(struct ether_header) + sizeof(struct iphdr));
		    length -= sizeof(struct ether_header);
		    length -= sizeof(struct iphdr); 

		    printf("IP HDR LEN: %lu\n", sizeof(struct iphdr));
		    /* check to see we have a packet of valid length */
		    if (length < sizeof(struct my_tcp))
		    {
			printf("truncated tcp/udp/other %d",length);
			return ;
		    }
		    if ( ntohs(tcp->th_dport) == 443 ) {
				printf("PORT SRC: %d\n and PORT DTS: %d\n",  ntohs(tcp->th_sport),  ntohs(tcp->th_dport));
				get_domain_name(args, pkthdr, packet);
		    }
		}
		//------------------------------------------------------------------
		u_char* handle_IP (u_char *args,const struct pcap_pkthdr* pkthdr,
					const u_char* packet)
		{
		    const struct my_ip* ip;
		    u_int length = pkthdr->len;
		    u_int hlen,off,version;
		    int i;

		    int len;

		    /* jump pass the ethernet header */
		    ip = (struct my_ip*)(packet + sizeof(struct ether_header));
		    length -= sizeof(struct ether_header); 

		    /* check to see we have a packet of valid length */
		    if (length < sizeof(struct my_ip))
		    {
			printf("truncated ip %d",length);
			return NULL;
		    }

		    len     = ntohs(ip->ip_len);
		    hlen    = IP_HL(ip); /* header length */
		    version = IP_V(ip);/* ip version */

		    /* check version */
		    if(version != 4)
		    {
		      fprintf(stdout,"Unknown version %d\n",version);
		      return NULL;
		    }

		    /* check header length */
		    if(hlen < 5 )
		    {
			fprintf(stdout,"bad-hlen %d \n",hlen);
		    }

		    /* see if we have as much packet as we should */
		    if(length < len)
			printf("\ntruncated IP - %d bytes missing\n",len - length);

		    /* Check to see if we have the first fragment */
		    off = ntohs(ip->ip_off);
		    if((off & 0x1fff) == 0 )/* aka no 1's in first 13 bits */
		    {/* print SOURCE DESTINATION hlen version len offset */
			fprintf(stdout,"IP: ");
			fprintf(stdout,"%s ",
				inet_ntoa(ip->ip_src));
			fprintf(stdout,"%s %d %d %d %d\n",
				inet_ntoa(ip->ip_dst),
				hlen,version,len,off);
		    }
		    handle_tcp(args, pkthdr, packet);
		    return NULL;
		}
		//----------------------------------------------------
		/* looking at ethernet headers */
		void my_callback(u_char *args,const struct pcap_pkthdr* pkthdr,
					const u_char* packet)
		{
		    u_int16_t type = handle_ethernet(args,pkthdr,packet);

		    if(type == ETHERTYPE_IP)
		    {/* handle IP packet */
			handle_IP(args,pkthdr,packet);
		    }else if(type == ETHERTYPE_ARP)
		    {
			/* DISCARD */
		    }
		    else if(type == ETHERTYPE_REVARP)
		    {
			/* DISCARD */
		    }
		}

		//----------------------------------------------------
		void dispatch_packet() {
			/* call pcap_loop(..) and pass in our callback function */
			pcap_loop(descr, LOOP_PACKETS, my_callback,NULL);
		}
};

class DBConnector {
	public:
		DBConnector();
		~DBConnector();
};


class SessionAggregator {
	private:
		unordered_multimap<u_short, Session> s_map; // map over source port
	public:
		void classify_session(Session s) {}
		
};
