#include <iostream>
#include <vector>
#include <winsock2.h>
#include "protocol.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 666

int main() {
    std::cout << "Starting Server..." << std::endl;

    if (!init_network()) return 1;

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

    while (true)
    {
        PacketHeader header;
        if (!recv_all(client_socket, (char*)&header, sizeof(header)))
        {
            std::cout << "Client disconnected." << std::endl;
            break;
        }

        int command = ntohl(header.command);
        int payload_size = ntohl(header.payload_size);

        if (command == CONFIG)
        {
            std::cout << "\n[CONFIG] Payload size: " << payload_size << " bytes." << std::endl;
            ConfigPayload payload;
            if (recv_all(client_socket, (char*)&payload, payload_size)) {
                int matrix_size = ntohl(payload.matrix_size);
                int threads = ntohl(payload.threads_count);

                std::cout << "-> Matrix size: " << matrix_size << "x" << matrix_size << std::endl;
                std::cout << "-> Threads: " << threads << std::endl;

                PacketHeader response;
                response.command = htonl(ACK_CONFIG);
                response.payload_size = htonl(0);
                send_all(client_socket, (char*)&response, sizeof(response));
            }
        }
        else if (command == SEND_DATA)
        {
            std::cout << "\n[SEND_DATA] Expecting " << payload_size << " bytes of matrix data." << std::endl;

            int elements_count = payload_size / sizeof(int32_t);
            std::vector<int32_t> matrix(elements_count);

            if (recv_all(client_socket, (char*)matrix.data(), payload_size))
            {
                for (int32_t& val : matrix)
                {
                    val = ntohl(val);
                }
                std::cout << "-> Successfully received matrix of " << elements_count << " elements." << std::endl;

                if (elements_count > 0)
                {
                    std::cout << "-> First element: " << matrix.front() << ", Last element: " << matrix.back() << std::endl;
                }

                PacketHeader response;
                response.command = htonl(ACK_DATA);
                response.payload_size = htonl(0);
                send_all(client_socket, (char*)&response, sizeof(response));
            }
        }
        else
        {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }

    closesocket(client_socket);
    closesocket(server_socket);
    WSACleanup();

    std::cout << "Server shutdown." << std::endl;
    return 0;
}