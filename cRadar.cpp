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
  #include "ui_draw.h"
}

#include "cRadar.h"
//#include "ui_window.h"

//#include <stdlib.h>
//#include "esUtil.h"

#define UDP_IP "192.168.0.1"
#define UDP_PORT 10001

radar_settings gset;
time_t last_onoff;

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
    radar_scanline_pkt packet;
    
    nBytes = recvfrom(clientSocket, recv_buffer, 3000, 0, NULL, NULL);
    //nBytes = recv(clientSocket, recv_buffer, 3000, 0);
    if (nBytes < 0)
    { 
        error("ERROR reading from socket");
        return nBytes;
    }
    //printf("Got %i bytes: %s\n", nBytes, recv_buffer);
    if (nBytes < 1000) {
        //printf("<- Recieved %i bytes: %s\n", nBytes, hexString(recv_buffer, nBytes));
    } else if (nBytes == 1192) {
        //printf("<- Recieved %i bytes\n", nBytes);
        // Copy the recv_buffer data into the packet structure
        memcpy(&packet, recv_buffer, sizeof(radar_scanline_pkt));

        get_settings(&packet);

        //printf("Copying to global buffer at angle %i\n", packet.angle);
        // Copy the scanline data into the global scan buffer
        unsigned char *dest_data = &global_scan_buffer[1024 * (packet.angle - 2) / 4];
        memcpy(dest_data, packet.scanline_data, 1024);
    }
    return nBytes;
}

void get_settings(radar_scanline_pkt *packet)
{
    gset.range_index = (packet->range_index)^256;
    gset.long_pulse  = packet->long_pulse;
    gset.gain_auto   = packet->gain_auto;
    gset.gain_value  = packet->gain_value;
    gset.rain_value  = packet->rain_value;
    gset.sea_auto    = packet->sea_auto;
    gset.sea_value   = packet->sea_value;    
}

void *inputLoop(void *arg)
{
    char key[10] = {0x00};
    int i=0;
    char *p;
    run_loops = 1;

    printf("Starting Input Loop\n");
    while (run_loops)
    {
        printf("Enter a command (h for help)\n");
        fgets(key,10,stdin);

        switch(key[0]) {
            case 'h':
                printf("Help commands... \n");
                print_help();
                break;
            case 'q':
                printf("Quitting...\n");
                run_loops = 0;
                break;
            case 's':
                toggle_scanning();
                break;
            case 'r':
                p = strchr(key, ' ');
                setRange(atoi(p));
                break;
            case 'g':
                p = strchr(key, ' ');
                setGain(atoi(p));
                break;
            case 'c':
                p = strchr(key, ' ');
                setSea(atoi(p));
                break;
            case 'a':
                p = strchr(key, ' ');
                setRain(atoi(p));
                break;
            case 'p':
                p = strchr(key, ' ');
                setPulse(atoi(p));
                break;
            case 'i':
                p = strchr(key, ' ');
                setIntervalScan(atoi(p));
                break;
            case 'w':
                p = strchr(key, ' ');
                setIntervalWait(atoi(p));
                break;
            case 'd':
                print_settings();
                break;
            default:
                printf("Command not found\n");
                break;
        }        
    };
    printf("Quitting input loop\n");
}

void print_help()
{
    printf("/*===================*/\n");
    printf("Help Commands:\n");
    printf("q = Quit\n");
    printf("s = Toggle (start/stop) Scanning\n");
    printf("r [0-15]= Set Range to index (0-15)\n");
    printf("g [%]= Set Gain to percent\n");
    printf("c [%]= Set Sea to percent\n");
    printf("a [%]= Set Rain to percent\n");
    printf("p [0|1]= Set Pulse to 0 or 1\n");

    printf("i [seconds]= Set Interval Scan Time (seconds)\n");
    printf("w [seconds]= Set Interval Wait Time (seconds)\n");
    printf("/*===================*/\n");
}

