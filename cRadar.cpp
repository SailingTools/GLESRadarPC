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
#include <time.h>

extern "C" {
  #include "esUtil.h"
}

#include "cRadar.h"
//#include "ui_window.h"

//#include <stdlib.h>
//#include "esUtil.h"

#define UDP_IP "192.168.0.1"
#define UDP_PORT 10001

void error(const char *msg)
{
    perror(msg);
    exit(0);
}

char* hexString(char *buf, int size)
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

void *recieveLoop(void *arg)
{
    int counter = 0;
    run_loops = 1;

    printf("Starting reciever loop\n");
    while (run_loops) 
    {
        if (recieve() < 0)
            break;
    };
    printf("Qutting reciever loop\n");
}

int recieve()
{
    nBytes = recvfrom(clientSocket, recv_buffer, 3000, 0, NULL, NULL);
    //nBytes = recv(clientSocket, recv_buffer, 3000, 0);
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


void *inputLoop(void *arg)
{
    char key[10];
    run_loops = 1;

    printf("Starting Input Loop\n");
    while (run_loops)
    {
        printf("Enter a command \n");
        fgets(key,10,stdin);

        switch(key[0]) {
            case 'q':
                printf("Quitting...\n");
                run_loops = 0;
                break;
            default:
                printf("Command not found\n");
        }        
    };
    printf("Quitting input loop\n");
}

void send(char *msg, int nBytes)
{
    printf("-> Sending %i bytes: %s\n", nBytes, hexString(msg, nBytes));
    sendto(clientSocket, msg, nBytes, 0, (struct sockaddr *)&serverAddr, addr_size); 
}

void makeConnection(char ip_address[], int portNum)
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

void setRange(int range)
{
    char msg[] = "\x26\x20\x00\x00\x0d";
    for (int i=0; i<3; i++)
    {
        msg[3-i] += ((range >> 8*i) & 0xff); 
    }
    send(msg, 5);
    recieve();
}

void pingRadar()
{
    /* Function that pings the radar every 5 seconds to keep it alive */
    time_t now;
    double duration; 
    char msg[5] = "\x26\xa7\x11\x0d";

    now = time(NULL);
    if ((now - last_ping) > 5)
    {
        if (scanning)
            char msg[5] = "\x26\xab\x11\x0d";
        send(msg, 4);
        last_ping = now;
    };
}

void main_loop()
{
    printf("Starting main render/ping loop\n");
    run_loops = 1;
    while (run_loops)
    {
        pingRadar();
    }
    printf("Quitting main loop\n");
}


void Init(char ip_address[], int portNum)
{
    int i = 0;
    char key[10];
    last_ping = time(NULL);
    scanning = 0;

    /* Open a connection to the server */
    //makeConnection(ip_address, portNum);

    /* Start the listening and input threads */
    //pthread_create(&recvThread,NULL,recieveLoop, 0);
    //pthread_create(&inThread,NULL,inputLoop,0);
    //pthread_create(&logThread,NULL,logWindow,0);
    //pthread_create(&drawThread,NULL,drawWindow,0);

    /* Send some data to the radar every 5 seconds */
    //char msg[] = "\x26\xa7\x11\x0d";
    //send(msg, 4);

    //main_loop();
    drawWindow(0);

    /*Receive message from server*/
    //recieve();

    /* Test the range set function */
    //int ranges[5] = {231, 463, 926, 1389, 66672};
    //for (i = 0; i < 5; i++)
    //{
    //    setRange(ranges[i]);
    //}

    // Close everything and exit 
    printf("Stopping Running Loops\n");
    run_loops = 0;
    pthread_cancel(recvThread);
    pthread_cancel(inThread);
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

    last_ping = time(NULL);
    scanning = 0;

    drawWindow(0);

    //radarControl r(argv[1], atoi(argv[2]));
    //Init(argv[1], atoi(argv[2]));
    
    printf("Stopping Running Loops\n");
    run_loops = 0;
    pthread_cancel(recvThread);
    pthread_cancel(inThread);
    printf("Closing socket connection\n");
    close(clientSocket);
} 
