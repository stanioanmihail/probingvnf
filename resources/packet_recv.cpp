#include <unordered_map>
#include <queue>
#include <ctime>
#include <thread>
#include <string>
#include <iostream>
#include <cstdlib>
#include <algorithm>

#include <postgresql/libpq-fe.h>
#include "packet.h"


#define LOOP_PACKETS		500

using namespace std;

enum transport_protocol {TCP, UDP, OTHER};
static const string t_prot_string[] = {"TCP", "UDP", "OTHER"};

enum type_of_service {HTTP, TORRENT, VIDEO, AUDIO, DEFAULT};
static const string t_service_string[] = {"HTTP", "TORRENT", "VIDEO", "AUDIO", "DEFAULT"};

char *format_time(time_t t) {
			struct tm * timeinfo = localtime(&t);
			char *buffer = new char [80];
			strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", timeinfo);
			puts(buffer);
			return buffer;
}

class Session {
	private:
		struct in_addr ip_src, ip_dst;
		u_short port_src, port_dst;
		transport_protocol t_prot;
		type_of_service t_service;
		unsigned long packets;
		unsigned long bytes;
		time_t start_timestamp;
		time_t end_timestamp;
		/* local, these don't get to the database */
		bool fin_activated, syn_activated;
	public:
		Session() : bytes(0), packets(0), start_timestamp(time(0)), end_timestamp(time(0)) {}
		~Session() {}
		bool operator ==(const Session &other) {
			/* packet is a request */
			return is_request(other) || is_reply(other);
		}

		bool is_request(const Session &other) {
			return (this->ip_src.s_addr == other.ip_src.s_addr &&
					this->port_src == other.port_src &&
					this->t_prot == other.t_prot);
		}
		bool is_reply(const Session &other) {
			return (this->ip_src.s_addr == other.ip_dst.s_addr &&
					this->ip_dst.s_addr == other.ip_src.s_addr &&
					this->port_src == other.port_dst &&
					this->port_dst == other.port_src &&
					this->t_prot == other.t_prot);
		}
		char * time_to_char(time_t t) {
			char *tt = ctime(&t);
			//cout << "Local date and time is:" << tt << endl;
			return tt;
		}

		/* type of service to string - "HTTP", "VIDEO", ...*/
		string t_service_to_string() {return t_service_string[t_service];}
		/* transport prot to string - "TCP", ... */
		string t_prot_to_string() {return t_prot_string[t_prot];}
		/* convert IPs to string */
		char *get_string_src_ip() { return inet_ntoa(this->ip_src);}
		char *get_string_dst_ip() { return inet_ntoa(this->ip_dst);}

		/* session to string  */
		string to_string() {
			return  "Source: " + string(get_string_src_ip()) + " "
				+ "Dest: " + string (get_string_dst_ip()) + " "
				+ "Source Port: " + std::to_string(port_src)
				+ "Dest Port: " + std::to_string(port_dst) + " "
				+ t_prot_string[t_prot] + " "
				+ t_service_string[t_service] + " "
				+ "Packets: " + std::to_string(packets)
				+ "Bytes: " + std::to_string(bytes)
				+ "Start time: " + string(time_to_char(start_timestamp))
				+ "End time: " + string(time_to_char(end_timestamp))
				+ "\n";
		}
		void set_end_timestamp() {this->end_timestamp = time(0);}