void print_settings()
{
    printf("/*===================*/\n");
    printf("CURRENT SETTINGS:\n");
    printf("Range = %i\n", gset.range_index);
    printf("Gain  = %i [auto=%i]\n", gset.gain_value, gset.gain_auto);
    printf("Sea  = %i [auto=%i]\n", gset.sea_value, gset.sea_auto);
    printf("Rain  = %i\n", gset.rain_value);
    printf("Pulse = %i\n", gset.long_pulse);

    printf("Interval Scan = %i seconds\n", gset.interval_scan);
    printf("Interval Wait = %i seconds\n", gset.interval_wait);
    printf("/*===================*/\n");
}

void send(char *msg, int nBytes)
{
    //printf("-> Sending %i bytes: %s\n", nBytes, hexString(msg, nBytes));
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

void setRange(int value)
{
    double ranges_miles[] = { 0.125, 0.25, 0.5, 0.75, 1.0, 1.5, 2.0, 3.0, 4.0, 6.0, 8.0, 12.0, 16.0, 24.0, 32.0, 36.0 };
    if ( value > 15 ) { value = 15; };
    if ( value < 0 ) { value = 0; };
    int range_meters = int(ranges_miles[value]*1852);

    printf("Setting RANGE to %f miles (%i meters)... \n", ranges_miles[value], range_meters);

    char msg[] = "\x26\x20\x00\x00\x0d";
    for (int i=0; i<3; i++)
    {
        msg[3-i] += ((range_meters >> 8*i) & 0xff); 
    }
    send(msg, 5);
}

void setGain(int value)
{
    printf("Setting GAIN to %i... \n", value);

    char msg[] = "\x24\x47\x00\x0d";
    if ( value >= 0 && value < 100 )
    {
        msg[2] += ((value >> 8*0) & 0xff); 
    } else {
        msg[1] = '\x67';
        msg[2] = '\x11';
    }
    send(msg, 4);
}

void setSea(int value)
{
    printf("Setting SEA to %i... \n", value);

    char msg[] = "\x24\x53\x00\x0d";
    if ( value >= 0 && value < 100 )
    {
        msg[2] += ((value >> 8*0) & 0xff); 
    } else {
        msg[1] = '\x80';
        msg[2] = '\x22';
    }
    send(msg, 4);
}

void setRain(int value)
{
    printf("Setting RAIN to %i... \n", value);

    char msg[] = "\x26\x46\x00\x0d";
    if ( value >= 0 && value < 100 )
    {
        msg[2] += ((value >> 8*0) & 0xff); 
    }
    send(msg, 4);
}

void setPulse(int value)
{
    printf("Setting PULSE to %i... \n", value);

    char msg[] = "\x26\xa4\x01\x0d";
    if ( value >= 0 && value <= 1 )
    {
        msg[2] += ((value >> 8*0) & 0xff); 
    }
    send(msg, 4);
}

void setIntervalScan(int value)
{
    printf("Setting Interval Scan Time to %i seconds... \n", value);
    gset.interval_scan = value;
}

void setIntervalWait(int value)
{
    printf("Setting wait time between Interval Scan to %i seconds... \n", value);
    gset.interval_wait = value;
}

void start_radar()
{
    printf("Starting Scanning\n");
    char msg[] = "\x26\x74\x11\x0d";
    scanning = 1;
    send(msg, 4);

    last_onoff = time(NULL);
}

void stop_radar()
{
    printf("Stopping Scanning\n");
    char msg[] = "\x26\x74\x00\x0d";
    scanning = 0;
    send(msg, 4);
    
    last_onoff = time(NULL);
}

void toggle_scanning()
{
    if (scanning)
    {
        stop_radar();
    } else {
        start_radar();
    }
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

    /* Auto Start/Stop the radar if set*/
    if (gset.interval_scan > 0 ){
      if ( scanning == 0 ) {
        if ((now - last_onoff) > gset.interval_wait){ start_radar(); };
      } else {
        if ((now - last_onoff) > gset.interval_scan){ stop_radar(); };
      };
    };
}

void initialize_radar()
{
    printf("Running Initialization commands\n");
    int i=0;
    int s=0;
    char messages[33][10] = {
        "\x26\xfe\xff\xff\xff\x00\x0d",
        "\x24\x4e\x11\x0d",
        "\x26\xfe\xff\xff\xff\x00\x0d",
        "\x26\x72\xff\x0d",
        "\x26\x9a\x11\x0d",
        "\x26\xad\x02\x0d",
        "\x26\xac\x00\x0d",
        "\x26\xa5\x00\x0d",
        "\x26\x99\x11\x0d",
        "\x24\x41\x22\x0d",
        "\x26\x88\x04\x0d",
        "\x26\x85\x55\x0d",
        "\x24\x67\x00\x0d", 
        "\x26\x82\x08\x0d",
        "\x26\x81\x07\x0d",
        "\x24\x80\x00\x0d",
        "\x26\x84\x0a\x0d",
        "\x26\x83\x0a\x0d",
        "\x26\x87\x07\x0d",
        "\x26\x30\x03\xd3\x0d",
        "\x26\x9c\x00\x00\x00\x00\x0d",
        "\x26\x44\xa5\x0d",
        "\x26\x20\x01\xcf\x0d",
        "\x24\x47\x32\x0d",
        "\x24\x53\x14\x0d",
        "\x26\x46\x32\x0d",
        "\x26\x49\x33\x0d",
        "\x26\x45\x00\x0d",
        "\x26\xa4\x00\x0d",
        "\x24\x47\x32\x0d",
        "\x24\x53\x14\x0d",
        "\x26\x46\x32\x0d",
        "\x26\x49\x33\x0d", 
    };

    for (i=0; i<33; i++){
        // Get the first occurance of the end-byte
        for (s=0; s<10; s++){
            if (messages[i][s] == '\x0d')
                break;
        };
        s += 1;
        //printf("%i bytes: %s\n", s, hexString(messages[i], s));
        send(messages[i], s);
        usleep(50000); /* Five hundred thousand microseconds is half a second. */
    }
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


void radarInit(char ip_address[], int portNum)
{
    /* Initialize the starting buffer */
    memset(global_scan_buffer, 0x00, sizeof(global_scan_buffer));

    /* Generate a random starting buffer
    int i;
    int s = sizeof(global_scan_buffer);
    for (i=0; i<s; i++) { 
        global_scan_buffer[i] = 255.0*rand()/RAND_MAX; 
    };*/

    last_ping = time(NULL);
    scanning = 0;

    /* Open a connection to the server */
    makeConnection(ip_address, portNum);

    /* Start the listening and input threads */
    pthread_create(&recvThread,NULL,recieveLoop, 0);
    pthread_create(&inThread,NULL,inputLoop,0);
    //pthread_create(&logThread,NULL,logWindow,0);
    //pthread_create(&drawThread,NULL,drawWindow,0);

    /* Send the initialization commands */
    initialize_radar();

    /* Set parameters for auto-scanning */
    gset.interval_wait = 600;
    gset.interval_scan = 0;
    last_onoff = time(NULL);

    /* Start the drawing loop*/
    drawWindow(0);


}


int main(int argc, char *argv[])
{
    printf("Starting program\n");
    if (argc < 3) {
        fprintf(stderr,"usage %s hostname port\n", argv[0]);
        exit(0);
    }

    // Star the radar and draw loop
    radarInit(argv[1], atoi(argv[2]));
    
    // Shutdown all the running loops
    printf("Stopping Running Loops\n");
    run_loops = 0;
    pthread_cancel(recvThread);
    pthread_cancel(inThread);
    printf("Closing socket connection\n");
    close(clientSocket);
} 
