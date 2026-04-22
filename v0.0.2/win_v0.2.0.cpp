#include <winsock2.h>
#include <ws2tcpip.h>

#include <atomic>  
#include <cstring>
#include <iostream>
#include <limits>
#include <string>
#include <thread>

#pragma comment(lib, "ws2_32.lib")

#define LEN 1024

std::atomic<bool> running(true);

void listener(SOCKET sock)
{
    char buf[LEN] = {0};

    std::cout << "Listener started\n";

    while (running) {
        std::memset(buf, 0, sizeof(buf));

        sockaddr_in client_addr;
        int client_len = sizeof(client_addr);

        int read_n = recvfrom(
            sock,
            buf,
            LEN - 1,
            0,
            (sockaddr*)&client_addr,
            &client_len
        );

        if (read_n == SOCKET_ERROR) {
            if (running) {
                std::cerr << "recvfrom error\n";
            }
            break;
        }

        buf[read_n] = '\0';

        std::cout << "\nMessage: " << buf << std::endl;
        std::cout << "> ";
        std::cout.flush();
    }
}

void sender(SOCKET sock, const std::string& peer_ip, int peer_port)
{
    sockaddr_in peer_addr;
    std::memset(&peer_addr, 0, sizeof(peer_addr));
    peer_addr.sin_family = AF_INET;
    peer_addr.sin_port = htons(peer_port);

    if (inet_pton(AF_INET, peer_ip.c_str(), &peer_addr.sin_addr) <= 0) {
        std::cerr << "wrong peer ip\n";
        running = false;
        return;
    }

    char buf[LEN] = {0};

    std::cout << "Type messages. Use /exit to quit.\n";
    std::cout << "> ";
    std::cout.flush();

    while (running && std::cin.getline(buf, LEN)) {

        if (std::strcmp(buf, "/exit") == 0) {
            running = false;

            closesocket(sock);
            break;
        }

        int sent_n = sendto(
            sock,
            buf,
            std::strlen(buf),
            0,
            (sockaddr*)&peer_addr,
            sizeof(peer_addr)
        );

        if (sent_n == SOCKET_ERROR) {
            if (running) {
                std::cerr << "sendto error\n";
            }
            running = false;
            closesocket(sock);
            break;
        }

        std::cout << "> ";
        std::cout.flush();
    }

    if (running) {
        running = false;
        closesocket(sock);
    }
}

int main()
{
    WSADATA wsaData;

    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    int my_port;
    std::string peer_ip;
    int peer_port;

    std::cout << "My port: ";
    std::cin >> my_port;

    std::cout << "Peer IP: ";
    std::cin >> peer_ip;

    std::cout << "Peer port: ";
    std::cin >> peer_port;

    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

    SOCKET sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "socket error\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in my_addr;
    std::memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_addr.s_addr = INADDR_ANY;
    my_addr.sin_port = htons(my_port);

    if (bind(sock, (sockaddr*)&my_addr, sizeof(my_addr)) == SOCKET_ERROR) {
        std::cerr << "bind error\n";
        closesocket(sock);
        WSACleanup();
        return 1;
    }

    std::cout << "Chat started\n";
    std::cout << "Listening on port " << my_port << "\n";
    std::cout << "Sending to " << peer_ip << ":" << peer_port << "\n";

    std::thread recv_thread(listener, sock);
    std::thread send_thread(sender, sock, peer_ip, peer_port);

    send_thread.join();
    recv_thread.join();

    running = false;

    WSACleanup();
    return 0;
}