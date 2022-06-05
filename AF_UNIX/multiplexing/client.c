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


struct arg_struct {
    char *buffer;
    table_entry_t *head;
    int data_socket;
    int ret;
};


static void *
thread_fn_callback(void *arg) {

    struct arg_struct *args = (struct arg_struct *)arg;
    char *buffer = args -> buffer;
    table_entry_t *head = args -> head;
    int data_socket = args -> data_socket;
    int ret = args -> ret;
	 while(1)
    {
          /* Receive result. */
        memset(buffer, 0, BUFFER_SIZE);
        
        ret = read(data_socket, buffer, BUFFER_SIZE);
        //ret = read(data_socket, &head, BUFFER_SIZE);
        if (ret == -1) {
            perror("read");
            exit(EXIT_FAILURE);
         }

        if(update_routing_table(head, buffer, sizeof(buffer)) == 0)
            continue;

        puts("Updated table is:");
        table_entry_t* next_node = head -> next;
        
        while(next_node)
        {
            printf("%s  %s  %s\n", next_node -> entry.destination, next_node -> entry.gateway_ip, next_node -> entry.oif);
            next_node = next_node -> next;
        }
    }
}


void
routingtb_syn_thread(char *buffer, int ret, table_entry_t *head, int data_socket) {


	pthread_t pthread1;

   
	/* Take some argument to be passed to the thread fn,
 	 * Look after that you always paas the persistent memory
 	 * as an argument to the thread, do not pass caller's 
 	 * local variables Or stack Memory*/	
	struct arg_struct sock_args = {buffer, head, data_socket, ret};


	/* Return 0 on success, otherwise returns errorcode, all
 	 * pthread functions return -ve error code on failure, they
 	 * do not set global 'errno' variable */
	int rc = pthread_create(&pthread1, 
				   NULL, 
				   thread_fn_callback,
				   (void *)&sock_args);
	if(rc != 0) {

		printf("Error occurred, thread could not be created, errno = %d\n", rc);
		exit(0);
	}
}

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

    routingtb_syn_thread();

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
