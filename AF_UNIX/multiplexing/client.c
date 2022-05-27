#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include "../routing_table/table.h"

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128

// typedef struct _msg_body{

//     char destination[20];
//     //char mask;
//     char gateway_ip[16];
//     char oif[32];

// }msg_body_t;

// typedef struct _table_entry{

//     msg_body_t entry;
//     struct _table_entry *next;

// }table_entry_t;


int
main(int argc, char *argv[])
{
    struct sockaddr_un addr;
    char *msg = malloc(64);
    int ret;
    int data_socket;
    char buffer[BUFFER_SIZE];
    sync_msg_t *data = malloc(sizeof(sync_msg_t));

    table_entry_t *head = malloc(sizeof(table_entry_t));
    
    //memset(&head, 0, sizeof(head));
    /* Create data socket. */

    data_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (data_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    /*
     * For portability clear the whole structure, since some
     * implementations have additional (nonstandard) fields in
     * the structure.
     * */

    memset(&addr, 0, sizeof(struct sockaddr_un));

    /* Connect socket to socket address */

    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, SOCKET_NAME, sizeof(addr.sun_path) - 1);

    ret = connect (data_socket, (const struct sockaddr *) &addr,
            sizeof(struct sockaddr_un));

    if (ret == -1) {
        fprintf(stderr, "The server is down.\n");
        exit(EXIT_FAILURE);
    }

    /* Send arguments. */
    while(1){
        memset(buffer, 0, BUFFER_SIZE);
        // strncpy (buffer, "RES", strlen("RES"));
        // buffer[strlen(buffer)] = '\0';
        // printf("buffer = %s\n", buffer);
        printf("Enter routing msg to send to server :\n");
        scanf("%s", buffer);
        ret = write(data_socket, buffer, sizeof(buffer));
        if (ret == -1) {
            perror("write");
            break;
        }
        printf("No of bytes sent = %d, buffer size = %ld\n", ret, sizeof(buffer)); 

         /* Receive result. */
        memset(buffer, 0, BUFFER_SIZE);
        
        ret = read(data_socket, buffer, BUFFER_SIZE);
        //ret = read(data_socket, &head, BUFFER_SIZE);
        if (ret == -1) {
            perror("read");
            exit(EXIT_FAILURE);
         }

        memset(data, 0, sizeof(sync_msg_t));
        int msg_integrity = 0;
        char *rest = buffer;
        char *opcode = strtok_r(buffer, ",", &rest);
        char *token = strtok_r(rest, ",", &rest);
        data ->op_code = strtol(opcode, NULL, 0);
        
        char *tk = strtok_r(token, ";", &rest);
        if(fill_entry(&data->msg_body.destination, tk))
        {
            
            tk = strtok_r(rest, ";", &rest);
            if(fill_entry(&data->msg_body.gateway_ip, tk))
            {
                tk = strtok_r(rest, ";", &rest);
                if(fill_entry(&data->msg_body.oif, tk))
                    msg_integrity = 1;
            }
        }
        
        
        if(msg_integrity == 0)
        {
            puts("msg body corrupted.");
            continue;
        }
        switch(data -> op_code) {
            /* Send result. */
            case CREATE:
                create_new_entry(&data ->msg_body);
                break;
            case UPDATE:
                update_entry(&data ->msg_body);
                break;
            case DELETE:
                delete_entry(&data->msg_body);
                break;
        }

        //head = (table_entry_t *)buffer;
        
        puts("Updated table is:");
        printf("%s  %s  %s\n", head -> entry.destination, head -> entry.gateway_ip, head -> entry.oif);
        //struct _table_entry *next_node = head -> next;
        while(head -> next)
        {
            struct _table_entry *next_node = malloc(sizeof(struct _table_entry));
            next_node = head -> next;
            printf("%s  %s  %s\n", next_node -> entry.destination, next_node -> entry.gateway_ip, next_node -> entry.oif);
            next_node = next_node -> next;
        }
        
    } 

    /* Request result. */
    

   

    /* Ensure buffer is 0-terminated. */

    // buffer[BUFFER_SIZE - 1] = 0;

    // printf("Result = %s\n", buffer);

    // /* Close socket. */

    // close(data_socket);

    // exit(EXIT_SUCCESS);
}
