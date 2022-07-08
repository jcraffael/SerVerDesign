#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include "upd_table.h"
#include "../routing_table/table.h"


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

rout_entry_t *
check_rout_entry(rout_body_t *msg, rout_entry_t *head){
    
    rout_entry_t *n_entry = head -> next;
    while(n_entry)
    {
        if(strcmp(n_entry ->entry.destination, msg ->destination) == 0)
            return n_entry;
        n_entry = n_entry -> next;
    }
    return NULL;

}


int
create_rout_entry(rout_body_t *msg, rout_entry_t *head){

    rout_entry_t *t_entry = check_rout_entry(msg, head);
    if(t_entry)
        return 0;
    else{

        rout_entry_t *new_entry = malloc(sizeof *new_entry);
        memcpy(&(new_entry -> entry), msg, sizeof(rout_body_t));
        new_entry ->next = NULL;
        rout_entry_t *n_entry = head -> next;
        if(n_entry == NULL)
            head -> next = new_entry;
        else
        {
            while(n_entry -> next){n_entry = n_entry -> next;}
            n_entry -> next = new_entry;

        }
        return 1;
    }
}

int
update_rout_entry(rout_body_t *msg, rout_entry_t *head){

    rout_entry_t *t_entry = check_rout_entry(msg, head);
    if(t_entry)
    {
        memcpy(&(t_entry -> entry), msg, sizeof(rout_body_t));
        return 1;
    }
    return 0;
}

int
delete_rout_entry(rout_body_t *msg, rout_entry_t *head){

    rout_entry_t *t_entry = check_rout_entry(msg, head);
    if(t_entry == NULL)
        return 0;
    else
    {
        rout_entry_t *n_entry = head;
        while(n_entry -> next != t_entry)
        {n_entry = n_entry -> next;}

        n_entry -> next = t_entry -> next;
        free(t_entry);
        return 1;
    }
    
}


int
rout_msg_parse(char *buffer, rout_body_t *rout, int b_size)
{
    char opcode = *buffer;
    //sscanf(buffer, "%c,%s;%s;%s", opcode, &rout -> destination, &rout ->gateway_ip, &rout ->oif);
    *rout = *(rout_body_t *)(buffer + 2);
    printf("New entry is %s, %s and %s\n", rout->destination, rout->gateway_ip, rout->oif);
    int op_code = opcode - '0';
    return op_code;
}

int
update_rout_table(rout_entry_t *head, char *buffer, int b_size)
{

    rout_body_t *rout = malloc(sizeof(rout_body_t));
    memset(rout, 0, sizeof(rout_body_t));

    switch(rout_msg_parse(buffer, rout, b_size)) {
        /* Send result. */
        case ERR:
            break;
        case CREATE:
            create_rout_entry(rout, head);
            break;
        case UPDATE:
            update_rout_entry(rout, head);
            break;
        case DELETE:
            delete_rout_entry(rout, head);
            break;
        default:
            break;
    }

    free(rout);
    return 1;
}

mac_entry_t *
check_mac_entry(mac_body_t *msg, mac_entry_t *head){
    
    mac_entry_t *n_entry = head -> next;
    //puts("Check mac entry.");
    while(n_entry != NULL)
    {
        if(strcmp(n_entry ->entry.mac, msg -> mac) == 0)
            return n_entry;
        n_entry = n_entry -> next;
    }
    return NULL;

}


int
create_mac_entry(mac_body_t *msg, mac_entry_t *head){
    mac_entry_t *t_entry = check_mac_entry(msg, head);
    if(t_entry != NULL)
    {
       puts("Duplicated entry");
       return 0;
    }
    else{

        mac_entry_t *new_entry = malloc(sizeof(mac_entry_t));
        memcpy(&(new_entry -> entry), msg, sizeof(mac_body_t));
        new_entry ->next = NULL;
        mac_entry_t *n_entry = head -> next;
        if(n_entry == NULL)
            head -> next = new_entry;
        else
        {
            while(n_entry -> next){n_entry = n_entry -> next;}
            n_entry -> next = new_entry;

        }
        return 1;
    }
}

int
update_mac_entry(mac_body_t *msg, mac_entry_t *head){

    mac_entry_t *t_entry = check_mac_entry(msg, head);
    if(t_entry)
    {
        memcpy(&(t_entry -> entry), msg, sizeof(mac_body_t));
        return 1;
    }
    return 0;
}

int
delete_mac_entry(mac_body_t *msg, mac_entry_t *head){

    mac_entry_t *t_entry = check_mac_entry(msg, head);
    if(t_entry == NULL)
        return 0;
    else
    {
        mac_entry_t *n_entry = head;
        while(n_entry -> next != t_entry)
        {n_entry = n_entry -> next;}

        n_entry -> next = t_entry -> next;
        free(t_entry);
        return 1;
    }
    
}


int
mac_msg_parse(char *buffer, mac_body_t *mac, int b_size)
{
    
    int op_code = *buffer - '0';
    memcpy(mac, buffer + 2, sizeof(mac_body_t));
    //printf("The opcode is %d, mac is %s\n", op_code, mac);
    return op_code;
}

int
update_mac_table(mac_entry_t *head, char *buffer, int b_size)
{

    mac_body_t *mac = malloc(sizeof(mac_body_t));
    memset(mac, 0, sizeof(mac_body_t));
    
    switch(mac_msg_parse(buffer, mac, b_size)) {
        /* Send result. */
        case ERR:
            break;
        case CREATE:
            create_mac_entry(mac, head);
            break;
        // case UPDATE:
        //     update_mac_entry(mac, head);
        //     break;
        // case DELETE:
        //     delete_mac_entry(mac, head);
        //     break;
        default:
            break;
    }

    free(mac);
    return 1;
}

void
create_pid_entry(pid_entry_t *head, char *buffer)
{
    int pid = atoi(buffer);
    printf("The received pid is %d\n", pid);
    
    pid_entry_t *t_entry = head;
    while(t_entry -> next != NULL)
        t_entry = t_entry -> next;
    pid_entry_t *new_entry = malloc(sizeof(pid_entry_t));
    new_entry -> pid = pid;
    new_entry -> next = NULL;
    t_entry -> next = new_entry;

}

void
flush_pid_table(pid_entry_t *head)
{
    //pid_entry_t *t_entry = head;
    while(head -> next != NULL)
    {
        pid_entry_t *t_entry = head -> next;
        head -> next = t_entry -> next;
        free(t_entry);
    }
}

void
flush_mac_table(mac_entry_t *mac_head)
{
    while(mac_head -> next)
    {
        mac_entry_t* next_node = mac_head -> next;
        mac_head -> next = next_node -> next;
        memset(next_node -> entry.mac, 0, MAC_SIZE);
        free(next_node);
    }
        
            
}

void
flush_rout_table(rout_entry_t *rout_head)
{
    while(rout_head -> next)
    {
        rout_entry_t* next_node = rout_head -> next;
        rout_head -> next = next_node -> next;
        rout_body_clean(next_node -> entry);
        free(next_node);
    }
}