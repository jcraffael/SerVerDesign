
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "table.h"



// rout_entry_t *
// check_entry(rout_body_t *msg, rout_entry_t *head){
    
//     rout_entry_t *n_entry = head -> next;
//     while(n_entry)
//     {
//         if(strcmp(n_entry ->entry.destination, msg ->destination) == 0)
//             return n_entry;
//         n_entry = n_entry -> next;
//     }
//     return NULL;

// }


// int
// create_new_entry(rout_body_t *msg, rout_entry_t *head){

//     rout_entry_t *t_entry = check_entry(msg, head);
//     if(t_entry)
//         return 0;
//     else{

//         rout_entry_t *new_entry = malloc(sizeof *new_entry);
//         memcpy(&(new_entry -> entry), msg, sizeof(rout_body_t));
//         new_entry ->next = NULL;
//         rout_entry_t *n_entry = head -> next;
//         if(n_entry == NULL)
//             head -> next = new_entry;
//         else
//         {
//             while(n_entry -> next){n_entry = n_entry -> next;}
//             n_entry -> next = new_entry;

//         }
//         return 1;
//     }
// }

// int
// update_entry(rout_body_t *msg, rout_entry_t *head){

//     rout_entry_t *t_entry = check_entry(msg, head);
//     if(t_entry)
//     {
//         memcpy(&(t_entry -> entry), msg, sizeof(rout_body_t));
//         return 1;
//     }
//     return 0;
// }

// int
// delete_entry(rout_body_t *msg, rout_entry_t *head){

//     rout_entry_t *t_entry = check_entry(msg, head);
//     if(t_entry == NULL)
//         return 0;
//     else
//     {
//         rout_entry_t *n_entry = head;
//         while(n_entry -> next != t_entry)
//         {n_entry = n_entry -> next;}

//         n_entry -> next = t_entry -> next;
//         free(t_entry);
//         return 1;
//     }
    
// }

// int 
// send_updated_table(int comm_socket_fd, char *buffer, int b_size){

//     int ret = write(comm_socket_fd, buffer, b_size);
//     if (ret == -1) {
//         perror("write");
//         exit(EXIT_FAILURE);
//     }
//     return 1;
// }

// int
// fill_entry(char *str1, char *str2)
// {
//     if(str2 != NULL)
//     {
//         strcpy(str1, str2);
//         return 1;
//     }
//     return 0;
// }

// int
// update_routing_table(rout_entry_t *head, char *buffer, int b_size)
// {
//     sync_msg_t *data = malloc(sizeof(sync_msg_t));
//     memset(data, 0, sizeof(sync_msg_t));
//     int msg_integrity = 0;
//     char rest[b_size], *ptr = rest;
//     memcpy(rest, buffer, b_size);
//     char *opcode = strtok_r(rest, ",", &ptr);
//     char *token = strtok_r(ptr, ",", &ptr);
//     data ->op_code = strtol(opcode, NULL, 0);
    
//     char *tk = strtok_r(token, ";", &ptr);
//     if(fill_entry(&data->rout_body.destination, tk))
//     {
//         if(data ->op_code == 3)
//         {
//             msg_integrity = 1;
//             goto CHECK_MSG;
//         }

//         tk = strtok_r(ptr, ";", &ptr);

//         if(fill_entry(&data->rout_body.gateway_ip, tk))
//         {
//             tk = strtok_r(ptr, ";", &ptr);
//             if(fill_entry(&data->rout_body.oif, tk))
//                 msg_integrity = 1;
//         }
//     }
    
//     CHECK_MSG:
//     if(msg_integrity == 0)
//     {
//         puts("msg body corrupted.");
//         return 0;
//     }
//     switch(data -> op_code) {
//         /* Send result. */
//         case CREATE:
//             create_new_entry(&data ->rout_body, head);
//             break;
//         case UPDATE:
//             update_entry(&data ->rout_body, head);
//             break;
//         case DELETE:
//             delete_entry(&data->rout_body, head);
//             break;
//     }
//     free(data);
//     return 1;
// }