#include <iostream>
#include <vector>
#include <winsock2.h>
#include "protocol.h"
#include "Matrix.cpp"

#pragma comment(lib, "ws2_32.lib")

#define PORT 666
#define SERVER_ADDR "127.0.0.1"

int main() {
    srand(static_cast<unsigned int>(time(0)) ^ GetCurrentProcessId());
    int MATRIX_SIZE = (rand() % 8) + 2;

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
    payload.matrix_size = htonl(MATRIX_SIZE);
    send_all(client_socket, (char*)&payload, sizeof(payload));

    std::cout << "Configuration sent. Waiting for confirmation..." << std::endl;

    PacketHeader response;
    if (recv_all(client_socket, (char*)&response, sizeof(response)))
    {
        if (ntohl(response.command) == ACK_CONFIG)
        {
            std::cout << "Server confirmed configuration (ACK_CONFIG)!" << std::endl;

            Matrix client_matrix(MATRIX_SIZE);
            std::cout << "Original Matrix:" << std::endl;
            client_matrix.print_matrix();

            client_matrix.to_network();

            PacketHeader data_header;
            data_header.command = htonl(SEND_DATA);
            data_header.payload_size = htonl(client_matrix.get_byte_size());

            send_all(client_socket, (char*)&data_header, sizeof(data_header));
            send_all(client_socket, client_matrix.get_raw_data(), client_matrix.get_byte_size());

            PacketHeader data_response;
            recv_all(client_socket, (char*)&data_response, sizeof(data_response));

            if (ntohl(data_response.command) == ACK_DATA)
            {
                std::cout << "Server confirmed data reception" << std::endl;

                PacketHeader start_hdr;
                start_hdr.command = htonl(START_TASK);
                start_hdr.payload_size = htonl(0);
                send_all(client_socket, (char*)&start_hdr, sizeof(start_hdr));

                PacketHeader start_response;
                recv_all(client_socket, (char*)&start_response, sizeof(start_response));
                if (ntohl(start_response.command) == ACK_START) {
                    std::cout << "Server started processing" << std::endl;
                }

                std::cout << "\nRequesting result from server..." << std::endl;
                PacketHeader get_res_hdr;
                get_res_hdr.command = htonl(GET_RESULT);
                get_res_hdr.payload_size = htonl(0);
                send_all(client_socket, (char*)&get_res_hdr, sizeof(get_res_hdr));

                PacketHeader res_header;
                if (recv_all(client_socket, (char*)&res_header, sizeof(res_header)))
                {
                    int res_command = ntohl(res_header.command);
                    int res_payload_size = ntohl(res_header.payload_size);

                    if (res_command == GET_RESULT && res_payload_size > 0)
                    {
                        if (recv_all(client_socket, client_matrix.get_raw_data(), res_payload_size)) {
                            client_matrix.to_host();

                            std::cout << "Result received! Processed Matrix:" << std::endl;
                            client_matrix.print_matrix();
                        }
                    }
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