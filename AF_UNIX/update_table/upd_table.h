#ifndef UPDATE_T
#define UPDATE_T
#include "../routing_table/table.h"

typedef enum{
    ERR = 0,
    CREATE,
    UPDATE,
    DELETE
}OPCODE;

typedef struct _sync_msg{

    OPCODE op_code;
    rout_body_t rout_body;
}sync_msg_t;

rout_entry_t *
check_entry(rout_body_t *msg, rout_entry_t *head);


int
create_new_entry(rout_body_t *msg, rout_entry_t *head);

int
update_entry(rout_body_t *msg, rout_entry_t *head);

int
delete_entry(rout_body_t *msg, rout_entry_t *head);

int
fill_entry(char *str1, char *str2);

int
update_routing_table(rout_entry_t *head, char *buffer, int b_size);


#endif