		/* Setters and getters*/
		u_short get_port_dst() { return this->port_dst;}
		u_short get_port_src() { return this->port_src;}
		struct in_addr get_ip_src() { return this->ip_src;}
		struct in_addr get_ip_dst() { return this->ip_dst;}
		unsigned long get_packets() { return this->packets;}
		unsigned long get_bytes() {return this->bytes;}
		time_t get_start_timestamp() {return this->start_timestamp;}
		time_t get_end_timestamp() {return this->end_timestamp;}
		void set_ip_src(struct in_addr ip_src) { this->ip_src.s_addr = ip_src.s_addr;}
		void set_ip_dst(struct in_addr ip_dst) { this->ip_dst.s_addr = ip_dst.s_addr;}
		void set_port_src(u_short port_src) { this->port_src = port_src;}
		void set_port_dst(u_short port_dst) { this->port_dst = port_dst;}
		void set_t_prot(transport_protocol prot) { this->t_prot = prot;}
		void set_t_service(type_of_service t) { this->t_service = t;}
		void set_packets(unsigned long p) { this->packets = p;}
		void inc_packets() {this->packets++;}
		void add_packet(unsigned long p) {this->packets += p;}
		void reset_packets() {this->packets = 0;}
		void set_bytes(unsigned long b) {this->bytes = b;}
		void add_bytes(unsigned long b) {this->bytes += b;}
};

class SessionAggregator {
	public:
		unsigned long total_bytes;
		unordered_multimap<u_short, Session> s_map; // map over source port

