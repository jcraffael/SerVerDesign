#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
//include "../routing_table/table.h"
#include "../update_table/upd_table.h"
#include "../shm/shm.h"
#define SOCKET_NAME "/tmp/DemoSocket"


#define MAX_CLIENT_SUPPORTED    32

char buffer[BUFFER_SIZE];
extern void rout_body_clean(rout_body_t entry);

/*An array of File descriptors which the server process
 * is maintaining in order to talk with the connected clients.
 * Master skt FD is also a member of this array*/
int monitored_fd_set[MAX_CLIENT_SUPPORTED];

/* Keep track of number of connected client */
int conn_clients = 0;

/*Each connected client's intermediate result is 
 * maintained in this client array.*/
//int client_result[MAX_CLIENT_SUPPORTED] = {0};

/*Remove all the FDs, if any, from the the array*/
static void
intitiaze_monitor_fd_set(){

    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++)
        monitored_fd_set[i] = -1;
}

/*Add a new FD to the monitored_fd_set array*/
static void
add_to_monitored_fd_set(int skt_fd){

    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++){

        if(monitored_fd_set[i] != -1)
            continue;
        monitored_fd_set[i] = skt_fd;
        break;
    }
    conn_clients++;
}

/*Remove the FD from monitored_fd_set array*/
static void
remove_from_monitored_fd_set(int skt_fd){

    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++){

        if(monitored_fd_set[i] != skt_fd)
            continue;

        monitored_fd_set[i] = -1;
        break;
    }

    conn_clients--;
}

/* Clone all the FDs in monitored_fd_set array into 
 * fd_set Data structure*/
static void
refresh_fd_set(fd_set *fd_set_ptr){

    FD_ZERO(fd_set_ptr);
    int i = 0;
    for(; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] != -1){
            FD_SET(monitored_fd_set[i], fd_set_ptr);
        }
    }
}

/*Get the numerical max value among all FDs which server
 * is monitoring*/

static int
get_max_fd(){

    int i = 0;
    int max = -1;

    for(; i < MAX_CLIENT_SUPPORTED; i++){
        if(monitored_fd_set[i] > max)
            max = monitored_fd_set[i];
    }

    return max;
}

