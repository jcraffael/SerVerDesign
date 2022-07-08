#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <regex.h>
#include <arpa/inet.h>
#include <ctype.h>
#include "../update_table/upd_table.h"

#define MAC_LEN 17
typedef enum{
    ERROR = 0,
    CIDR,
    IP,
    PORT,
    MAC
}nw_msg;


void rout_body_clean(rout_body_t entry)
{
    memset(entry.destination, 0, CIDR_SIZE);
    memset(entry.gateway_ip, 0, IP_SIZE);
    memset(entry.oif, 0, OIF_SIZE);
}

int check_ip_msg_format(char *str)
{
    //
    if(str == NULL) return 0;

    if(str == strstr(str, "eth"))
        return PORT;

    struct sockaddr_in sa;
    struct in_addr addr;
    printf("The string is %s\n", str);
    int result = inet_pton(AF_INET, str, &addr);
    if(result)
        return IP;
    else
    {
        char new_str[CIDR_SIZE], *ptr = new_str;
        memcpy(new_str, str, CIDR_SIZE);
        char *ip = strtok(ptr, "/");
        char *mask = strtok(NULL, "/");
        //if(ip != NULL && inet_pton(AF_INET, ip, &(sa.sin_addr)) && mask && atoi(mask) <= 32)
        if(ip != NULL)
        {
            if(inet_pton(AF_INET, ip, &(sa.sin_addr)))
            {
                if(mask && atoi(mask) <= 32)
                   return CIDR;
            }
        }
            //return CIDR;
    }

    //printf("Result is %d, string is %s\n", result, str);
    
    return 0;
}

int check_mac_msg_format(char *str)
{
    
    if(str == NULL) return 0;

    char *mac = str;
    
    for(int i = 0; i < MAC_LEN; i++)
    {
        if((i + 1) % 3 == 0)
        {
            if(*(mac + i) != ':') 
                return 0;
        }
        else
        {
            if(!isalpha(*(mac + i)) && !isdigit(*(mac + i)))
                return 0;
        }
    }

    return MAC;
}

int
check_rout_msg(char *buffer, char *ip_dev, char *ip_gw, char *port, int b_size)
{
    int msg_integrity = 0;
    //int b_size = sizeof *buffer;
    char rest[b_size], *ptr = rest;
    memcpy(rest, buffer, b_size);
    char *opcode = strtok_r(rest, ",", &ptr);
    char *token = strtok_r(ptr, ",", &ptr);
    //data ->op_code = strtol(opcode, NULL, 0);
    int op_code = atoi(opcode);
    if(op_code <= 0 || op_code > 3)
    {
        puts("Operational code error!");
        return 0;
    }
    char *tk = strtok_r(token, ";", &ptr);

    if(fill_entry(ip_dev, tk) && check_ip_msg_format(ip_dev) == CIDR)
    {
        if(op_code == DELETE)
        {
            msg_integrity = 1;
            goto CHECK_MSG;
        }

        tk = strtok_r(ptr, ";", &ptr);

        if(fill_entry(ip_gw, tk) && check_ip_msg_format(ip_gw) == IP)
        {
            tk = strtok_r(ptr, ";", &ptr);
            if(fill_entry(port, tk) && check_ip_msg_format(port) == PORT)
                msg_integrity = 1;
            else
                puts("Port format error!");
        }
        else
            puts("GW IP format error!");
    }
    else
        puts("CIDR format error!");
    
    CHECK_MSG:
    if(msg_integrity == 0)
    {
        puts("msg body corrupted.");
        return 0;
    }
    return op_code;
}


int
check_mac_msg(char *buffer, char *mac_addr, char *ip_addr, int b_size)
{
    int msg_integrity = 0;
    //int b_size = sizeof *buffer;
    char rest[b_size], *ptr = rest;
    memset(rest, 0, b_size);
    memcpy(rest, buffer, b_size);
    //int i = 0;
    // for(; *(rest + i) != '\n'; i++)
    // ;
    // *(rest + i) = '\0';
    char *opcode = strtok_r(rest, ",", &ptr);

    int op_code = atoi(opcode);
    if(op_code <= 0 || op_code > 3)
    {
        puts("Operational code error!");
        return 0;
    }
    char *tk = strtok_r(ptr, ";", &ptr);
    if(fill_entry(mac_addr, tk) && check_mac_msg_format(mac_addr) == MAC)
    {
        //tk = strtok_r(ptr, ";", &ptr);
        if(fill_entry(ip_addr, ptr) && check_ip_msg_format(ip_addr) == IP)
            msg_integrity = 1;
    }
    else
        puts("MAC addr format error");
        

    if(msg_integrity == 0)
    {
        puts("msg body corrupted.");
        return 0;
    }
    return op_code;
}