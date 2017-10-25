#include <winsock2.h>
#include <cstdio>
#pragma comment(lib, "Ws2_32.lib")


int main(int argc, char* argv[])
{
	WORD wVersionRequested;
	WSADATA wsaData;
	int err;

	wVersionRequested = MAKEWORD(1, 1);

	err = WSAStartup(wVersionRequested, &wsaData);
	if (err != 0) {
		return 0;
	}

	if (LOBYTE(wsaData.wVersion) != 1 ||
		HIBYTE(wsaData.wVersion) != 1) {
		WSACleanup();
		return 0;
	}
	SOCKET sockSrv = socket(AF_INET, SOCK_STREAM, 0);

	SOCKADDR_IN addrSrv;
	addrSrv.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
	addrSrv.sin_family = AF_INET;
	addrSrv.sin_port = htons(6000);

	bind(sockSrv, (SOCKADDR*)&addrSrv, sizeof(SOCKADDR));

	listen(sockSrv, 5);

	fd_set fdSocket;        // 所有可用套节字集合
	FD_ZERO(&fdSocket);
	FD_SET(sockSrv, &fdSocket);
	while (TRUE)
	{
		// 2）将fdSocket集合的一个拷贝fdRead传递给select函数，
		// 当有事件发生时，select函数移除fdRead集合中没有未决I/O操作的套节字句柄，然后返回。
		fd_set fdRead = fdSocket;
		int nRet = ::select(0, &fdRead, NULL, NULL, NULL);
		if (nRet > 0)
		{
			// 3）通过将原来fdSocket集合与select处理过的fdRead集合比较，
			// 确定都有哪些套节字有未决I/O，并进一步处理这些I/O。
			for (int i = 0; i<(int)fdSocket.fd_count; i++)
			{
				if (FD_ISSET(fdSocket.fd_array[i], &fdRead))
				{
					if (fdSocket.fd_array[i] == sockSrv)        // （1）监听套节字接收到新连接
					{
						if (fdSocket.fd_count < FD_SETSIZE)
						{
							sockaddr_in addrRemote;
							int nAddrLen = sizeof(addrRemote);
							SOCKET sNew = ::accept(sockSrv, (SOCKADDR*)&addrRemote, &nAddrLen);

							FD_SET(sNew, &fdSocket);
							printf("接收到连接（%s）\n", ::inet_ntoa(addrRemote.sin_addr));
						}
						else
						{
							printf(" Too much connections! \n");
							continue;
						}
					}
					else
					{
						char szText[256];
						int nRecv = ::recv(fdSocket.fd_array[i], szText, strlen(szText), 0);
						if (nRecv > 0)                        // （2）可读
						{
							szText[nRecv] = '\0';
							printf("接收到数据：%s \n", szText);
						}
						else                                // （3）连接关闭、重启或者中断
						{
							::closesocket(fdSocket.fd_array[i]);

							printf("关闭\n");
							FD_CLR(fdSocket.fd_array[i], &fdSocket);
						}
					}
				}
			}
		}
		else
		{
			printf(" Failed select() \n");
			break;
		}
	}
	return 0;
}