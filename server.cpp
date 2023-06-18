#include <iostream>
#include <WS2tcpip.h>
#include <vector>
#include <string>
#include <thread>
#include <algorithm>

#pragma comment (lib, "ws2_32.lib")

std::vector<SOCKET*> clientSockets;
std::vector<std::string> clientNicknames;

void ClientHandler(SOCKET* clientSocket, std::string clientNickname) {
    // ��ࠢ�� �ਢ���⢨� �������
    std::string welcomeMessage = "���� ���������� � ��, " + clientNickname + "!";
    send(*clientSocket, welcomeMessage.c_str(), welcomeMessage.size() + 1, 0);

    char buf[4096];
    while (true) {
        ZeroMemory(buf, 4096);

        // �ਥ� ᮮ�饭�� �� ������
        int bytesReceived = recv(*clientSocket, buf, 4096, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "�訡�� �ਥ�� ᮮ�饭�� �� ������. ��� �訡��: " << WSAGetLastError() << std::endl;
            break;
        }

        if (bytesReceived == 0) {
            // ������ �⪫�稫��
            std::cout << clientNickname << " �⪫�稫��" << std::endl;

            // ��ࠢ�� 㢥�������� � �⪫�祭�� ������ �ᥬ ��⠫�� �����⠬
            std::string disconnectMessage = clientNickname + " ������ ��.";
            for (auto it = clientSockets.begin(); it != clientSockets.end(); ++it) {
                if (*it != clientSocket) {
                    send(**it, disconnectMessage.c_str(), disconnectMessage.size() + 1, 0);
                }
            }

            // �������� ᮪�� ������ � ���� �� ᮮ⢥������� ����஢
            auto it = std::find(clientSockets.begin(), clientSockets.end(), clientSocket);
            if (it != clientSockets.end()) {
                clientSockets.erase(it);
                clientNicknames.erase(clientNicknames.begin() + (it - clientSockets.begin()));
            }

            break;
        }

        // �뢮� ����祭���� ᮮ�饭�� ���짮��⥫�
        std::cout << "����祭� ᮮ�饭�� �� " << clientNickname << ": " << std::string(buf, 0, bytesReceived) << std::endl;

        // ��ࠢ�� ᮮ�饭�� �ᥬ ��⠫�� �����⠬
        std::string messageToSend = clientNickname + ": " + std::string(buf, 0, bytesReceived);
        for (auto it = clientSockets.begin(); it != clientSockets.end(); ++it) {
            if (*it != clientSocket) {
                send(**it, messageToSend.c_str(), messageToSend.size() + 1, 0);
            }
        }
    }

    // �����⨥ ᮪�� ������
    closesocket(*clientSocket);
    delete clientSocket;
}

int main() {
    // ���樠������ Winsock
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);

    int wsResult = WSAStartup(ver, &wsData);
    if (wsResult != 0) {
        std::cerr << "�訡�� ���樠����樨 Winsock. ��� �訡��: " << wsResult << std::endl;
        return -1;
    }

    // �������� ᮪��
    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET) {
        std::cerr << "�訡�� ᮧ����� ᮪��. ��� �訡��: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // �ਢ離� ᮪�� � IP-����� � �����
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    // �������� ᮥ������� �����⮢
    listen(listening, SOMAXCONN);

    std::cout << "��ࢥ� ����饭. �������� ������祭�� �����⮢..." << std::endl;

    // ���� �������� ᮥ������� �����⮢
    while (true) {
        sockaddr_in client;
        int clientSize = sizeof(client);

        // �ਭ�⨥ ������ ᮥ�������
        SOCKET* clientSocket = new SOCKET(accept(listening, (sockaddr*)&client, &clientSize));

        // ����祭�� ���� ������
        char nicknameBuf[4096];
        ZeroMemory(nicknameBuf, 4096);
        int nicknameBytesReceived = recv(*clientSocket, nicknameBuf, 4096, 0);
        std::string clientNickname = std::string(nicknameBuf, 0, nicknameBytesReceived);

        // ���������� ᮪�� ������ � ���� � ᮮ⢥�����騥 ������
        clientSockets.push_back(clientSocket);
        clientNicknames.push_back(clientNickname);

        // �������� ������ ��⮪� ��� ��ࠡ�⪨ ������
        std::thread clientThread(ClientHandler, clientSocket, clientNickname);
        clientThread.detach();

        // �뢮� ���ଠ樨 � ����� ������
        std::cout << "������稫�� ���� ������: " << clientNickname << std::endl;
    }

    // �����⨥ ᮪�� ����訢����
    closesocket(listening);

    // ���⪠ Winsock
    WSACleanup();

    return 0;
}
