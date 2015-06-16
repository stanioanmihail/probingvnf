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
		bool operator ==(const Session &other) { 
			/* packet is a request */
			if (this->port_src == other.port_src && this->ip_src.s_addr == other.ip_src.s_addr)
				return true;
			/* packet is a reply */
			if (this->port_src == other.port_dst && this->ip_src.s_addr == other.ip_dst.s_addr)
				return true;
			return false;
		}
		u_short get_port_dst() { return this->port_dst;}
		u_short get_port_src() { return this->port_src;}
		struct in_addr get_ip_src() { return this->ip_src;}
		struct in_addr get_ip_dst() { return this->ip_dst;}
		unsigned long get_packets() { return this->packets;}
		void set_ip_src(struct in_addr ip_src) { this->ip_src.s_addr = ip_src.s_addr;}
		void set_ip_dst(struct in_addr ip_dst) { this->ip_dst.s_addr = ip_dst.s_addr;}
		void set_port_src(u_short port_src) { this->port_src = port_src;}
		void set_port_dst(u_short port_dst) { this->port_dst = port_dst;}
		void set_t_prot(transport_protocol prot) { this->t_prot = prot;}
		void set_t_service(type_of_service t) { this->t_service = t;}
		void set_packets(unsigned long p) { this->packets = p;}
		void inc_packets() {this->packets++;}
		void add_packet_no(unsigned long p) {this->packets += p;}
		void reset_packets() {this->packets = 0;}

		~Session() {}
};

class SessionAggregator {
	protected:
		unordered_multimap<u_short, Session> s_map; // map over source port
	public:
		void add_session(Session s) {
			pair<u_short, Session> s_pair(s.get_port_src(), s);
			s_map.insert(s_pair);
		}
		/* Remove the given session from the aggregator
		 * Useful when number of packets get written to database and
		 * multimap must be cleared.
		 */
		Session delete_session(Session s) {
			unordered_multimap<u_short, Session>::iterator it = s_map.find(s.get_port_src());
			for (; it != s_map.end(); it++) {
				if (it->second == s) {
					s_map.erase(it);
				}
			}
		}
		/**
		 * Receive info about session, classify it.
		 * If existent, increase packet no. with existing one.
		 * # test that find
		 */
		void classify_session(Session s) {
			unordered_multimap<u_short, Session>::iterator it = s_map.find(s.get_port_src());
			if (it == s_map.end()) {
				add_session(s);
				return;
			}
			for (; it != s_map.end(); it++) {
				if (it->second == s) {
					it->second.add_packet_no(s.get_packets());
				}
			}
		}
		/**
		 * Reset multimap when info written to the database
		 * Erase all elements;
		 * */
		void reset_multimap() {
			s_map.clear();
		}
};

/**
 * Processes a packet by building a session out of it.
 * All happens sequentially.
 * # ugly part, mixing C with C++
 * */
class PacketProcess {
	private:
		Session s;
	public:
		queue<struct pcap_pkthdr> packet_buffer; // Not sure if to use it yet
		PacketProcess() {}
		~PacketProcess() {}
		Session get_session() {return this->s;}
		void set_session(Session s) {this->s = s;}
		int is_syn_activated(struct my_tcp *tcp) {
			return (tcp->th_flags & 0x02);
		}

		int is_fin_activated(struct my_tcp *tcp) {
			return (tcp->th_flags  & 0x01);
		}

		int is_session_start(struct my_tcp *tcp) {
			return (tcp->th_flags & 0x10); /* SYN + ACK */
		}
		int process_packet (u_char *args,const struct pcap_pkthdr* pkthdr,
				     const u_char* packet)
		{
			u_int16_t type = handle_ethernet(args,pkthdr,packet);

			if(type == ETHERTYPE_IP) {/* handle IP packet */
				if (!handle_IP(args,pkthdr,packet)) {
					s.inc_packets();
					return 0;  /* success */
				}
			}
			/* DISCARD */
			return -1;
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
		int handle_tcp (u_char *args, const struct pcap_pkthdr* pkthdr,
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
		    if (length < sizeof(struct my_tcp)) {
			printf("truncated tcp/udp/other %d",length);
			return -1;
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
		    return 0;
		}
		//------------------------------------------------------------------
		int handle_IP (u_char *args,const struct pcap_pkthdr* pkthdr,
					const u_char* packet)
		{
		    const struct my_ip* ip;
		    u_int length = pkthdr->len;
		    u_int hlen,off,version;
		    int i, len;

		    /* jump pass the ethernet header */
		    ip = (struct my_ip*)(packet + sizeof(struct ether_header));
		    length -= sizeof(struct ether_header); 

		    /* check to see we have a packet of valid length */
		    if (length < sizeof(struct my_ip)) {
			printf("truncated ip %d",length);
			return -1;
		    }
		    len     = ntohs(ip->ip_len);
		    hlen    = IP_HL(ip); /* header length */
		    version = IP_V(ip);/* ip version */

		    /* check version */
		    if(version != 4) {
		      fprintf(stdout,"Unknown version %d\n",version);
		      return -1;
		    }
		    /* check header length */
		    if(hlen < 5 ) {
			fprintf(stdout,"bad-hlen %d \n",hlen);
		    }
		    /* see if we have as much packet as we should */
		    if(length < len) {
			printf("\ntruncated IP - %d bytes missing\n",len - length);
		    }
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
		    return 0; /* success */
		}
};



/* Acts as a manager for the events happening on one interface;
 * Takes packets from the if and send them to his SessionAggregator;
 * Then, it should talk to the DB through a DBConnector object;
 * */

class DevProbing {
	public:
		char *dev;
		char errbuf[PCAP_ERRBUF_SIZE];
		pcap_t* descr;
		static SessionAggregator s_aggr;
		//----------------------------------------------------
		DevProbing() : dev(0), descr(0) {}
		~DevProbing() {}
		void open_dev() {
			/* lookup interface */
			this->dev = pcap_lookupdev(errbuf);
			if(dev == NULL){
				printf("%s\n",errbuf); exit(1);
				return;
			}
			/* open device for reading */
			descr = pcap_open_live(dev,BUFSIZ,0,-1,errbuf);
			if(descr == NULL){
				printf("pcap_open_live(): %s\n",errbuf);
				return;
			}
		}
		/*
		 * Callback function to be used for the packet processing on event
		 * Is it possible that this function will be called twice
		 * in the same time, resulting in a race condition?!
		 * Then, access to multimap should be sync;
		 * */
		static void my_callback(u_char *args,const struct pcap_pkthdr* pkthdr, 
				const u_char* packet)
		{
			// Better use a Visitor.visit();
			PacketProcess process_pack;
			if (!process_pack.process_packet(args, pkthdr, packet)) { /* only on success */
				s_aggr.classify_session(process_pack.get_session());
			}
		}
		/* *
		 * Call pcap_loop(..) and pass in the callback function.
		 * Loop packets on the selected interface.
		 */
		void dispatch_packet() {
			pcap_loop(descr, LOOP_PACKETS, my_callback,NULL);
		}
};

/*
 * Should definitely be done on another thread!
 *
 * */
class DBConnector {
	public:
		// TODO - add sqlite3 connection info
		DBConnector();
		~DBConnector();
};


int main() {return 0;}

