#include <iostream>
#include <vector>
#include <winsock2.h>
#include "protocol.h"
#include "Matrix.cpp"

#pragma comment(lib, "ws2_32.lib")

#define PORT 666
#define SERVER_ADDR "127.0.0.1"

bool send_config_to_server(SOCKET s, int matrix_size, int num_threads)
{
    PacketHeader header;
    header.command = htonl(CONFIG);
    header.payload_size = htonl(sizeof(ConfigPayload));
    send_all(s, (char*)&header, sizeof(header));

    ConfigPayload payload;
    payload.matrix_size = htonl(matrix_size);
    payload.num_threads = htonl(num_threads);
    send_all(s, (char*)&payload, sizeof(payload));

    std::cout << "Configuration sent. Waiting for confirmation..." << std::endl;

    PacketHeader response;
    if (recv_all(s, (char*)&response, sizeof(response)))
    {
        int response_cmd = ntohl(response.command);
        if (response_cmd == ACK_CONFIG)
        {
            std::cout << "Server confirmed configuration (ACK_CONFIG)!" << std::endl;
            return true;
        } else if (response_cmd == ERR_BAD_DATA)
        {
            std::cerr << "Error: Server rejected configuration (ERR_BAD_DATA). Matrix might be too large." << std::endl;
        } else {
            std::cerr << "Error: Unexpected response from server: " << response_cmd << std::endl;
        }
    } else {
        std::cerr << "Receive failed or server disconnected." << std::endl;
    }
    return false;
}

bool send_matrix_to_server(SOCKET s, Matrix& client_matrix)
{
    client_matrix.to_network();

    PacketHeader data_header;
    data_header.command = htonl(SEND_DATA);
    data_header.payload_size = htonl(client_matrix.get_byte_size());

    send_all(s, (char*)&data_header, sizeof(data_header));
    send_all(s, client_matrix.get_raw_data(), client_matrix.get_byte_size());

    PacketHeader data_response;
    if(recv_all(s, (char*)&data_response, sizeof(data_response)))
    {
        int data_cmd = ntohl(data_response.command);
        if (data_cmd == ACK_DATA)
        {
            std::cout << "Server confirmed data reception" << std::endl;
            return true;
        } else if (data_cmd == ERR_NO_DATA)
        {
            std::cerr << "Error: Server reported missing or empty data (ERR_NO_DATA)." << std::endl;
        }
    }
    return false;
}

bool start_server_processing(SOCKET s)
{
    PacketHeader start_hdr;
    start_hdr.command = htonl(START_TASK);
    start_hdr.payload_size = htonl(0);
    send_all(s, (char *) &start_hdr, sizeof(start_hdr));

    PacketHeader start_response;
    if (recv_all(s, (char *) &start_response, sizeof(start_response)))
    {
        if (ntohl(start_response.command) == ACK_START)
        {
            std::cout << "Server started processing" << std::endl;
            return true;
        }
    }
    return false;
}

bool wait_for_server_status(SOCKET s)
{
    bool is_done = false;
    while (!is_done)
    {
        PacketHeader status_req;
        status_req.command = htonl(GET_STATUS);
        status_req.payload_size = htonl(0);
        send_all(s, (char *) &status_req, sizeof(status_req));

        PacketHeader status_res;
        if (recv_all(s, (char *) &status_res, sizeof(status_res)))
        {
            int status = ntohl(status_res.command);

            if (status == STATUS_IN_PROGRESS)
            {
                std::cout << "Status: In progress... waiting." << std::endl;
                Sleep(50);
            } else if (status == STATUS_DONE)
            {
                std::cout << "Status: DONE!" << std::endl;
                is_done = true;
            } else
            {
                std::cerr << "Unexpected status received: " << status << std::endl;
                return false;
            }
        } else
        {
            std::cerr << "Connection lost during status check." << std::endl;
            return false;
        }
    }
    return true;
}

bool receive_final_result(SOCKET s, Matrix& client_matrix)
{
    std::cout << "\nRequesting final result from server..." << std::endl;
    PacketHeader get_res_hdr;
    get_res_hdr.command = htonl(GET_RESULT);
    get_res_hdr.payload_size = htonl(0);
    send_all(s, (char *) &get_res_hdr, sizeof(get_res_hdr));

    PacketHeader res_header;
    if (recv_all(s, (char *) &res_header, sizeof(res_header)))
    {
        int res_command = ntohl(res_header.command);
        int res_payload_size = ntohl(res_header.payload_size);

        if (res_command == GET_RESULT && res_payload_size > 0)
        {
            if (recv_all(s, client_matrix.get_raw_data(), res_payload_size))
            {
                client_matrix.to_host();
                std::cout << "Result received! Processed Matrix:" << std::endl;
                client_matrix.print_matrix();
                return true;
            }
        }
    }
    return false;
}

int main() {
    srand(static_cast<unsigned int>(time(0)) ^ GetCurrentProcessId());
    int MATRIX_SIZE = (rand() % 8) + 2;
    int NUM_THREADS = 4;

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

    if (send_config_to_server(client_socket, MATRIX_SIZE, NUM_THREADS))
    {
        Matrix client_matrix(MATRIX_SIZE);
        std::cout << "Original Matrix:" << std::endl;
        client_matrix.print_matrix();

        if (send_matrix_to_server(client_socket, client_matrix))
        {
            if (start_server_processing(client_socket))
            {
                if (wait_for_server_status(client_socket))
                {
                    receive_final_result(client_socket, client_matrix);
                }
            }
        }
    }

    closesocket(client_socket);
    WSACleanup();

    std::cout << "Client shutdown." << std::endl;
    return 0;
}