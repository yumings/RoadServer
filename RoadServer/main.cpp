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

	fd_set fdSocket;        // ���п����׽��ּ���
	FD_ZERO(&fdSocket);
	FD_SET(sockSrv, &fdSocket);
	while (TRUE)
	{
		// 2����fdSocket���ϵ�һ������fdRead���ݸ�select������
		// �����¼�����ʱ��select�����Ƴ�fdRead������û��δ��I/O�������׽��־����Ȼ�󷵻ء�
		fd_set fdRead = fdSocket;
		int nRet = ::select(0, &fdRead, NULL, NULL, NULL);
		if (nRet > 0)
		{
			// 3��ͨ����ԭ��fdSocket������select�������fdRead���ϱȽϣ�
			// ȷ��������Щ�׽�����δ��I/O������һ��������ЩI/O��
			for (int i = 0; i<(int)fdSocket.fd_count; i++)
			{
				if (FD_ISSET(fdSocket.fd_array[i], &fdRead))
				{
					if (fdSocket.fd_array[i] == sockSrv)        // ��1�������׽��ֽ��յ�������
					{
						if (fdSocket.fd_count < FD_SETSIZE)
						{
							sockaddr_in addrRemote;
							int nAddrLen = sizeof(addrRemote);
							SOCKET sNew = ::accept(sockSrv, (SOCKADDR*)&addrRemote, &nAddrLen);

							FD_SET(sNew, &fdSocket);
							printf("���յ����ӣ�%s��\n", ::inet_ntoa(addrRemote.sin_addr));
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
						if (nRecv > 0)                        // ��2���ɶ�
						{
							szText[nRecv] = '\0';
							printf("���յ����ݣ�%s \n", szText);
						}
						else                                // ��3�����ӹرա����������ж�
						{
							::closesocket(fdSocket.fd_array[i]);

							printf("�ر�\n");
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