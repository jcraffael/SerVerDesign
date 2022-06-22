#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include "../routing_table/table.h"
#include "../update_table/upd_table.h"

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128

//pthread_mutex_t lock;
// int data_socket;
pthread_t pthread1;
pthread_t pthread2;

struct arg_struct {
    //char *buffer;
    int data_socket;
    rout_entry_t *head;
    
};


static void *
tbsync_fn_callback(void *arg) {

    puts("Sync thread fn.");
    struct arg_struct *args = (struct arg_struct *)arg;
    rout_entry_t *head = args -> head;
    int data_socket = args -> data_socket;
    printf("Data socket of syncing th handle is %d\n", data_socket);
    char buffer[BUFFER_SIZE];
	 while(1)
    {
          /* Receive result. */
        memset(buffer, 0, BUFFER_SIZE);
        
        //pthread_mutex_lock(&lock);
        int ret = read(data_socket, buffer, BUFFER_SIZE);
        //ret = read(data_socket, &head, BUFFER_SIZE);
        if (ret == -1) {
            perror("read");
            exit(EXIT_FAILURE);
         }
        
        //pthread_mutex_unlock(&lock);

        if(update_routing_table(head, buffer, sizeof(buffer)) == 0)
            continue;

        puts("Updated table is:");
        rout_entry_t* next_node = head -> next;
        
        while(next_node)
        {
            printf("%s  %s  %s\n", next_node -> entry.destination, next_node -> entry.gateway_ip, next_node -> entry.oif);
            next_node = next_node -> next;
        }
    }
}

static void *
send_fn_callback(void *arg)
{
    struct arg_struct *args = (struct arg_struct *)arg;
    int data_socket = args -> data_socket;
    //int data_socket = *((int *)args);
    printf("Data socket of sending th in the handle is %d\n", data_socket);
    char buffer[BUFFER_SIZE];
    /* Send arguments. */
    puts("Send thread fn.");
    while(1){
        memset(buffer, 0, BUFFER_SIZE);

        printf("Enter routing msg to send to server :\n");
        scanf("%s", buffer);
        //pthread_mutex_lock(&lock);

        int ret = write(data_socket, buffer, sizeof(buffer));
        if (ret == -1) {
            perror("write");
            break;
        }

        
        printf("\nNo of bytes sent = %d, buffer size = %ld\n", ret, sizeof(buffer)); 

        //pthread_mutex_unlock(&lock);

    } 
}
void
routingtb_syn_thread(/*int data_socket*/) {

    
    // pthread_attr_t attr;

	// pthread_attr_init(&attr);
	// pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED /*PTHREAD_CREATE_JOINABLE*/);

    char buffer[BUFFER_SIZE];
    rout_entry_t head;
    memset(head.entry.destination, 0, sizeof(head.entry.destination));
    memset(head.entry.gateway_ip, 0, sizeof(head.entry.gateway_ip));
    memset(head.entry.oif, 0, sizeof(head.entry.oif));
    head.next = NULL;
    
    struct sockaddr_un addr;
    int ret;

    /* Create data socket. */

    int data_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (data_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    else
        printf("Data socket of syncing th is %d\n", data_socket);

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

    /* Receive the routing table from server */
    // memset(buffer, 0, sizeof(buffer));
    // ret = read(data_socket, buffer, BUFFER_SIZE);
    // if (ret == -1) {
    //     fprintf(stderr, "Failed to receive routing table from server.\n");
    //     exit(EXIT_FAILURE);
    // }

	/* Take some argument to be passed to the thread fn,
 	 * Look after that you always paas the persistent memory
 	 * as an argument to the thread, do not pass caller's 
 	 * local variables Or stack Memory*/	
	struct arg_struct sock_args = {data_socket, &head};

    struct arg_struct *_sock_args = malloc(sizeof(struct arg_struct));
    _sock_args = &sock_args;
 
    
	/* Return 0 on success, otherwise returns errorcode, all
 	 * pthread functions return -ve error code on failure, they
 	 * do not set global 'errno' variable */

  
    puts("Create sync thread.");
    int rc = pthread_create(&pthread1, 
                NULL,
                tbsync_fn_callback,
                (void *)_sock_args);
    if(rc != 0) {

        printf("Error occurred, thread could not be created, errno = %d\n", rc);
        exit(0);
    }

}


void
tb_send_thread(/*int data_socket*/) {

	
    // pthread_attr_t attr;

	// pthread_attr_init(&attr);
	// pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED /*PTHREAD_CREATE_JOINABLE*/);

    struct sockaddr_un addr;
    int ret;

    /* Create data socket. */

    int data_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (data_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    else
        printf("Data socket of sending th is %d\n", data_socket);

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

	/* Take some argument to be passed to the thread fn,
 	 * Look after that you always paas the persistent memory
 	 * as an argument to the thread, do not pass caller's 
 	 * local variables Or stack Memory*/	

    rout_entry_t *nul_ptr = NULL;
	struct arg_struct sock_args = {data_socket, nul_ptr};
    struct arg_struct *_sock_args = malloc(sizeof(struct arg_struct));

    _sock_args = &sock_args;
	/* Return 0 on success, otherwise returns errorcode, all
 	 * pthread functions return -ve error code on failure, they
 	 * do not set global 'errno' variable */

    
    puts("Create send thread.");
    int rc = pthread_create(&pthread2, 
                NULL,
                send_fn_callback,
                (void *)_sock_args);
    if(rc != 0) {

        printf("Error occurred, thread could not be created, errno = %d\n", rc);
        exit(0);
    }

    //printf("Data socket of sending th converted is %d\n",_sock_args ->data_socket);


}

int
main(int argc, char *argv[])
{
    
    tb_send_thread();
    sleep(2);
    routingtb_syn_thread();

    pthread_join(pthread1, NULL);
    pthread_join(pthread2, NULL);


   

    /* Ensure buffer is 0-terminated. */

    // buffer[BUFFER_SIZE - 1] = 0;

    // printf("Result = %s\n", buffer);

    // /* Close socket. */

    // close(data_socket);

    // exit(EXIT_SUCCESS);
}
