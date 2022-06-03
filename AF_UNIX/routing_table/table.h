#ifndef TABLE
#define TABLE

typedef enum{
    ERR = 0,
    CREATE,
    UPDATE,
    DELETE
}OPCODE;

typedef struct _msg_body{

    char destination[20];
    //char mask;
    char gateway_ip[16];
    char oif[32];

}msg_body_t;

typedef struct _table_entry{

    msg_body_t entry;
    struct _table_entry *next;

}table_entry_t;


typedef struct _sync_msg{

    OPCODE op_code;
    msg_body_t msg_body;
}sync_msg_t;

table_entry_t *
check_entry(msg_body_t *msg, table_entry_t *head);


int
create_new_entry(msg_body_t *msg, table_entry_t *head);

int
update_entry(msg_body_t *msg, table_entry_t *head);

int
delete_entry(msg_body_t *msg, table_entry_t *head);

int 
send_updated_table(int comm_socket_fd, char *buffer, int b_size);

int
fill_entry(char *str1, char *str2);

int
update_routing_table(table_entry_t *head, char *buffer, int b_size);

#endif