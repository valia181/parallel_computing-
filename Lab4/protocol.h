#include <iostream>
#include <vector>
#include <winsock2.h>
#include <stdint.h>

#pragma comment(lib, "ws2_32.lib")

enum CommandType : int32_t {
    CONFIG = 1,
    SEND_DATA = 2,
    START_TASK = 3,
    GET_STATUS = 4,
    GET_RESULT = 5
};


enum ResponseType : int32_t {
    ACK_CONFIG = 10,
    ACK_DATA = 11,
    ACK_START = 12,
    STATUS_IN_PROGRESS = 13,
    STATUS_DONE = 14,
    ERR_BAD_DATA = 90,
    ERR_NO_DATA = 91,
    ERR_NOT_READY = 92
};

struct PacketHeader {
    int32_t command;
    int32_t payload_size;
};

struct ConfigPayload {
    int32_t matrix_size;
    int32_t num_threads;
};

bool init_network()
{
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed: " << WSAGetLastError() << std::endl;
        return false;
    }
    return true;
}

bool send_all(SOCKET s, const char* buffer, int length)
{
    int total_sent = 0;
    while (total_sent < length)
    {
        int bytes_sent = send(s, buffer + total_sent, length - total_sent, 0);
        if (bytes_sent <= 0) return false;
        total_sent += bytes_sent;
    }
    return true;
}

bool recv_all(SOCKET s, char* buffer, int length)
{
    int total_read = 0;
    while (total_read < length)
    {
        int bytes_read = recv(s, buffer + total_read, length - total_read, 0);
        if (bytes_read <= 0) return false;
        total_read += bytes_read;
    }
    return true;
}