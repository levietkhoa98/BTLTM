#include <iostream>
#include "winsock2.h"
#include <stdio.h>
/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main() {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2), &wsa);
	
	SOCKET client = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	SOCKADDR_IN addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = inet_addr("127.0.0.1");
	addr.sin_port = htons(8080);
	
	int ret = connect(client, (SOCKADDR *)&addr, sizeof(addr));
	
	if(ret == 0) {
		printf("Ket noi thanh cong\n");
		
		char buff[256];
		while (1)
    	{
        printf("Nhap du lieu tu ban phim: ");
        gets(buff);
 
        send(client, buff, strlen(buff), 0);
 
        if (strncmp(buff, "exit", 4) == 0)
            break;
    	}
	}
	else {
		printf("Ket noi that bai ret = %d\n", ret);
        ret = WSAGetLastError();
        printf("Ma loi %d", ret);
	}
	closesocket(client);
    WSACleanup();
	return 0;
}
