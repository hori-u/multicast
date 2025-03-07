#include <iostream>
#include <fstream>
#include <cstring>
#include <vector>

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
#define BUFFER_SIZE 4096

std::vector<std::string> segments = { "segment0.ts", "segment1.ts", "segment2.ts", 
                                      "segment3.ts", "segment4.ts", "segment5.ts", 
                                      "segment6.ts", "segment7.ts" };

void sendFile(const std::string& filename, int sock, sockaddr_in& addr) {
    std::ifstream file(filename, std::ios::binary);
    if (!file) {
        std::cerr << "Failed to open: " << filename << std::endl;
        return;
    }

    char buffer[BUFFER_SIZE];

    while (!file.eof()) {
        file.read(buffer, BUFFER_SIZE);
        int bytesRead = file.gcount();

        if (sendto(sock, buffer, bytesRead, 0, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
            perror("sendto failed");
            break;
        }
    }

    file.close();
    std::cout << "Sent: " << filename << std::endl;
}

int main() {
#ifdef _WIN32
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed" << std::endl;
        return 1;
    }
#endif

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket creation failed");
        return 1;
    }

    sockaddr_in multicastAddr{};
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_addr.s_addr = inet_addr(MULTICAST_GROUP);
    multicastAddr.sin_port = htons(PORT);

    std::cout << "Starting multicast streaming on " << MULTICAST_GROUP << ":" << PORT << std::endl;

    while (true) {
        for (const auto& segment : segments) {
            sendFile(segment, sock, multicastAddr);

#ifdef _WIN32
            Sleep(6000);  // 6秒待機 (Windows)
#else
            sleep(6);  // 6秒待機 (Linux)
#endif
        }
    }

#ifdef _WIN32
    closesocket(sock);
    WSACleanup();
#else
    close(sock);
#endif

    return 0;
}
