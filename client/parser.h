void do_status( int up, int force, int im, int im_itm, int im_tm, int im_fuzz, 
	char *im_to, char *force_to, char *to );

void do_load( int itxtotal, int irxtotal );

void do_interface( char *interface, char *local_ip, char *remote_ip );

void do_state( char *state );

void do_message( char *message );

void do_endqueue();

void do_queue();

void do_packet( char *protocol, char *ip1, int port1, char *ip2, int port2,
	char *ttl );
