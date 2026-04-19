#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 666
#define SERVER_ADDR "127.0.0.1"

int main() {
    std::cout << "Starting Client..." << std::endl;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKET client_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (client_socket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = inet_addr(SERVER_ADDR);

    if (connect(client_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        std::cerr << "Connection failed: " << WSAGetLastError() << std::endl;
        closesocket(client_socket);
        WSACleanup();
        return 1;
    }


    std::cout << "Connected to server!" << std::endl;

    const char* message = "Ping from Client!";
    send(client_socket, message, strlen(message), 0);
    std::cout << "Message sent." << std::endl;

    char buffer[1024] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0)
    {
        std::cout << "Received from server: " << buffer << std::endl;
    } else {
        std::cerr << "Receive failed or server disconnected." << std::endl;
    }

    closesocket(client_socket);
    WSACleanup();

    std::cout << "Client shutdown." << std::endl;
    return 0;
}