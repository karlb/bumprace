#ifdef NET
#ifndef NET_H
#define NET_H
        void ClientGameInit();
        void ServerGameInit();
        void sendState(struct sockaddr_in *address, int size, player* Player);
        void recState(struct sockaddr_in *address, int size, player* Player);
        uint32_t inet_addr(char *string);
#endif
#endif
