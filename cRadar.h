/****************************************************************************
 * A basic standalone implementation of pyRadar using wxWidgets and OpenGL(ES)
 * for fast rendering of the Radar
  ***************************************************************************
 */

using namespace std;

pthread_t recvThread;

class radarControl
{
public:
    void makeConnection(char ip_address[], int portNum);
    void send(char *msg, int nBytes);
    int recieve(void);
    void setRange(int range);

    radarControl(char ip_address[], int portNum);
    ~radarControl(void);
    
private:
    int clientSocket, portNum, nBytes;
    bool scanning;
    std::clock_t last_ping;
    char hex_buffer[3072];
    char recv_buffer[1024];
    struct sockaddr_in serverAddr;
    socklen_t addr_size;

    void recieveLoop();
    void pinger();
    char* hexString(char *buf, int size);
};
