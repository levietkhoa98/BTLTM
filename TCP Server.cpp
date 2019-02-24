#include <iostream>
#include "winsock2.h"

/* run this program using the console pauser or add your own getch, system("pause") or input loop */

int main(int argc, char** argv) {
	WSADATA wsa;
	WSAStartup(MAKEWORD(2,2),&wsa);
	
	SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	
	SOCKADDR_IN addr;
    addr.sin_family = AF_INET;	
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(8080);
	
	bind(listener, (SOCKADDR *)&addr, sizeof(addr));
	listen(listener, 5);
	 
	SOCKET client = accept(listener, NULL, NULL);
	if (client) {
		char buf[256];
		FILE *fp1 = fopen("chao.txt","rb");
		fgets(buf, 256, (FILE *)fp1);
//		printf("%s\n", buf);
		send(client, buf, sizeof(buf), 0);	
		fclose(fp1);
	}
	char buff[256];
	int ret;
	
	while (1) {
		ret=recv(client,buff,sizeof(buff),0);
		if(ret<=0) break;
		buff[ret]=0;
		FILE *fp2 = fopen("client.txt","w+");
		fputs(buff,fp2);
		
	}
	closesocket(client);
	closesocket(listener);
	return 0;
}

