#ifdef NET
#ifndef NET_H
#define NET_H
        int ClientGameInit();
        void ServerGameInit();
        void sendState(struct sockaddr_in *address, int size, player* Player);
        void recState(struct sockaddr_in *address, int size, player* Player);
	void wrongip();
        uint32_t inet_addr(char *string);
#endif
#endif
