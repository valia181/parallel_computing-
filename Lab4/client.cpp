#include <iostream>
#include <vector>
#include <winsock2.h>
#include "protocol.h"

#pragma comment(lib, "ws2_32.lib")

#define PORT 666
#define SERVER_ADDR "127.0.0.1"

int main() {
    std::cout << "Starting Client..." << std::endl;

    if (!init_network()) return 1;

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

    PacketHeader header;
    header.command = htonl(CONFIG);
    header.payload_size = htonl(sizeof(ConfigPayload));
    send_all(client_socket, (char*)&header, sizeof(header));

    ConfigPayload payload;
    payload.matrix_size = htonl(5000);
    payload.threads_count = htonl(8);
    send_all(client_socket, (char*)&payload, sizeof(payload));

    std::cout << "Configuration sent. Waiting for confirmation..." << std::endl;

    PacketHeader response;
    if (recv_all(client_socket, (char*)&response, sizeof(response)))
    {
        if (ntohl(response.command) == ACK_CONFIG)
        {
            std::cout << "Server confirmed configuration (ACK_CONFIG)!" << std::endl;

            int matrix_size = 10;
            int elements_count = matrix_size * matrix_size;
            std::vector<int32_t> matrix(elements_count);

            for (int i = 0; i < elements_count; ++i)
            {
                matrix[i] = htonl(i + 1);
            }

            PacketHeader data_header;
            data_header.command = htonl(SEND_DATA);
            data_header.payload_size = htonl(elements_count * sizeof(int32_t));
            send_all(client_socket, (char*)&data_header, sizeof(data_header));

            std::cout << "\nSending matrix data (" << elements_count * sizeof(int32_t) << " bytes)..." << std::endl;
            send_all(client_socket, (char*)matrix.data(), elements_count * sizeof(int32_t));

            PacketHeader data_response;
            if (recv_all(client_socket, (char*)&data_response, sizeof(data_response)))
            {
                if (ntohl(data_response.command) == ACK_DATA)
                {
                    std::cout << "Server confirmed data reception (ACK_DATA)!" << std::endl;
                }
            }
        }
    } else
    {
        std::cerr << "Receive failed or server disconnected." << std::endl;
    }

    closesocket(client_socket);
    WSACleanup();

    std::cout << "Client shutdown." << std::endl;
    return 0;
}