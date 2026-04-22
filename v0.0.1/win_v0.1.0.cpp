#include <winsock2.h>
#include <ws2tcpip.h>

#include <cstring>
#include <iostream>
#include <limits>
#include <string>

#pragma comment(lib, "ws2_32.lib")

#define LEN 1024

void listener(int port)
{
    char buf[LEN] = {0};

    SOCKET server_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (server_sock == INVALID_SOCKET) {
        std::cerr << "socket error\n";
        return;
    }

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(port);

    if (bind(server_sock, (sockaddr*)&address, sizeof(address)) == SOCKET_ERROR) {
        std::cerr << "bind error\n";
        closesocket(server_sock);
        return;
    }

    std::cout << "Listening on port " << port << "...\n";

    while (true) {
        std::memset(buf, 0, sizeof(buf));

        sockaddr_in client_addr;
        int client_len = sizeof(client_addr);

        int read_n =
            recvfrom(server_sock, buf, LEN - 1, 0, (sockaddr*)&client_addr, &client_len);

        if (read_n == SOCKET_ERROR) {
            std::cerr << "recvfrom error\n";
            break;
        }

        buf[read_n] = '\0';
        std::cout << "Message: " << buf << std::endl;
    }

    closesocket(server_sock);
}

void sender(const std::string& ip, int port)
{
    SOCKET client_sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (client_sock == INVALID_SOCKET) {
        std::cerr << "socket error\n";
        return;
    }

    sockaddr_in address;
    std::memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &address.sin_addr) <= 0) {
        std::cerr << "wrong ip\n";
        closesocket(client_sock);
        return;
    }

    char buf[LEN] = {0};

    std::cout << "Type messages. Use /exit to quit.\n";

    while (std::cin.getline(buf, LEN)) {
        if (std::strcmp(buf, "/exit") == 0) {
            break;
        }

        int sent_n = sendto(
            client_sock,
            buf,
            std::strlen(buf),
            0,
            (sockaddr*)&address,
            sizeof(address)
        );

        if (sent_n == SOCKET_ERROR) {
            std::cerr << "sendto error\n";
            break;
        }
    }

    closesocket(client_sock);
}

int main()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    std::string ip;
    int port;
    int mode;

    std::cout << "IP: ";
    std::cin >> ip;

    std::cout << "Port: ";
    std::cin >> port;

    std::cout << "Select mode: sender[1] or listener[2]: ";
    std::cin >> mode;

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    if (mode == 1) {
        sender(ip, port);
    } else if (mode == 2) {
        listener(port);
    } else {
        std::cout << "wrong mode\n";
    }

    WSACleanup();
    return 0;
}