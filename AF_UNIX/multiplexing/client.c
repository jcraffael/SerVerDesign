#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include "../routing_table/table.h"
#include "../update_table/upd_table.h"
#include "../shm/shm.h"

#define SOCKET_NAME "/tmp/DemoSocket"


extern int
check_rout_msg(char *buffer, char *ip_dev, char *ip_gw, char *port, int b_size);

extern void 
rout_body_clean(rout_body_t entry);

rout_entry_t rout_head;
mac_entry_t mac_head;
pthread_t pthread1;
pthread_t pthread2;

struct arg_struct {
    //char *buffer;
    int data_socket;
    table_entry_t *table_head;
    
};

void
signal_handler()
{
    puts("Flush all tables!");
    flush_mac_table(&mac_head);
    flush_rout_table(&rout_head);
}


static void *
tbsync_fn_callback(void *arg) {

    puts("Sync thread fn.");
    struct arg_struct *args = (struct arg_struct *)arg;
    
    int data_socket = args -> data_socket;

    //mac_entry_t *mac_head = &(args -> table_head -> mac_table_entry);
    if(mac_head.next != NULL) 
        mac_head.next = NULL;

    //rout_entry_t *rout_head = &(args -> table_head -> rout_table_entry);
    
    char buffer[BUFFER_SIZE];

    int pid = getpid();
    int sent_pid = 0;
	while(1)
    {
          /* Receive result. */
        memset(buffer, 0, BUFFER_SIZE);
        
        //pthread_mutex_lock(&lock);
        int ret = read(data_socket, buffer, BUFFER_SIZE);
        
        if (ret == -1) {
            perror("read");
            exit(EXIT_FAILURE);
         }
        

        if(buffer[0] - '0' == L3)
        {
            //rout_entry_t *rout_head = &(args -> table_head -> rout_table_entry);

            //printf("Received buffer is %s\n", buffer + 1);
            if(update_rout_table(&rout_head, buffer + 1, sizeof(buffer) - 1) == 0)
                continue;

            if(sent_pid == 0)
            {
                memset(buffer, 0, BUFFER_SIZE);
                sprintf(buffer, "%d", pid);

                write(data_socket, buffer, sizeof(buffer));
                sent_pid = 1;
            }
            
        }
        else if(buffer[0] - '0' == L2)
        {
            //mac_entry_t *mac_head = &(args -> table_head -> mac_table_entry);

            if(update_mac_table(&mac_head, buffer + 1, sizeof(buffer) - 1) == 0)
                 continue;

        }
        else if(strcmp(buffer, "st") == 0)
        {
            puts("Updated mac table is:");
            mac_entry_t* next_node = mac_head.next;
            
            while(next_node)
            {
                printf("%s; ", next_node -> entry.mac);
                
                char linked_IP[IP_SIZE];
                read_from_shared_memory (next_node -> entry.mac, 
                         linked_IP, IP_SIZE, IP_SIZE);
                printf("the linked IP is %s\n", linked_IP);
                next_node = next_node -> next;
            }            
            puts("Updated rout table is:");
            
            rout_entry_t* n_node = rout_head.next;
            
            while(n_node)
            {
                printf("%s  %s  %s\n", n_node -> entry.destination, n_node -> entry.gateway_ip, n_node -> entry.oif);
                n_node = n_node -> next;
            }
        }
        else
        {
            printf("Unknown msg: %s\n", buffer);

            exit(EXIT_FAILURE);
            //continue;
        }

        signal(SIGUSR1, signal_handler);
        
    }
}

static void *
send_fn_callback(void *arg)
{
    struct arg_struct *args = (struct arg_struct *)arg;
    int data_socket = args -> data_socket;
    
    //printf("Data socket of sending th in the handle is %d\n", data_socket);
    char buffer[BUFFER_SIZE];
    /* Send arguments. */
    
    while(1){
        memset(buffer, 0, BUFFER_SIZE);

        rout_body_t rout_entry;
        rout_body_clean(rout_entry);
        int op_code;
        do
        {
            printf("Enter routing msg to send to server :\n");
            scanf("%s", buffer);
        } while (!(op_code = check_rout_msg(buffer, rout_entry.destination, rout_entry.gateway_ip, rout_entry.oif, sizeof(buffer) - 1)));
        
        memset(buffer, 0, BUFFER_SIZE);
        *buffer = '2';
        *(buffer + 1) = op_code + '0';
        *(buffer + 2) = ',';
        memcpy(buffer + 3, &rout_entry, sizeof(rout_entry));

        int ret = write(data_socket, buffer, sizeof(buffer));
        if (ret == -1) {
            perror("write");
            break;
        }
        
        printf("\nNo of bytes sent = %d, buffer size = %ld\n", ret, sizeof(buffer)); 
    } 
}
void
routingtb_syn_thread(/*int data_socket*/) {

    char buffer[BUFFER_SIZE];
    
    rout_body_clean(rout_head.entry);
    rout_head.next = NULL;
    
    
    memset(mac_head.entry.mac, 0, MAC_SIZE);
    mac_head.next = NULL;

    struct sockaddr_un addr;
    int ret;

    /* Create data socket. */

    int data_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (data_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

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
    table_entry_t table_head;
    table_head.mac_table_entry = mac_head;
    table_head.rout_table_entry = rout_head;
	struct arg_struct sock_args = {data_socket, &table_head};

    struct arg_struct *_sock_args = malloc(sizeof(struct arg_struct));
    _sock_args = &sock_args;
 
    
	/* Return 0 on success, otherwise returns errorcode, all
 	 * pthread functions return -ve error code on failure, they
 	 * do not set global 'errno' variable */

  
    puts("Create sync thread.");
    int rc = pthread_create(&pthread2, 
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

    struct sockaddr_un addr;
    int ret;

    /* Create data socket. */

    int data_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (data_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
  
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

    table_entry_t *nul_ptr = NULL;
	struct arg_struct sock_args = {data_socket, nul_ptr};
    struct arg_struct *_sock_args = malloc(sizeof(struct arg_struct));

    _sock_args = &sock_args;
	/* Return 0 on success, otherwise returns errorcode, all
 	 * pthread functions return -ve error code on failure, they
 	 * do not set global 'errno' variable */

    
    puts("Create sending thread.");
    int rc = pthread_create(&pthread1, 
                NULL,
                send_fn_callback,
                (void *)_sock_args);
    if(rc != 0) {

        printf("Error occurred, thread could not be created, errno = %d\n", rc);
        exit(0);
    }
}


int
main(int argc, char *argv[])
{
    
    tb_send_thread();
    sleep(1);
    routingtb_syn_thread();


    pthread_join(pthread1, NULL);
    pthread_join(pthread2, NULL);


    // /* Close socket. */

    // close(data_socket);

    // exit(EXIT_SUCCESS);
}
