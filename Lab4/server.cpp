#include <iostream>
#include <vector>
#include <winsock2.h>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include "protocol.h"
#include "Matrix.cpp"
#include "ThreadPool.cpp"

#pragma comment(lib, "ws2_32.lib")

#define PORT 666

const int NETWORK_QUEUES = 4;
const int NETWORK_THREADS_PER_QUEUE = 2;

const int COMPUTE_QUEUES = 2;
const int COMPUTE_THREADS_PER_QUEUE = 2;

const int TOTAL_COMPUTE_THREADS = COMPUTE_QUEUES * COMPUTE_THREADS_PER_QUEUE;

void handle_client(SOCKET client_socket, ThreadPool* computePool)
{
    Matrix server_matrix;

    std::cout << "[NETWORK WORKER] Handling new client in thread: " << std::this_thread::get_id() << std::endl;

    while (true)
    {
        PacketHeader header;
        if (!recv_all(client_socket, (char*)&header, sizeof(header)))
        {
            std::cout << "[NETWORK WORKER] Client disconnected." << std::endl;
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

                server_matrix.resize(size);

                std::cout << "\n[CONFIG] Size: " << size << "x" << size << std::endl;

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
            std::cout << "\n[START_TASK] Sending matrix to ComputePool (" << TOTAL_COMPUTE_THREADS << " tasks)..." << std::endl;

            std::atomic<int> completed_tasks{0};
            std::mutex wait_mtx;
            std::condition_variable wait_cv;

            for (int i = 0; i < TOTAL_COMPUTE_THREADS; i++)
            {
                Task compute_task([&server_matrix, i, &completed_tasks, &wait_mtx, &wait_cv]()
                                  {
                                      server_matrix.change_matrix(TOTAL_COMPUTE_THREADS, i);

                                      int current_completed = ++completed_tasks;

                                      if (current_completed == TOTAL_COMPUTE_THREADS)
                                      {
                                          std::lock_guard<std::mutex> lock(wait_mtx);
                                          wait_cv.notify_one();
                                      }
                                  }, i);

                computePool->add_task(compute_task);
            }

            std::unique_lock<std::mutex> lock(wait_mtx);
            wait_cv.wait(lock, [&]() { return completed_tasks == TOTAL_COMPUTE_THREADS; });

            std::cout << "Processing finished in ComputePool" << std::endl;

            PacketHeader response;
            response.command = htonl(ACK_START);
            response.payload_size = htonl(0);
            send_all(client_socket, (char*)&response, sizeof(response));
        }
        else if (command == GET_RESULT)
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
        else {
            std::cout << "Unknown command: " << command << std::endl;
        }
    }

    closesocket(client_socket);
}

int main() {
    std::cout << "Starting Multi-Pool Server..." << std::endl;

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

    while (true) {
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