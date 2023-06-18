#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <thread>

#pragma comment (lib, "ws2_32.lib")

// �㭪�� ��⮪� ��� �ਥ�� ᮮ�饭�� �� �ࢥ�
void ReceiveMessages(SOCKET sock) {
    char buf[4096];
    while (true) {
        ZeroMemory(buf, 4096);

        // �ਥ� ᮮ�饭�� �� �ࢥ�
        int bytesReceived = recv(sock, buf, 4096, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "�訡�� �ਥ�� ᮮ�饭�� �� �ࢥ�. ��� �訡��: " << WSAGetLastError() << std::endl;
            break;
        }

        if (bytesReceived == 0) {
            // ��ࢥ� �⪫�稫��
            std::cout << "���������� � �ࢥ஬ ࠧ�ࢠ��." << std::endl;
            break;
        }

        // �뢮� ����祭���� ᮮ�饭��
        std::cout << std::string(buf, 0, bytesReceived) << std::endl;
    }
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
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "�訡�� ᮧ����� ᮪��. ��� �訡��: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // ���� IP-���� �ࢥ� � ����
    std::string ipAddress = "127.0.0.1";  // IP-���� �ࢥ�
    int port = 54000;                     // ���� �ࢥ�

    // ���������� �������� � ���ଠ樥� � �ࢥ�
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    // ������祭�� � �ࢥ��
    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) {
        std::cerr << "�訡�� ������祭�� � �ࢥ��. ��� �訡��: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // ���� ���� ���짮��⥫�
    std::string userNickname;
    std::cout << "������ ᢮� ���: ";
    std::getline(std::cin, userNickname);

    // ��ࠢ�� ���� �� �ࢥ�
    send(sock, userNickname.c_str(), userNickname.size() + 1, 0);

    // �������� ��⮪� ��� �ਥ�� ᮮ�饭�� �� �ࢥ�
    std::thread receiveThread(ReceiveMessages, sock);

    // �᭮���� ��⮪ ��� ��ࠢ�� ᮮ�饭��
    std::string userInput;
    while (true) {
        // ���� ᮮ�饭��
        std::getline(std::cin, userInput);

        // ��ࠢ�� ᮮ�饭�� �� �ࢥ�
        int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
        if (sendResult == SOCKET_ERROR) {
            std::cerr << "�訡�� ��ࠢ�� ᮮ�饭�� �� �ࢥ�. ��� �訡��: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    // �����⨥ ᮪��
    closesocket(sock);

    // �������� �����襭�� ��⮪� �ਥ�� ᮮ�饭��
    receiveThread.join();

    // ���⪠ Winsock
    WSACleanup();

    return 0;
}
