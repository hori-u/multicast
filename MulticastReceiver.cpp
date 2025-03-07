#include <iostream>
#include <fstream>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <unistd.h>
#endif

#define MULTICAST_GROUP "239.255.0.1"
#define PORT 12345
#define BUFFER_SIZE 4096

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif

    int sock;
    sockaddr_in recvAddr{};
    sockaddr_in senderAddr{};
    char buffer[BUFFER_SIZE];

    // ソケット作成
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        return 1;
    }

    // 受信アドレス設定
    recvAddr.sin_family = AF_INET;
    recvAddr.sin_addr.s_addr = INADDR_ANY;
    recvAddr.sin_port = htons(PORT);

    // ソケットをバインド
    if (bind(sock, (struct sockaddr*)&recvAddr, sizeof(recvAddr)) < 0) {
        perror("bind failed");
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return 1;
    }

    // マルチキャストグループに参加
    struct ip_mreq multicastRequest{};
    multicastRequest.imr_multiaddr.s_addr = inet_addr(MULTICAST_GROUP);
    multicastRequest.imr_interface.s_addr = INADDR_ANY;

    if (setsockopt(sock, IPPROTO_IP, IP_ADD_MEMBERSHIP, 
                   (char*)&multicastRequest, sizeof(multicastRequest)) < 0) {
        perror("setsockopt failed");
#ifdef _WIN32
        closesocket(sock);
        WSACleanup();
#else
        close(sock);
#endif
        return 1;
    }

    std::cout << "Listening for multicast video segments on " 
              << MULTICAST_GROUP << ":" << PORT << std::endl;

    std::ofstream outputFile("received.ts", std::ios::binary | std::ios::app);
    
    // メッセージ受信ループ
    while (true) {
        socklen_t senderAddrLen = sizeof(senderAddr);
        memset(buffer, 0, BUFFER_SIZE);
        
        int recvLen = recvfrom(sock, buffer, BUFFER_SIZE, 0, 
                               (struct sockaddr*)&senderAddr, &senderAddrLen);
        if (recvLen < 0) {
            perror("recvfrom failed");
            break;
        }

        outputFile.write(buffer, recvLen);
    }

    outputFile.close();

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return 0;
}
