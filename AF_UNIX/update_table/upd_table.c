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
    //printf("The received buffer is: %s\n", buffer);

    int msg_integrity = 0;
    //int b_size = sizeof *buffer;
    char rest[b_size], *ptr = rest;
    memcpy(rest, buffer, b_size);
    char *opcode = strtok_r(rest, ",", &ptr);
    char *token = strtok_r(ptr, ",", &ptr);
    //data ->op_code = strtol(opcode, NULL, 0);
    int op_code = atoi(opcode);
    //printf("op code is %d\n", op_code);
    char *tk = strtok_r(token, ";", &ptr);
    if(fill_entry(&rout -> destination, tk))
    {
        if(op_code == DELETE)
        {
            msg_integrity = 1;
            goto CHECK_MSG;
        }

        tk = strtok_r(ptr, ";", &ptr);

        if(fill_entry(&rout ->gateway_ip, tk))
        {
            tk = strtok_r(ptr, ";", &ptr);
            if(fill_entry(&rout ->oif, tk))
                msg_integrity = 1;
        }
    }
    
    CHECK_MSG:
    if(msg_integrity == 0)
    {
        puts("msg body corrupted.");
        return 0;
    }
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
    while(n_entry)
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
    if(t_entry)
        return 0;
    else{

        mac_entry_t *new_entry = malloc(sizeof *new_entry);
        memcpy(&(new_entry -> entry), msg, sizeof *new_entry);
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
    int msg_integrity = 0;
    //int b_size = sizeof *buffer;
    char rest[b_size], *ptr = rest;
    memcpy(rest, buffer, b_size);

    char *opcode = strtok_r(rest, ",", &ptr);
    char *token = strtok_r(ptr, ",", &ptr);
    //data ->op_code = strtol(opcode, NULL, 0);
    int op_code = atoi(opcode);
    
    if(fill_entry(&mac -> mac, token))
        msg_integrity = 1;

    // char *tk = strtok_r(token, ";", &ptr);
    // if(fill_entry(&mac -> mac, tk))
    // {
    //     if(op_code == DELETE)
    //     {
    //         msg_integrity = 1;
    //         goto CHECK_MSG;
    //     }

    //     tk = strtok_r(ptr, ";", &ptr);

    //     if(fill_entry(&rout ->gateway_ip, tk))
    //     {
    //         tk = strtok_r(ptr, ";", &ptr);
    //         if(fill_entry(&rout ->oif, tk))
    //             msg_integrity = 1;
    //     }
    // }
    
    //CHECK_MSG:
    if(msg_integrity == 0)
    {
        puts("msg body corrupted.");
        return 0;
    }
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
        case UPDATE:
            update_mac_entry(mac, head);
            break;
        case DELETE:
            delete_mac_entry(mac, head);
            break;
        default:
            break;
    }

    free(mac);
    return 1;
}
