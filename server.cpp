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
    // Отправка приветствия клиенту
    std::string welcomeMessage = "Добро пожаловать в чат, " + clientNickname + "!";
    send(*clientSocket, welcomeMessage.c_str(), welcomeMessage.size() + 1, 0);

    char buf[4096];
    while (true) {
        ZeroMemory(buf, 4096);

        // Прием сообщения от клиента
        int bytesReceived = recv(*clientSocket, buf, 4096, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Ошибка приема сообщения от клиента. Код ошибки: " << WSAGetLastError() << std::endl;
            break;
        }

        if (bytesReceived == 0) {
            // Клиент отключился
            std::cout << clientNickname << " отключился" << std::endl;

            // Отправка уведомления о отключении клиента всем остальным клиентам
            std::string disconnectMessage = clientNickname + " покинул чат.";
            for (auto it = clientSockets.begin(); it != clientSockets.end(); ++it) {
                if (*it != clientSocket) {
                    send(**it, disconnectMessage.c_str(), disconnectMessage.size() + 1, 0);
                }
            }

            // Удаление сокета клиента и ника из соответствующих векторов
            auto it = std::find(clientSockets.begin(), clientSockets.end(), clientSocket);
            if (it != clientSockets.end()) {
                clientSockets.erase(it);
                clientNicknames.erase(clientNicknames.begin() + (it - clientSockets.begin()));
            }

            break;
        }

        // Вывод полученного сообщения пользователя
        std::cout << "Получено сообщение от " << clientNickname << ": " << std::string(buf, 0, bytesReceived) << std::endl;

        // Отправка сообщения всем остальным клиентам
        std::string messageToSend = clientNickname + ": " + std::string(buf, 0, bytesReceived);
        for (auto it = clientSockets.begin(); it != clientSockets.end(); ++it) {
            if (*it != clientSocket) {
                send(**it, messageToSend.c_str(), messageToSend.size() + 1, 0);
            }
        }
    }

    // Закрытие сокета клиента
    closesocket(*clientSocket);
    delete clientSocket;
}

int main() {
    // Инициализация Winsock
    WSADATA wsData;
    WORD ver = MAKEWORD(2, 2);

    int wsResult = WSAStartup(ver, &wsData);
    if (wsResult != 0) {
        std::cerr << "Ошибка инициализации Winsock. Код ошибки: " << wsResult << std::endl;
        return -1;
    }

    // Создание сокета
    SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
    if (listening == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета. Код ошибки: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // Привязка сокета к IP-адресу и порту
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(54000);
    hint.sin_addr.S_un.S_addr = INADDR_ANY;

    bind(listening, (sockaddr*)&hint, sizeof(hint));

    // Ожидание соединений клиентов
    listen(listening, SOMAXCONN);

    std::cout << "Сервер запущен. Ожидание подключений клиентов..." << std::endl;

    // Цикл ожидания соединений клиентов
    while (true) {
        sockaddr_in client;
        int clientSize = sizeof(client);

        // Принятие нового соединения
        SOCKET* clientSocket = new SOCKET(accept(listening, (sockaddr*)&client, &clientSize));

        // Получение ника клиента
        char nicknameBuf[4096];
        ZeroMemory(nicknameBuf, 4096);
        int nicknameBytesReceived = recv(*clientSocket, nicknameBuf, 4096, 0);
        std::string clientNickname = std::string(nicknameBuf, 0, nicknameBytesReceived);

        // Добавление сокета клиента и ника в соответствующие векторы
        clientSockets.push_back(clientSocket);
        clientNicknames.push_back(clientNickname);

        // Создание нового потока для обработки клиента
        std::thread clientThread(ClientHandler, clientSocket, clientNickname);
        clientThread.detach();

        // Вывод информации о новом клиенте
        std::cout << "Подключился новый клиент: " << clientNickname << std::endl;
    }

    // Закрытие сокета прослушивания
    closesocket(listening);

    // Очистка Winsock
    WSACleanup();

    return 0;
}
