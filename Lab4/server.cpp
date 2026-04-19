#include <iostream>
#include <vector>
#include <winsock2.h>
#include "protocol.h"
#include "Matrix.cpp"
#include <thread>

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

    Matrix server_matrix;
    int active_threads = 0;

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
            ConfigPayload payload;
            if (recv_all(client_socket, (char*)&payload, payload_size))
            {
                int size = ntohl(payload.matrix_size);
                active_threads = ntohl(payload.threads_count);

                server_matrix.resize(size);
                std::cout << "\n[CONFIG] Size: " << size << "x" << size << ", Threads: " << active_threads << std::endl;

                PacketHeader response;
                response.command = htonl(ACK_CONFIG);
                response.payload_size = htonl(0);
                send_all(client_socket, (char*)&response, sizeof(response));
            }
        }
        else if (command == SEND_DATA)
        {
            std::cout << "\n[SEND_DATA] Receiving matrix..." << std::endl;

            if (recv_all(client_socket, server_matrix.get_raw_data(), payload_size))
            {
                server_matrix.to_host();
                std::cout << "Matrix successfully received" << std::endl;

                PacketHeader response;
                response.command = htonl(ACK_DATA);
                response.payload_size = htonl(0);
                send_all(client_socket, (char*)&response, sizeof(response));
            }
        }
        else if (command == START_TASK)
        {
            std::cout << "\n[START_TASK] Processing matrix with " << active_threads << " threads..." << std::endl;

            vector<std::thread> workers;
            for (int i = 0; i < active_threads; i++)
            {
                workers.emplace_back(&Matrix::change_matrix, &server_matrix, active_threads, i);
            }

            for (auto& t : workers)
            {
                t.join();
            }

            std::cout << "Processing finished" << std::endl;

            PacketHeader response;
            response.command = htonl(ACK_START);
            response.payload_size = htonl(0);
            send_all(client_socket, (char*)&response, sizeof(response));
        }
        else if (command == GET_RESULT) {
            std::cout << "\n[GET_RESULT] Client requested processed matrix." << std::endl;

            server_matrix.to_network();

            PacketHeader response;
            response.command = htonl(GET_RESULT);
            response.payload_size = htonl(server_matrix.get_byte_size());
            send_all(client_socket, (char*)&response, sizeof(response));

            send_all(client_socket, server_matrix.get_raw_data(), server_matrix.get_byte_size());

            server_matrix.to_host();
            std::cout << "Processed matrix sent to client." << std::endl;
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