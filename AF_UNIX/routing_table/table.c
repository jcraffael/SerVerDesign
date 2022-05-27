
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "table.h"



table_entry_t *
check_entry(msg_body_t *msg){
    
    table_entry_t *n_entry = head.next;
    while(n_entry)
    {
        if(strcmp(n_entry ->entry.destination, msg ->destination) == 0)
            return n_entry;
        n_entry = n_entry -> next;
    }
    return NULL;

}


int
create_new_entry(msg_body_t *msg){

    table_entry_t *t_entry = check_entry(msg);
    if(t_entry)
        return 0;
    else{

        table_entry_t *new_entry = malloc(sizeof *new_entry);
        memcpy(&(new_entry -> entry), msg, sizeof(msg_body_t));
        new_entry ->next = NULL;
        table_entry_t *n_entry = head.next;
        if(n_entry == NULL)
            head.next = new_entry;
        else
        {
            while(n_entry -> next){n_entry = n_entry -> next;}
            n_entry -> next = new_entry;

        }
        return 1;
    }
}

int
update_entry(msg_body_t *msg){

    table_entry_t *t_entry = check_entry(msg);
    if(t_entry)
    {
        memcpy(&(t_entry -> entry), msg, sizeof(msg_body_t));
        return 1;
    }
    return 0;
}

int
delete_entry(msg_body_t *msg){

    table_entry_t *t_entry = check_entry(msg);
    if(t_entry == NULL)
        return 0;
    else
    {
        table_entry_t *n_entry = head.next;
        while(n_entry -> next != t_entry)
        {n_entry = n_entry -> next;}

        n_entry -> next = t_entry -> next;
        free(t_entry);
        return 1;
    }
    
}

int 
send_updated_table(int comm_socket_fd, char *buffer, int b_size){

    // memset(buffer, 0, b_size);
    // memcpy(buffer, head.next, sizeof(table_entry_t));
    // printf("sending final result back to client\n");
    int ret = write(comm_socket_fd, buffer, b_size);
    if (ret == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }
    return 1;
}

int
fill_entry(char *str1, char *str2)
{
    if(str2 != NULL)
    {
        strcpy(str1, str2);
        return 1;
    }
    return 0;
}
