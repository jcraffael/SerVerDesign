#ifndef TABLE_H
#define TABLE_H


#define BUFFER_SIZE 128
#define CIDR_SIZE 20
#define MAC_SIZE 20
#define IP_SIZE 16
#define OIF_SIZE 10

typedef struct _rout_body{

    char destination[CIDR_SIZE];
    //char mask;
    char gateway_ip[IP_SIZE];
    char oif[OIF_SIZE];

}rout_body_t;

typedef struct _mac_body{

    char mac[MAC_SIZE];
}mac_body_t;

// typedef union{

//     rout_body_t rout_body;
//     mac_body_t mac_body;
// }msg_body_t;

typedef struct _rout_entry{

    rout_body_t entry;
    struct _rout_entry *next;

}rout_entry_t;

typedef struct _mac_entry{

    mac_body_t entry;
    struct _mac_entry *next;

}mac_entry_t;

typedef union {

    rout_entry_t rout_table_entry;
    mac_entry_t mac_table_entry;
}table_entry_t;


#endif