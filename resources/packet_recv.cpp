#include <unordered_map>
#include <queue>

#include "packet.h"

#define LOOP_PACKETS		100

using namespace std;

enum transport_protocol {TCP, UDP, OTHER};
enum type_of_service {HTTP, TORRENT, VIDEO, DEFAULT};

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
		void set_ip_src(struct in_addr ip_src) {}
		void set_ip_dst(struct in_addr ip_dst) {}
		void set_port_src(u_short port_src) {}
		void set_port_dst(u_short port_dst) {}
		void set_t_prot(transport_protocol prot) {}
		void set_t_service(type_of_service t) {}
		void set_packets(unsigned long p) {}
		void inc_packets() {this->packets++;}
		void reset_packets() {this->packets = 0;}

		~Session() {}
};

class PacketProcess {
	public:
		queue<struct pcap_pkthdr> packet_buffer; // Not sure if to use it yet
		Session s;
		PacketProcess() {}
		~PacketProcess() {}
		int is_syn_activated(struct my_tcp *tcp) {
			return (tcp->th_flags & 0x02);
		}

		int is_fin_activated(struct my_tcp *tcp) {
			return (tcp->th_flags  & 0x01);
		}

		int is_session_start(struct my_tcp *tcp) {
			return (tcp->th_flags & 0x10); /* SYN + ACK */
		}
		void process_packet (u_char *args,const struct pcap_pkthdr* pkthdr,
				     const u_char* packet)
		{
			u_int16_t type = handle_ethernet(args,pkthdr,packet);
			if(type == ETHERTYPE_IP)
			{/* handle IP packet */
				handle_IP(args,pkthdr,packet);
			}
			/* DISCARD */
		}
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
		    s.set_port_src(ntohs(tcp->th_sport));
		    s.set_port_dst(ntohs(tcp->th_dport));

		    if (ntohs(tcp->th_dport) == 443 || ntohs(tcp->th_dport) == 80) {

			s.set_t_service(HTTP); 
			    //printf("PORT SRC: %d\n and PORT DTS: %d\n",  ntohs(tcp->th_sport),  ntohs(tcp->th_dport));
			    // TODO get_domain_name(args, pkthdr, packet);
		    } else if (ntohs(tcp->th_dport) == 9091 || ntohs(tcp->th_dport) == 30301 ||
				(ntohs(tcp->th_dport) > 6881 && ntohs(tcp->th_dport) < 6887) )
		    {
			s.set_t_service(TORRENT);
		    } else {
			    s.set_t_service(DEFAULT);
		    } // TODO add if (VIDEO)

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

		    off = ntohs(ip->ip_off);

		    s.set_ip_src(ip->ip_src);
		    s.set_ip_dst(ip->ip_dst);

		    if (ip->ip_p == 0x06) {
			s.set_t_prot(TCP);
		    } else if (ip->ip_p == 0x11) {
			s.set_t_prot(UDP);
		    } else {
			s.set_t_prot(OTHER);
		    }

		     /* Check to see if we have the first fragment */
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
};

class DevProbing {
	public:
		char *dev;
		char errbuf[PCAP_ERRBUF_SIZE];
		pcap_t* descr;
		DevProbing(){
			this->dev = NULL;
			this->descr = NULL;
		}
		~DevProbing() {}
		// TODO - Add support for more probing interfaces: wlan, eth0
		//-----------------------------------------------------------
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
		static void my_callback(u_char *args,const struct pcap_pkthdr* pkthdr,
				 const u_char* packet)
		{
			PacketProcess process_pack;
			process_pack.process_packet(args, pkthdr, packet);
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

int main() {return 0;}

