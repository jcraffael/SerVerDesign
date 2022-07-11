CC = gcc
CFLAGS = -Wall -Wextra
DEPSRC = update_table/upd_table.c shm/shm.c utils.c

default:
	$(CC) $(CFLAGS) server.c $(DEPSRC) -o server -lrt
	$(CC) $(CFLAGS) client.c $(DEPSRC) -o client -pthread -lrt

# server: 
# 	$(CC) $(CFLAGS) server.c $(DEPSRC) -o server -lrt

# client: 
# 	$(CC) $(CFLAGS) client.c $(DEPSRC) -o client -pthread -lrt



