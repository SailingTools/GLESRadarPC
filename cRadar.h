/****************************************************************************
 * A basic standalone implementation of pyRadar using OpenGL(ES)
 * for fast rendering of the Radar
  ***************************************************************************
 */

pthread_t recvThread;
pthread_t inThread;
pthread_t logThread;
pthread_t drawThread;

char recv_buffer[3000];
char hex_buffer[9000];

int clientSocket, portNum, nBytes;
struct sockaddr_in serverAddr;
socklen_t addr_size;
int scanning;
time_t last_ping;
int run_loops;

void Init(char ip_address[], int portNum);
void *recieveLoop(void *arg);
int recieve();
void send(char *msg, int nBytes);
char* hexString(char *buf, int size);
void *inputLoop(void *arg);
void main_loop();
void makeConnection(char ip_address[], int portNum);
void setRange(int range);
extern "C" void pingRadar();

void *logWindow(void *arg);
extern "C" void *drawWindow(void *arg);
//extern "C" {
//  #include "esUtil.h"
//}


