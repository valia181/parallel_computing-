#include <iostream>
#include <winsock2.h>
#include <thread>
#include <atomic>
#include <mutex>
#include "protocol.h"
#include "Matrix.cpp"
#include "ThreadPool.cpp"

#pragma comment(lib, "ws2_32.lib")

#define PORT 666

const int NETWORK_QUEUES = 4;
const int NETWORK_THREADS_PER_QUEUE = 2;

const int COMPUTE_QUEUES = 2;
const int COMPUTE_THREADS_PER_QUEUE = 2;

bool handle_config_command(SOCKET client_socket, int payload_size, Matrix& server_matrix, int& requested_threads)
{
    ConfigPayload payload;
    if (recv_all(client_socket, (char*)&payload, payload_size))
    {
        int size = ntohl(payload.matrix_size);
        requested_threads = ntohl(payload.num_threads);
        int total_elements = size * size;
        requested_threads = std::min(requested_threads, total_elements);

        try
        {
            if (size <= 0 || size > 15000 || requested_threads <= 0 || requested_threads > 100)
            {
                throw std::length_error("Matrix size or thread is out of allowed bounds!");
            }
            server_matrix.resize(size);

            std::cout << "\n[CONFIG] Size: " << size << "x" << size << std::endl;

            PacketHeader response;
            response.command = htonl(ACK_CONFIG);
            response.payload_size = htonl(0);
            send_all(client_socket, (char*)&response, sizeof(response));
            return true;
        }
        catch (const std::exception& e)
        {
            std::cerr << "\n[ERROR] Failed to allocate memory for matrix: " << e.what() << std::endl;

            PacketHeader response;
            response.command = htonl(ERR_BAD_DATA);
            response.payload_size = htonl(0);
            send_all(client_socket, (char*)&response, sizeof(response));
            return false;
        }
    }
    return true;
}

void handle_send_data_command(SOCKET client_socket, int payload_size, Matrix& server_matrix, bool& has_data)
{
    std::cout << "\n[SEND_DATA] Receiving matrix..." << std::endl;

    if (payload_size <= 0)
    {
        std::cerr << "[ERROR] Client sent SEND_DATA with 0 or negative payload size." << std::endl;
        PacketHeader response;
        response.command = htonl(ERR_NO_DATA);
        response.payload_size = htonl(0);
        send_all(client_socket, (char*)&response, sizeof(response));
        return;
    }

    if (recv_all(client_socket, server_matrix.get_raw_data(), payload_size))
    {
        server_matrix.to_host();
        std::cout << "Matrix successfully received" << std::endl;
        has_data = true;

        PacketHeader response;
        response.command = htonl(ACK_DATA);
        response.payload_size = htonl(0);
        send_all(client_socket, (char*)&response, sizeof(response));
    }
}

void handle_start_task_command(SOCKET client_socket, ThreadPool* computePool,
                               Matrix& server_matrix, int requested_threads,
                               std::atomic<int>& completed_tasks, bool& is_processing, bool has_data)
{
    if (!has_data)
    {
        std::cerr << "[ERROR] Client requested START_TASK but no data was sent!" << std::endl;
        PacketHeader response;
        response.command = htonl(ERR_NO_DATA);
        response.payload_size = htonl(0);
        send_all(client_socket, (char*)&response, sizeof(response));
        return;
    }

    std::cout << "\n[START_TASK] Sending matrix to ComputePool (" << requested_threads << " tasks)..." << std::endl;

    completed_tasks = 0;
    is_processing = true;

    for (int i = 0; i < requested_threads; i++)
    {
        Task compute_task([&server_matrix, i, &completed_tasks, requested_threads]()
        {
            server_matrix.change_matrix(requested_threads, i);
            completed_tasks++;
        }, i);

        computePool->add_task(compute_task);
    }
    std::cout << "Tasks dispatched. Replying ACK_START to client." << std::endl;
    PacketHeader response;
    response.command = htonl(ACK_START);
    response.payload_size = htonl(0);
    send_all(client_socket, (char*)&response, sizeof(response));
}