		SessionAggregator() : total_bytes(0) {}
		~SessionAggregator() {}
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
			return s;
		}
		bool discard_session(Session s) {
			string ip = string(s.get_string_src_ip());
			string str(ip, 0, 3);

			cout << ip << endl;
			if (ip == "255.255.255.255")
				return true;
			if (s.get_port_dst() == 53  || s.get_port_src() == 53)
				return true;
			if (str == "224")
				return true;
			return false;
		}
		/**
		 * Receive info about session, classify it.
		 * If existent, increase packet no. with existing one.
		 * # test that find
		 */
		void classify_session(Session s) {
			unordered_multimap<u_short, Session>::iterator it = s_map.find(s.get_port_dst());
			/* search after dst first */
			if (it == s_map.end()) {
				/* search after source */
				it = s_map.find(s.get_port_src());
				if ( it == s_map.end()) {
					add_session(s);
					return;
				}
			}

			for (; it != s_map.end(); it++) {
				if (it->second == s) { /* overloaded operator */
					it->second.add_packet(s.get_packets());
				}
			}
		}
		/*
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
		u_int len;
	public:
		static unsigned long total_packets;
		static unsigned long total_ms;
		queue<struct pcap_pkthdr> packet_buffer; // Not sure if to use it yet
		PacketProcess() {}
		~PacketProcess() {}
		Session& get_session() {return this->s;}
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
		/* function from tcpdump */
		void print_hex_ascii_line(const u_char *payload, int len, int offset, Session& s) {
			int i, gap;
			const u_char *ch;

			/* offset */
			printf("%05d   ", offset);

			/* hex */
			ch = payload;
			for(i = 0; i < len; i++) {
				printf("%02x ", *ch);
				ch++;
				/* print extra space after 8th byte for visual aid */
				if (i == 7)
					printf(" ");
			}
			/* print space to handle line less than 8 bytes */
			if (len < 8)
				printf(" ");

			/* fill hex gap with spaces if not full line */
			if (len < 16) {
				gap = 16 - len;
				for (i = 0; i < gap; i++) {
					printf("   ");
				}
			}
			printf("   ");

			/* ascii (if printable) */
			ch = payload;
			char line[100]; int charno = 0;
			for(i = 0; i < len; i++) {
				if (isprint(*ch)) {
					printf("%c", *ch);
					line[charno] = (char) *ch;
				} else {
					printf(".");
				}
				ch++;
				if (line[charno] % 100 == 0) {
					string find_str(line, 100);
					transform(find_str.begin(), find_str.end(), find_str.begin(), ::tolower);

				// Ugly! Better with contains, sets or something
					if(find_str.find("flv") != string::npos
						|| find_str.find("mov") != string::npos
						|| find_str.find("mpeg4") != string::npos
						|| find_str.find("mp4") != string::npos
						|| find_str.find("mov") != string::npos
						|| find_str.find("avi") != string::npos
						|| find_str.find("mpegps") != string::npos
						|| find_str.find("webm") != string::npos) {
							cout << "FOUND\n";
							s.set_t_service(VIDEO);
					}
				}
			}
			printf("\n");
		}

		/*
		* print packet payload data (avoid printing binary data)
		*/
		void print_payload(const u_char *payload, Session& s) {

			int len_rem = len;
			int line_width = 16;   /* number of bytes per line */
			int line_len;
			int offset = 0;     /* zero-based offset counter */
			const u_char *ch = payload;

			if (len <= 0)
				return;

			/* data fits on one line */
			if (len <= line_width) {
				print_hex_ascii_line(ch, len, offset, s);
				return;
			}

			/* data spans multiple lines */
			for ( ;; ) {
				/* compute current line length */
				line_len = line_width % len_rem;
				/* print line */
				print_hex_ascii_line(ch, line_len, offset, s);
				/* compute total remaining */
				len_rem = len_rem - line_len;
				/* shift pointer to remaining bytes to print */
				ch = ch + line_len;
				/* add offset */
				offset = offset + line_width;
				/* check if we have line width chars or less */
				if (len_rem <= line_width) {
					/* print last line and get out */
					print_hex_ascii_line(ch, len_rem, offset, s);
					break;
				}
			}
		}

		int process_packet (u_char *args,const struct pcap_pkthdr* pkthdr,
				     const u_char* packet)
		{
			u_int16_t type = handle_ethernet(args,pkthdr,packet);
			total_packets++;
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

		    /* jump pass the ethernet header */
		    tcp = (struct my_tcp*)(packet + sizeof(struct ether_header) + sizeof(struct iphdr));
		    length -= sizeof(struct ether_header);
		    length -= sizeof(struct iphdr);

		    char *payload = (char *)(packet + sizeof(struct ether_header)
					+ sizeof(struct iphdr) + sizeof(struct my_tcp));
		    /* check to see we have a packet of valid length */
		    if (length < sizeof(struct my_tcp)) {
			printf("truncated tcp/udp/other %d",length);
			return -1;
		    }
		    //cout << " Src port:" << to_string(ntohs(tcp->th_sport));
		    //cout << " Dst port:" << to_string(ntohs(tcp->th_dport));

		    s.set_port_src(ntohs(tcp->th_sport));
		    s.set_port_dst(ntohs(tcp->th_dport));

		    this->len -= sizeof(struct iphdr);
		    this->len -= sizeof(struct my_tcp);

		    if (ntohs(tcp->th_dport) == 80 | ntohs(tcp->th_sport == 80)) {
			s.set_t_service(HTTP);
		    }
		    if (ntohs(tcp->th_dport) == 443 | ntohs(tcp->th_sport) == 443){
			    s.set_t_service(HTTP);
			    //printf("PORT SRC: %d\n and PORT DTS: %d\n",  ntohs(tcp->th_sport),  ntohs(tcp->th_dport));
			    // TODO get_domain_name(args, pkthdr, packet);
		    } else if (ntohs(tcp->th_dport) == 9091 || ntohs(tcp->th_dport) == 30301 ||
				(ntohs(tcp->th_dport) > 6881 && ntohs(tcp->th_dport) < 6887) )
		    {
			s.set_t_service(TORRENT);
			cout << "FOUND TORRENT\n";
		    } else {
			    s.set_t_service(DEFAULT);
		    }
		    return 0;
		}
		//------------------------------------------------------------------
		int handle_IP (u_char *args,const struct pcap_pkthdr* pkthdr,
					const u_char* packet)
		{
		    const struct my_ip* ip;
		    u_int length = pkthdr->len;
		    u_int hlen,off,version;

		    /* jump pass the ethernet header */
		    ip = (struct my_ip*)(packet + sizeof(struct ether_header));
		    length -= sizeof(struct ether_header);

		    /* check to see we have a packet of valid length */
		    if (length < sizeof(struct my_ip)) {
			printf("truncated ip %d\n",length);
			return -1;
		    }
		    this->len     = ntohs(ip->ip_len);
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
			//fprintf(stdout,"IP: ");
			//fprintf(stdout,"%s ",
			//	inet_ntoa(ip->ip_src));
			//fprintf(stdout,"%s\n",
			//	inet_ntoa(ip->ip_dst));
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
		const char *dev;
		char errbuf[PCAP_ERRBUF_SIZE];
		pcap_t* descr;
		static SessionAggregator s_aggr;
		unsigned int loop_packs;
		//----------------------------------------------------
		DevProbing() : dev(0), descr(0), loop_packs(100) {}
		DevProbing(unsigned int p) : dev(0), descr(0), loop_packs(p) {}
		~DevProbing() {}
		static void print_buckets() {
			unordered_multimap<u_short, Session> s_map = s_aggr.s_map;

			int bucket_no = s_map.bucket_count();
			for (int i = 0; i < bucket_no; i++) {
				cout << "#Bucket " << i << " : ";
				for (auto it = s_map.begin(i); it != s_map.end(i); it++) {
					cout << it->second.to_string();
				}
				cout << endl;
			}
		}
		int open_dev() {
			/* lookup interface */
			//this->dev = pcap_lookupdev(errbuf); - TODO Maybe perform a lookup
			dev = "wlan0";
			if(dev == NULL){
				printf("Error: %s\n",errbuf); exit(1);
				return -1;
			}
			/* open device for reading */
			descr = pcap_open_live(dev,BUFSIZ,0,-1,errbuf);
			if(descr == NULL){
				printf("pcap_open_live(): %s\n",errbuf);
				return -1;
			}
			return 0;
		}
		/*
		 * Callback function to be used for the packet processing on event
		 * Is it possible that this function will be called twice
		 * in the same time, resulting in a race condition?!
		 * Then, access to multimap should be sync;
		 * -- No need, pcap uses buffers, all on a single thread.
		 * */
		static void my_callback(u_char *args,const struct pcap_pkthdr* pkthdr,
				const u_char* packet)
		{
			PacketProcess process_pack;
			if (!process_pack.process_packet(args, pkthdr, packet)) { /* only on success */

				/* print payload for non-discarded and classify*/
				if (!s_aggr.discard_session(process_pack.get_session())) {
					process_pack.print_payload(packet, process_pack.get_session());
					s_aggr.classify_session(process_pack.get_session());
				}
			}
		}
		/* *
		 * Call pcap_loop(..) and pass in the callback function.
		 * Loop packets on the selected interface.
		 */
		int dispatch_packet() {
			pcap_loop(descr, LOOP_PACKETS, my_callback,NULL);
		}
};

