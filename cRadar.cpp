/****************************************************************************
 * A basic standalone implementation of pyRadar using wxWidgets and OpenGL(ES)
 * for fast rendering of the Radar
  ***************************************************************************
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h> 
#include <arpa/inet.h>

#include <pthread.h>
#include <ctime>
#include <cstdlib>

#include "cRadar.h"

//#include <stdlib.h>
//#include "esUtil.h"

#define UDP_IP "192.168.0.1"
#define UDP_PORT 10001

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

char* radarControl::hexString(char *buf, int size)
{
int i;
memset(hex_buffer, 0, sizeof(hex_buffer));
char* buf_ptr = hex_buffer;
for (i = 0; i < size; i++)
{
    buf_ptr += sprintf(buf_ptr, i < size - 1 ? "%02X:" : "%02X\0", buf[i]);
}
return hex_buffer;
}


void radarControl::send(char *msg, int nBytes)
{
    printf("-> Sending %i bytes: %s\n", nBytes, hexString(msg, nBytes));
    sendto(clientSocket, msg, nBytes, 0, (struct sockaddr *)&serverAddr, addr_size); 
}

void radarControl::makeConnection(char ip_address[], int portNum)
{
    printf("Creating socket\n");
    clientSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (clientSocket < 0) 
        error("ERROR opening socket");

    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(portNum);
    serverAddr.sin_addr.s_addr = inet_addr(ip_address);
    addr_size = sizeof serverAddr;

    /* Try to connect to the server*/
    printf("Trying to connect to server %s on port %i... \n", ip_address, portNum);
    //bind(clientSocket, (struct sockaddr *) &serverAddr, addr_size);
    if (connect(clientSocket, (struct sockaddr *) &serverAddr, addr_size) < 0)
        error("ERROR connecting to server");

    printf("Successfully connected to server\n");
}

void radarControl::recieveLoop()
{
    printf("Listinging for input\n");
    int counter = 0;
    while (true) 
    {
        if (recieve() < 0)
            return;
    };
}

int radarControl::recieve(void)
{
    nBytes = recvfrom(clientSocket, recv_buffer, 1024, 0, NULL, NULL);
    //nBytes = recv(clientSocket, recv_buffer, 1024, 0);
    if (nBytes < 0)
    { 
        error("ERROR reading from socket");
        return nBytes;
    }
    //printf("Got %i bytes: %s\n", nBytes, recv_buffer);
    if (nBytes < 1000)
        printf("<- Recieved %i bytes: %s\n", nBytes, hexString(recv_buffer, nBytes));
    return nBytes;
}


void radarControl::setRange(int range)
{
    char msg[] = "\x26\x20\x00\x00\x0d";
    for (int i=0; i<3; i++)
    {
        msg[3-i] += ((range >> 8*i) & 0xff); 
    }
    send(msg, 5);
    recieve();
}

void radarControl::pinger()
{
    /* Function that pings the radar every 5 seconds to keep it alive */
    std::clock_t now;
    double duration; 
    char msg[5] = "\x26\xa7\x11\x0d";

    now = std::clock();
    if (((now - last_ping)/(double)CLOCKS_PER_SEC) > 5)
    {
        if (scanning)
            char msg[5] = "\x26\xab\x11\x0d";
        send(msg, 4);
    };
}

radarControl::radarControl(char ip_address[], int portNum)
{
    int i = 0;
    std::clock_t last_ping = std::clock();
    bool scanning = false;

    /* Open a connection to the server */
    makeConnection(ip_address, portNum);

    /* Start the recieve thread listening */
    pthread_create(&recvThread,NULL,recieve,NULL);

    /* Send some data to the radar every 5 seconds */
    //char msg[] = "\x26\xa7\x11\x0d";
    //send(msg, 4);

    /*Receive message from server*/
    //recieve();

    /* Test the range set function */
    //int ranges[5] = {231, 463, 926, 1389, 66672};
    //for (i = 0; i < 5; i++)
    //{
    //    setRange(ranges[i]);
    //} 
}

radarControl::~radarControl()
{
    printf("Stopping Recieve Loop\n");
    pthread_cancel(recvThread);
    printf("Closing socket connection\n");
    close(clientSocket);
}

int main(int argc, char *argv[])
{
    printf("Starting program\n");
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }

    //radarControl r(argv[1], atoi(argv[2]));

    

} 
