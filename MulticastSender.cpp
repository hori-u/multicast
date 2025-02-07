#include <iostream>
#include <cstring>

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define MULTICAST_GROUP "239.255.0.1"
#define PORT 12345

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif

    int sock;
    sockaddr_in multicastAddr;
    char message[] = "Hello, Multicast!";

    // ソケット作成
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        return 1;
    }

    // マルチキャストアドレスの設定
    memset(&multicastAddr, 0, sizeof(multicastAddr));
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    multicastAddr.sin_port = htons(PORT);

    // データをマルチキャストグループに送信
    for (int i = 0; i < 5; i++) {
        if (sendto(sock, message, strlen(message), 0, 
                   (struct sockaddr*)&multicastAddr, sizeof(multicastAddr)) < 0) {
            perror("sendto failed");
            break;
        }
        std::cout << "Sent: " << message << std::endl;
#ifdef _WIN32
        Sleep(2000); // Windows の場合
#else
        sleep(2); // Linux の場合
#endif
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return 0;
}