SessionAggregator DevProbing::s_aggr;

/*
 * Should definitely be done on another thread!
 *
 * */
class DBConnector {
	public:
		string database_name;
		PGconn          *conn;
		PGresult        *res;
		int             rec_count;
		int             row, col;
		string		IP, user, pass;

		/* Default constructor */
		DBConnector() : database_name("vpersonna"), IP("92.81.85.239"), user("vpersonna"), pass("Abcd123!") {}
		/* Params for ip user password */
		DBConnector(const string ip, const string user, const string pass) {
			this->IP = ip;
			this->user = user;
			this->pass = pass;
		}
		void set_IP(string ip) {
			this->IP = ip;
		}
		string get_IP(string ip) {
			return IP;
		}
		~DBConnector() {}

		void open_database() {
			cout << "Open db with IP " << IP << endl;
			string connect_info = "dbname=vpersonna host=" + IP + " user=" + user + " password=" + pass;
			cout << "COnnect info: " << connect_info << "\n";
			conn = PQconnectdb(connect_info.c_str());
			if (PQstatus(conn) == CONNECTION_BAD) {
				cout << "We were unable to connect to the database\n";
			} else {
				cout << "Connected\n";
			}
			cout << "Exit open_db()\n";
		}
		void close_database() {
			PQclear(res);
			PQfinish(conn);
		}
		string create_insert(Session s, char *table) { /* Does not want to write it */
			/* hardcoded database for now */
			return "INSERT INTO sessions(ip_src, ip_dst, port_src, port_dst, type_of_service) VALUES('"
				+ string(s.get_string_src_ip()) + "', '"
				+ string(s.get_string_dst_ip()) + "', "
				+ to_string(s.get_port_src()) + ", "
				+ to_string(s.get_port_dst())
				+ ", 'TCP');";
		}
		void write_session_to_db(Session s) {
			const char *data = "Insert called\n";

			string insert_str = "INSERT INTO vprofile_rawdata(ip_src, ip_dst, \
				port_src, port_dst, transport_protocol, \
				host_address, traffic_type, \
				timestamp_start, timestamp_end, \
				no_packets, no_bytes) \
				VALUES('" + string(s.get_string_src_ip())
				+ "', '"
				+ string (s.get_string_dst_ip())
				+ "', "
				+ to_string(s.get_port_src())
				+ ", "
				+ to_string(s.get_port_dst())
				+ ", '"
				+ s.t_prot_to_string()
				+ "', '', '"
				+ s.t_service_to_string()
				+ "', '"
				+ string(format_time(s.get_start_timestamp()))
				+ "', '"
				+ string(format_time(s.get_end_timestamp()))
				+ "', "
				+ to_string(s.get_packets())
				+ ", "
				+ to_string(s.get_bytes())
				+ ");";
			const char *insert = insert_str.c_str();
			cout << insert << "\n";
			if (conn == NULL) {
				cout << "DB is null\n";
				return;
			}
			res = PQexec(conn, insert);

			if (PQresultStatus(res) != PGRES_TUPLES_OK) {
				cout << "We did not get any data!\n";
			}
			rec_count = PQntuples(res);
			cout << "We received " << rec_count << " records.\n";
			cout << "==========================\n";

			for (row=0; row<rec_count; row++) {
				for (col=0; col<3; col++) {
					printf("%s\t", PQgetvalue(res, row, col));
				}
				puts("");
			}
		}
};

