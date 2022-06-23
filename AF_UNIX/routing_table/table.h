#ifndef TABLE
#define TABLE


typedef struct _rout_body{

    char destination[20];
    //char mask;
    char gateway_ip[16];
    char oif[10];

}rout_body_t;

typedef struct _mac_body{

    char mac[20];
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