#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include "../routing_table/table.h"

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128

int
main(int argc, char *argv[])
{
    table_entry_t head;
    memset(head.entry.destination, 0, sizeof(head.entry.destination));
    memset(head.entry.gateway_ip, 0, sizeof(head.entry.gateway_ip));
    memset(head.entry.oif, 0, sizeof(head.entry.oif));
    head.next = NULL;
    struct sockaddr_un addr;
    char *msg = malloc(64);
    int ret;
    int data_socket;
    char buffer[BUFFER_SIZE];
    

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

        if(update_routing_table(&head, buffer, sizeof(buffer)) == 0)
            continue;

        puts("Updated table is:");
        table_entry_t* next_node = head.next;
        //printf("%s  %s  %s\n",first_entry -> entry.destination, first_entry -> entry.gateway_ip, first_entry -> entry.oif);
        //struct _table_entry *next_node = head -> next;
        while(next_node)
        {
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
