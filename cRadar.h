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

void radarInit(char ip_address[], int portNum);
void *recieveLoop(void *arg);
int recieve();
void send(char *msg, int nBytes);
char* hexString(char *buf, int size);
void *inputLoop(void *arg);
void main_loop();
void makeConnection(char ip_address[], int portNum);
void setRange(int range);
extern "C" void pingRadar();

void print_help();
void print_settings();
void toggle_scanning();
void setRange(int value);
void setGain(int value);
void setRain(int value);
void setSea(int value);
void setPulse(int value);

void *logWindow(void *arg);
extern "C" void *drawWindow(void *arg);
//extern "C" {
//  #include "esUtil.h"
//}

typedef struct {
    unsigned short  range_index; //16-17
    unsigned char   long_pulse; //18
    unsigned char   gain_auto; //20
    unsigned char   gain_value; //21
    unsigned char   rain_value; //24
    unsigned char   sea_auto; //26
    unsigned char   sea_value; //27
} radar_settings;

typedef struct {
    unsigned char   misc_a[15]; //0-15
    unsigned short  range_index; //16-17
    unsigned char   long_pulse; //18
    unsigned char   misc_b; //19 
    unsigned char   gain_auto; //20
    unsigned char   gain_value; //21
    unsigned char   misc_c[2]; //22-23
    unsigned char   rain_value; //24
    unsigned char   misc_d; //25
    unsigned char   sea_auto; //26
    unsigned char   sea_value; //27
    unsigned char   misc_e[14]; //28-41
    unsigned short  angle; //42-43
    unsigned char   misc_f[124]; //44-167
    unsigned char   scanline_data[1024]; //168-1192 (end)
} radar_scanline_pkt;

void get_settings(radar_scanline_pkt *packet);

