#ifdef NET
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <poll.h>
// for player struct
#include "bumprace.h"

extern struct sockaddr_in client_address, server_address;
extern int skt, levelnum, mode;
extern player user[MAX_PLAYER_NUM];

unsigned char *
serializeInt(unsigned char *buffer, int value){
    buffer[0] = value >> 24;
    buffer[1] = value >> 16;
    buffer[2] = value >> 8;
    buffer[3] = value;
    return buffer + 4;// -> pointer to end
}

unsigned char * serializeFloat(unsigned char *buffer, float value) 
{ 
    unsigned int ivalue = *((unsigned int*)&value); // warning assumes 32-bit "unsigned int"
    buffer[0] = ivalue >> 24;  
    buffer[1] = ivalue >> 16;  
    buffer[2] = ivalue >> 8;  
    buffer[3] = ivalue;  
    return buffer + 4; 
}

unsigned char * 
deSerializeInt(unsigned char *buffer, int *value) {
    *value = 0;
    *value += buffer[0] << 24;
    *value += buffer[1] << 16;
    *value += buffer[2] << 8;
    *value += buffer[3];
    return buffer + 4;
}

unsigned char * 
deSerializeFloat(unsigned char *buffer, float *value) {
    union {
        float f;
        uint64_t u;
    } fu = {.u = 0};
    fu.u += buffer[0] << 24;
    fu.u += buffer[1] << 16;
    fu.u += buffer[2] << 8;
    fu.u += buffer[3];
    *value = fu.f;
    return buffer + 4;
}

unsigned char * 
serializeLevelInit(unsigned char *buffer)
{
    buffer = serializeInt(buffer, levelnum);
    buffer = serializeInt(buffer, mode);
    buffer = serializeInt(buffer, user[0].racernum);
    //buffer = serializeInt(buffer, user[1].racernum);
    return buffer;
}

unsigned char * 
deSerializeLevelInit(unsigned char *buffer)
{
    buffer = deSerializeInt(buffer, &levelnum);
    buffer = deSerializeInt(buffer, &mode);
    buffer = deSerializeInt(buffer, &(user[0].racernum));
    //buffer = deSerializeInt(buffer, &(user[1].racernum));
    return buffer;
}

unsigned char *
serializeClientInit(unsigned char *buffer)
{
    buffer = serializeInt(buffer, user[1].racernum);
    return buffer;
}

unsigned char *
deSerializeClientInit(unsigned char *buffer)
{
    buffer = deSerializeInt(buffer, &user[1].racernum);
    return buffer;
}

#define GAMEINITSIZE 12
#define CGAMEINITSIZE 4
void
ServerGameInit(){
  int size = sizeof(client_address);
  unsigned char recbuffer[CGAMEINITSIZE], buffer[GAMEINITSIZE];

  recvfrom(skt, recbuffer, sizeof(recbuffer), 0, &client_address, (int*) &size);// wait for reply of client
  deSerializeClientInit(recbuffer);
                                                                 
  serializeLevelInit(buffer);
  sendto(skt, buffer, sizeof(buffer), 0, &client_address, size);
}

int
ClientGameInit(){
  int size = sizeof(server_address);
  unsigned char sendbuffer[CGAMEINITSIZE], buffer[GAMEINITSIZE];

  serializeClientInit(sendbuffer);
  sendto(skt, sendbuffer, sizeof(sendbuffer), 0, &client_address, sizeof(client_address));
  if (recvfrom(skt, buffer, sizeof(buffer), 0, &server_address, &size) < 0){
      perror("error: Server did not respond, retrying\n");
      return 1;
  }
  deSerializeLevelInit(buffer);
  return 0;
}

unsigned char * // for sending host controlled player
serializeInGame(unsigned char *buffer, player *Player)
{
    buffer = serializeFloat(buffer, Player->realx);
    buffer = serializeFloat(buffer, Player->realy);
    buffer = serializeFloat(buffer, Player->xspeed);
    buffer = serializeFloat(buffer, Player->yspeed);
    buffer = serializeFloat(buffer, Player->turn);
    return buffer;
}

#define GAMESTATESIZE 20
unsigned char * 
deSerializeInGame(unsigned char *buffer, player* Players)
{
    buffer = deSerializeFloat(buffer, &(Players->realx));
    buffer = deSerializeFloat(buffer, &(Players->realy));
    buffer = deSerializeFloat(buffer, &(Players->xspeed));
    buffer = deSerializeFloat(buffer, &(Players->yspeed));
    buffer = deSerializeFloat(buffer, &(Players->turn));
    return buffer;
}

// get the state from the other and set values accordingly
void
recState(struct sockaddr_in *address, int size, player* Player){
  unsigned char buffer[GAMESTATESIZE];
    if (recvfrom(skt, buffer, sizeof(buffer), 0, address, &size) < 0){
      perror("recorded wrongly");
      return;
    }
    deSerializeInGame(buffer, Player);
    return;
}

// get state from possible players and return all positions
void
sendState(struct sockaddr_in *address, int size, player* Player){
  unsigned char buffer[GAMESTATESIZE];
  serializeInGame(buffer, Player);
  if (sendto(skt, buffer, sizeof(buffer), 0, address, size) == -1){
      perror("send fail");
      return;
  }
}

void
wrongip(){
    puts("you need to supply the server address with -c ip.v4.adr.ess");
    exit(EXIT_FAILURE);
}

uint32_t
inet_addr(char *string)
{
    int a, b, c, d;
    char arr[4];
    int res = sscanf(string, "%d.%d.%d.%d", &a, &b, &c, &d);
    if (res != 4) wrongip();
    arr[0] = a;arr[1] = b; arr[2] = c; arr[3] = d;
    return *(uint32_t *)arr;
}
#endif // NET
