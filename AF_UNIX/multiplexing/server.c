#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>

#define SOCKET_NAME "/tmp/DemoSocket"
#define BUFFER_SIZE 128

#define MAX_CLIENT_SUPPORTED    32

char buffer[BUFFER_SIZE];
typedef enum{
    ERR = 0,
    CREATE,
    UPDATE,
    DELETE
}OPCODE;

typedef struct _msg_body{

    char destination[20];
    //char mask;
    char gateway_ip[16];
    char oif[32];

}msg_body_t;

typedef struct _table_entry{

    msg_body_t entry;
    struct _table_entry *next;

}table_entry_t;

table_entry_t head = {{0}, NULL};

int num_entries = 0;

typedef struct _sync_msg{

    OPCODE op_code;
    msg_body_t msg_body;
}sync_msg_t;

/*An array of File descriptors which the server process
 * is maintaining in order to talk with the connected clients.
 * Master skt FD is also a member of this array*/
int monitored_fd_set[MAX_CLIENT_SUPPORTED];

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

static table_entry_t *
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


static int
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

static int
update_entry(msg_body_t *msg){

    table_entry_t *t_entry = check_entry(msg);
    if(t_entry)
    {
        memcpy(&(t_entry -> entry), msg, sizeof(msg_body_t));
        return 1;
    }
    return 0;
}

static int
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

static int 
send_updated_table(int comm_socket_fd){

    memset(buffer, 0, BUFFER_SIZE);
    //sprintf(buffer, "Result = %d", client_result[i]);
    memcpy(buffer, (void *)&(head.next), sizeof(table_entry_t));
    printf("sending final result back to client\n");
    int ret = write(comm_socket_fd, buffer, BUFFER_SIZE);
    if (ret == -1) {
        perror("write");
        exit(EXIT_FAILURE);
    }
}

static int
fill_entry(char *str1, char *str2)
{
    if(str2 != NULL)
    {
        strcpy(str1, str2);
        return 1;
    }
    return 0;
}

int
main(int argc, char *argv[])
{
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
    sync_msg_t *data = malloc(sizeof(sync_msg_t));
    
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

            printf("Connection accepted from client\n");

            add_to_monitored_fd_set(data_socket);
        }
        else if(FD_ISSET(0, &readfds)){
            memset(buffer, 0, BUFFER_SIZE);
            ret = read(0, buffer, BUFFER_SIZE);
            printf("Input read from console : %s\n", buffer);
        }
        else /* Data srrives on some other client FD*/
        {
            /*Find the client which has send us the data request*/
            i = 0, comm_socket_fd = -1;
            for(; i < MAX_CLIENT_SUPPORTED; i++){

                if(FD_ISSET(monitored_fd_set[i], &readfds)){
                    comm_socket_fd = monitored_fd_set[i];

                    /*Prepare the buffer to recv the data*/
                    memset(buffer, 0, BUFFER_SIZE);

                    /* Wait for next data packet. */
                    /*Server is blocked here. Waiting for the data to arrive from client
                     * 'read' is a blocking system call*/
                    printf("Waiting for data from the client\n");
                    ret = read(comm_socket_fd, buffer, BUFFER_SIZE);

                    if (ret == -1) {
                        perror("read");
                        exit(EXIT_FAILURE);
                    }

                    /* Add received summand. */
                    memset(data, 0, sizeof(sync_msg_t));
                    char *rest = buffer;
                    char *opcode = strtok_r(buffer, ",", &rest);
                    char *token = strtok_r(rest, ",", &rest);
                    data ->op_code = strtol(opcode, NULL, 0);
                    //memcpy(data, (sync_msg_t *)buffer, sizeof(sync_msg_t));
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

                    
                        /* Close socket. */
                        // close(comm_socket_fd);
                        // client_result[i] = 0; 
                        // remove_from_monitored_fd_set(comm_socket_fd);
                    }
                    send_updated_table(comm_socket_fd);
                    continue; /*go to select() and block*/
                }
            }
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
