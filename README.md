# SerVerDesign
Server Designing in C
This is a personal project for the Udemy course "Linux Inter Process Communication (IPC) from scratch in C"

The project consists of a unix socket server and a client. The server serves the purpose of: 1. Receive connection request from client and build the connection; 2. Receive IP routing table entry from client and create its own, in the meantime syncronize the table to all the connected clients through unix socket; 3 Receive and save the MAC:IP copy from user input, and broadcast it the mac table to all the connected clients wheares write the linked IP into shared memory; 4. Give instructions to the connected clients ("show table" and "flush all");

The client works in the way that: 1. It creates 2 threads, one for sending routing entry to server and one for receiving syncronized message from server (and build its own routing/mac table). Both threads build their own connection to the server; 2. When receiving instruction "show table" from server the client prints its maintaned routing table and mac table, together with the linked IP for each mac entry by reading from the share memory. 3. When receiving instruction "flush all" client flush its mac and routing tables;

Note:
1. From client side the routing entry message format is OP_CODE,CIDR;IP;INTERFACE, where: 1) OP_CODE is an integer among 1(create), 2(modify) and 3(delete); 2) CIDR is the host IP address plus slash '\' plus an integer between 0 and 32 to indicate subnet mask; 3) Gateway IP; 4) Ethernet interface (starting with "eth");
2. From server side the mac entry message format is OP_CODE,MAC;IP, where: 1) OP_CODE can only be 1(create); 2) MAC address; 3) Linked IP address;