class DBSessionManager {
	public:
		static DevProbing vprobing;
		static DBConnector db_connector;
		unsigned long seconds, bytes; /* threshold for writing to db */
		bool is_probing;

		DBSessionManager(unsigned long seconds, unsigned long bytes) : is_probing(false) {
			this->seconds = seconds;
			this->bytes = bytes;
		}
		~DBSessionManager() {}
		/*
		 * Start listening for packets on an interface
		 * */
		void start_probing(){
			vprobing.open_dev();
			is_probing = true;
			//while(is_probing) {
				vprobing.dispatch_packet();
				if (--seconds == 0) {
					write_to_database();
					// TODO - change to (seconds == 0 || bytes == 0)
					// with seconds and bytes computed the proper way
					// not with second--
					// TODO 2 - start new thread and call write_to_database() on it - easy
				}
			//}
		}
		void stop_probing() {this->is_probing = false;}
		/*
		 * Insert all sessions into the sqlite3 datbase
		 * */
		void write_to_database() {

			db_connector.open_database();
			SessionAggregator s = vprobing.s_aggr;
			unordered_multimap<u_short, Session> s_map = s.s_map;
			cout << "Start writing to db\n";
			int bucket_no = s_map.bucket_count();
			for (int i = 0; i < bucket_no; i++) {
				for (auto it = s_map.begin(i); it != s_map.end(i); it++) {
					db_connector.write_session_to_db(it->second);
				}
			}
			db_connector.close_database();
		}
};

unsigned long PacketProcess::total_packets = 0;
unsigned long PacketProcess::total_ms = 0;

DevProbing DBSessionManager::vprobing;
DBConnector DBSessionManager::db_connector;


int main(int argc, char *argv[]) {
	cout << "Found time: " << PacketProcess::total_ms << endl;

	if (argc < 2) {
		cout << "Number of arguments min 2\n";
		return 0;
	}
	char *ip = argv[1];
	cout << "ip is: \n" << ip << endl;
	DBSessionManager manager(1000, 30);
	if (manager.vprobing.open_dev()) {
		return -1;
	}
	manager.db_connector.set_IP(string(ip));
	manager.vprobing.dispatch_packet();
	manager.vprobing.print_buckets();

	manager.write_to_database();
	return 0;
}
