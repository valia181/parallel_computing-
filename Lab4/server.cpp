#include <iostream>
#include <winsock2.h>

#pragma comment(lib, "ws2_32.lib")

#define PORT 666

int main() {
    std::cout << "Starting Server..." << std::endl;

    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed with error: " << WSAGetLastError() << std::endl;
        return 1;
    }

    SOCKET server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return 1;
    }


    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    server_addr.sin_addr.s_addr = INADDR_ANY;

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    if (listen(server_socket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Waiting for connection on port " << PORT << "..." << std::endl;

    struct sockaddr_in client_addr;
    int client_addr_size = sizeof(client_addr);
    SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);

    if (client_socket == INVALID_SOCKET)
    {
        std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
        closesocket(server_socket);
        WSACleanup();
        return 1;
    }

    std::cout << "Client connected!" << std::endl;

    char buffer[1024] = {0};
    int bytes_received = recv(client_socket, buffer, sizeof(buffer), 0);
    if (bytes_received > 0) {
        std::cout << "Received from client: " << buffer << std::endl;

        const char* response = "Pong from Server!";
        send(client_socket, response, strlen(response), 0);
        std::cout << "Response sent to client." << std::endl;
    } else
    {
        std::cerr << "Receive failed or client disconnected." << std::endl;
    }

    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    std::cout << "Server shutdown." << std::endl;
    return 0;
}