void handle_get_status_command(SOCKET client_socket, std::atomic<int>& completed_tasks,
                               int requested_threads, bool& is_processing)
{
    PacketHeader response;
    response.payload_size = htonl(0);

    if (is_processing && completed_tasks == requested_threads)
    {
        response.command = htonl(STATUS_DONE);
        is_processing = false;
    } else if (is_processing)
    {
        response.command = htonl(STATUS_IN_PROGRESS);
    } else
    {
        response.command = htonl(ERR_NOT_READY);
    }

    send_all(client_socket, (char*)&response, sizeof(response));
}

void handle_get_result_command(SOCKET client_socket, Matrix& server_matrix)
{
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

void handle_client(SOCKET client_socket, ThreadPool* computePool)
{
    Matrix server_matrix;
    std::atomic<int> completed_tasks{0};
    bool is_processing = false;
    bool has_data = false;
    bool is_configured = false;
    int requested_threads = 1;

    std::cout << "[NETWORK WORKER] Handling new client in thread: " << std::this_thread::get_id() << std::endl;

    while (true)
    {
        PacketHeader header;
        if (!recv_all(client_socket, (char*)&header, sizeof(header)))
        {
            std::cout << "[NETWORK WORKER] Client disconnected." << std::endl;

            if (is_processing && completed_tasks < requested_threads)
            {
                std::cout << "Waiting for compute threads to finish before closing socket..." << std::endl;
                while (completed_tasks < requested_threads)
                {
                    std::this_thread::sleep_for(std::chrono::milliseconds(10));
                }
            }
            break;
        }

        int command = ntohl(header.command);
        int payload_size = ntohl(header.payload_size);

        bool disconnect = false;

        switch (command)
        {
            case CONFIG:
                if (handle_config_command(client_socket, payload_size, server_matrix, requested_threads))
                {
                    is_configured = true;
                }
                else
                {
                    disconnect = true;
                }
                break;

            case SEND_DATA:
                if (!is_configured)
                {
                    std::cerr << "[ERROR] Client attempted to send data before configuration!" << std::endl;
                    PacketHeader response;
                    response.command = htonl(ERR_NOT_READY);
                    response.payload_size = htonl(0);
                    send_all(client_socket, (char *) &response, sizeof(response));
                }
                else
                {
                    handle_send_data_command(client_socket, payload_size, server_matrix, has_data);
                }
                break;

            case START_TASK:
                handle_start_task_command(client_socket, computePool, server_matrix, requested_threads, completed_tasks, is_processing, has_data);
                break;

            case GET_STATUS:
                handle_get_status_command(client_socket, completed_tasks, requested_threads, is_processing);
                break;

            case GET_RESULT:
                handle_get_result_command(client_socket, server_matrix);
                break;

            default:
                std::cout << "Unknown command: " << command << std::endl;
                break;
        }

        if (disconnect)
        {
            break;
        }
    }

    closesocket(client_socket);
}

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

    ThreadPool networkPool;
    networkPool.initialize(NETWORK_QUEUES, NETWORK_THREADS_PER_QUEUE);

    ThreadPool computePool;
    computePool.initialize(COMPUTE_QUEUES, COMPUTE_THREADS_PER_QUEUE);

    std::cout << "Network and Compute Pools initialized." << std::endl;
    std::cout << "Waiting for connections on port " << PORT << "..." << std::endl;

    while (true)
    {
        struct sockaddr_in client_addr;
        int client_addr_size = sizeof(client_addr);
        SOCKET client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_size);

        if (client_socket == INVALID_SOCKET)
        {
            std::cerr << "Accept failed: " << WSAGetLastError() << std::endl;
            continue;
        }

        std::cout << "[MAIN THREAD] Client connected! Dispatching to NetworkPool..." << std::endl;

        Task client_task([client_socket, &computePool]()
        {
            handle_client(client_socket, &computePool);
        }, client_socket);

        networkPool.add_task(client_task);
    }

    networkPool.terminate();
    computePool.terminate();
    closesocket(server_socket);
    WSACleanup();

    std::cout << "Server shutdown." << std::endl;
    return 0;
}