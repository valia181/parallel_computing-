#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <thread>

#pragma comment(lib, "ws2_32.lib")
#define PORT 8080

void sendResponse(SOCKET clientSocket, const std::string& response)
{
    send(clientSocket, response.c_str(), response.size(), 0);
}

void handleRequest(SOCKET clientSocket)
{
    char buffer[2048];
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived > 0)
    {
        buffer[bytesReceived] = '\0';
        std::string request(buffer);

        size_t firstSpace = request.find(" ");
        size_t secondSpace = request.find(" ", firstSpace + 1);
        if (firstSpace == std::string::npos || secondSpace == std::string::npos) return;

        std::string path = request.substr(firstSpace + 1, secondSpace - firstSpace - 1);
        if (path == "/") path = "/index.html";

        std::string filename = path.substr(1);
        std::ifstream file(filename, std::ios::binary);
        std::string httpResponse;

        if (file)
        {
            std::stringstream content;
            content << file.rdbuf();
            std::string body = content.str();
            httpResponse = "HTTP/1.1 200 OK\r\nContent-Length: " + std::to_string(body.size()) +
                           "\r\nContent-Type: text/html\r\n\r\n" + body;
        } else
        {
            std::string body = "<h1>404 Not Found</h1>";
            httpResponse = "HTTP/1.1 404 Not Found\r\nContent-Length: " + std::to_string(body.size()) +
                           "\r\nContent-Type: text/html\r\n\r\n" + body;
        }

        sendResponse(clientSocket, httpResponse);
    }
    closesocket(clientSocket);
}

int main()
{
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);

    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed\n";
        WSACleanup();
        return 1;
    }

    struct sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, (struct sockaddr*)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Bind failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    if (listen(serverSocket, 10) == SOCKET_ERROR)
    {
        std::cerr << "Listen failed\n";
        closesocket(serverSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Server listening on port " << PORT << " ...\n";

    while (true)
    {
        struct sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (struct sockaddr*)&clientAddr, &clientAddrLen);

        if (clientSocket != INVALID_SOCKET)
        {
            char client_ip[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &(clientAddr.sin_addr), client_ip, INET_ADDRSTRLEN);
            std::thread(handleRequest, clientSocket).detach();
        }
    }

    closesocket(serverSocket);
    WSACleanup();
    return 0;
}