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

table_entry_t head;


//int num_entries = 0;

typedef struct _sync_msg{

    OPCODE op_code;
    msg_body_t msg_body;
}sync_msg_t;

table_entry_t *
check_entry(msg_body_t *msg);


int
create_new_entry(msg_body_t *msg);

int
update_entry(msg_body_t *msg);

int
delete_entry(msg_body_t *msg);

int 
send_updated_table(int comm_socket_fd, char *buffer, int b_size);

int
fill_entry(char *str1, char *str2);


#endif