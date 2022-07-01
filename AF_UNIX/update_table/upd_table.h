#ifndef UPDATE_TAB
#define UPDATE_TAB
#include "../routing_table/table.h"

typedef enum{
    ERR = 0,
    CREATE,
    UPDATE,
    DELETE
}OPCODE;

typedef enum{
    L2 = 1,
    L3
}LAYER;



// rout_entry_t *
// check_entry(rout_body_t *msg, rout_entry_t *head);


// int
// create_new_entry(rout_body_t *msg, rout_entry_t *head);

// int
// update_entry(rout_body_t *msg, rout_entry_t *head);

// int
// delete_entry(rout_body_t *msg, rout_entry_t *head);

int
fill_entry(char *str1, char *str2);

int
update_rout_table(rout_entry_t *head, char *buffer, int b_size);

int
update_mac_table(mac_entry_t *head, char *buffer, int b_size);


#endif