static void
syncronize_nw_table(char *buffer, int b_size)
{
    for(int i = 2; i < MAX_CLIENT_SUPPORTED || i <= conn_clients; i++){
        if(monitored_fd_set[i] != -1)
        {
            int ret = write(monitored_fd_set[i], buffer, b_size);
            if (ret == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void
compose_rout_sync_msg(rout_body_t *msg)
{
    memset(buffer, 0, sizeof(buffer));
    char *ptr = buffer;
    strcpy(ptr, "21,");

    memcpy(ptr + 3, msg, sizeof(rout_body_t));
}

void
compose_mac_sync_msg(mac_body_t *msg)
{
    memset(buffer, 0, sizeof(buffer));
    char *ptr = buffer;
    strcpy(ptr, "11,");
    strcat(ptr, msg -> mac);
    
}


static void
brdcast_rout_table(rout_entry_t *rout_head, int data_socket)
{
    rout_entry_t *one_entry = rout_head -> next;
    if(one_entry == NULL) return;

    rout_body_t msg = one_entry -> entry;
    compose_rout_sync_msg(&msg);

    int ret = write(data_socket, buffer, sizeof(buffer));
            if (ret == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
    brdcast_rout_table(one_entry, data_socket);
}

static void
brdcast_mac_table(mac_entry_t *mac_head, int data_socket)
{
    mac_entry_t *one_entry = mac_head -> next;
    if(one_entry == NULL) return;

    mac_body_t msg = one_entry -> entry;
    compose_mac_sync_msg(&msg);

    int ret = write(data_socket, buffer, sizeof(buffer));
            if (ret == -1) {
                perror("write");
                exit(EXIT_FAILURE);
            }
    brdcast_mac_table(one_entry, data_socket);
}
int
main(int argc, char *argv[])
{
    rout_entry_t rout_head;
    // memset(rout_head.entry.destination, 0, sizeof(rout_head.entry.destination));
    // memset(rout_head.entry.gateway_ip, 0, sizeof(rout_head.entry.gateway_ip));
    // memset(rout_head.entry.oif, 0, sizeof(rout_head.entry.oif));
    rout_body_clean(rout_head.entry);
    rout_head.next = NULL;

    mac_entry_t mac_head;
    memset(mac_head.entry.mac, 0, sizeof(mac_head.entry.mac));
    mac_head.next = NULL;
    struct sockaddr_un name;
    
#if 0  
    struct sockaddr_un {
        sa_family_t sun_family;               /* AF_UNIX */
        char        sun_path[108];            /* pathname */
    };
#endif

    int ret;
    int connection_socket;
    int data_socket;
    int msg_integrity = 0;
    //sync_msg_t *data = malloc(sizeof(sync_msg_t));
    
    fd_set readfds;
    int comm_socket_fd, i;
    intitiaze_monitor_fd_set();
    add_to_monitored_fd_set(0);

    /*In case the program exited inadvertently on the last run,
     *remove the socket.
     **/

    unlink(SOCKET_NAME);

    /* Create Master socket. */

    /*SOCK_DGRAM for Datagram based communication*/
    connection_socket = socket(AF_UNIX, SOCK_STREAM, 0);

    if (connection_socket == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    printf("Master socket created\n");

    /*initialize*/
    memset(&name, 0, sizeof(struct sockaddr_un));

    /*Specify the socket Cridentials*/
    name.sun_family = AF_UNIX;
    strncpy(name.sun_path, SOCKET_NAME, sizeof(name.sun_path) - 1);

    /* Bind socket to socket name.*/
    /* Purpose of bind() system call is that application() dictate the underlying 
     * operating system the criteria of recieving the data. Here, bind() system call
     * is telling the OS that if sender process sends the data destined to socket "/tmp/DemoSocket", 
     * then such data needs to be delivered to this server process (the server process)*/
    ret = bind(connection_socket, (const struct sockaddr *) &name,
            sizeof(struct sockaddr_un));

    if (ret == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    printf("bind() call succeed\n");
    /*
     * Prepare for accepting connections. The backlog size is set
     * to 20. So while one request is being processed other requests
     * can be waiting.
     * */

    ret = listen(connection_socket, 20);
    if (ret == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    /*Add master socket to Monitored set of FDs*/
    add_to_monitored_fd_set(connection_socket);

    /* This is the main loop for handling connections. */
    /*All Server process usually runs 24 x 7. Good Servers should always up
     * and running and shold never go down. Have you ever seen Facebook Or Google
     * page failed to load ??*/
    for (;;) {

        refresh_fd_set(&readfds); /*Copy the entire monitored FDs to readfds*/
        /* Wait for incoming connection. */
        printf("Waiting on select() sys call\n");

        /* Call the select system call, server process blocks here. 
         * Linux OS keeps this process blocked untill the connection initiation request Or 
         * data requests arrives on any of the file Drscriptors in the 'readfds' set*/

        select(get_max_fd() + 1, &readfds, NULL, NULL, NULL);

        if(FD_ISSET(connection_socket, &readfds)){

            /*Data arrives on Master socket only when new client connects with the server (that is, 'connect' call is invoked on client side)*/
            printf("New connection recieved recvd, accept the connection\n");

            data_socket = accept(connection_socket, NULL, NULL);

            if (data_socket == -1) {
                perror("accept");
                exit(EXIT_FAILURE);
            }

            //printf("Connection accepted from client\n");

            add_to_monitored_fd_set(data_socket);

            if(rout_head.next != NULL) 
            {
                brdcast_rout_table(&rout_head, data_socket);
                
            }
            if(mac_head.next != NULL) 
            {
                
                brdcast_mac_table(&mac_head, data_socket);
            }

        }
        else if(FD_ISSET(0, &readfds)){

            mac_body_t mac_entry;
            char ip_addr[IP_SIZE];
            
            
            memset(ip_addr, 0, IP_SIZE);
            memset(mac_entry.mac, 0, MAC_SIZE);
            //puts("Type MAC address here:");
            
            int op_code;
           do
           {
                memset(buffer, 0, BUFFER_SIZE);
                ret = read(0, buffer, BUFFER_SIZE);
           } while (!(op_code = check_mac_msg(buffer, mac_entry.mac, ip_addr, BUFFER_SIZE)));
           
            
            printf("Mac is %s and Ip is %s\n", mac_entry.mac, ip_addr);

            create_and_write_shared_memory(mac_entry.mac, ip_addr, IP_SIZE);
            memset(buffer, 0, BUFFER_SIZE);
            *buffer = '1';
            *(buffer + 1) = op_code + '0';
            *(buffer + 2) = ',';
            
            memcpy(buffer + 3, &mac_entry, sizeof(mac_body_t));
            if(update_mac_table(&mac_head, buffer + 1, sizeof(buffer) - 1) == 0)
                continue;
            
            syncronize_nw_table(buffer, BUFFER_SIZE);

        }
        else /* Data arrives on some other client FD*/
        {
            /*Find the client which has send us the data request*/
            //comm_socket_fd = -1;
            for(i = 0; i < MAX_CLIENT_SUPPORTED; i++){

                if(FD_ISSET(monitored_fd_set[i], &readfds)){
                    //comm_socket_fd = monitored_fd_set[i];

                    /*Prepare the buffer to recv the data*/
                    memset(buffer, 0, BUFFER_SIZE);

                    /* Wait for next data packet. */
                    /*Server is blocked here. Waiting for the data to arrive from client
                     * 'read' is a blocking system call*/
                    printf("Waiting for data from the client\n");
                    ret = read(monitored_fd_set[i], buffer, BUFFER_SIZE);

                    if (ret == -1) {
                        perror("read");
                        exit(EXIT_FAILURE);
                    }
                    
                    if( buffer[0] == 0)
                    {
                        /* Close socket. */
                        close(comm_socket_fd);
                        //client_result[i] = 0; 
                        remove_from_monitored_fd_set(comm_socket_fd);
                        continue;
                    }

                    /* Add received summand. */
                    if(buffer[0] - '0' == L3)
                    {
                        if(update_rout_table(&rout_head, buffer + 1, sizeof(buffer) - 1) == 0)
                            continue;
                    }
                    else if(buffer[0] - '0' == L2)
                    {
                        if(update_mac_table(&mac_head, buffer + 1, sizeof(buffer) - 1) == 0)
                            continue;
                    }
                    else
                    ;

                    /*go to select() and block*/
                }
            }
            //if(buffer[0] != '')
            syncronize_nw_table(buffer, BUFFER_SIZE);
            continue;

        }
    } /*go to select() and block*/

    /*close the master socket*/
    close(connection_socket);
    remove_from_monitored_fd_set(connection_socket);
    printf("connection closed..\n");

    /* Server should release resources before getting terminated.
     * Unlink the socket. */

    unlink(SOCKET_NAME);
    exit(EXIT_SUCCESS);
}
