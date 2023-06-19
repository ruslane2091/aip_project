#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <thread>

#pragma comment (lib, "ws2_32.lib")

// Функция потока для приема сообщений от сервера
void ReceiveMessages(SOCKET sock) {
    char buf[4096];
    while (true) {
        ZeroMemory(buf, 4096);

        // Прием сообщения от сервера
        int bytesReceived = recv(sock, buf, 4096, 0);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "Ошибка приема сообщения от сервера. Код ошибки: " << WSAGetLastError() << std::endl;
            break;
        }

        if (bytesReceived == 0) {
            // Сервер отключился
            std::cout << "Соединение с сервером разорвано." << std::endl;
            break;
        }

        // Вывод полученного сообщения
        std::cout << std::string(buf, 0, bytesReceived) << std::endl;
    }
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
    SOCKET sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock == INVALID_SOCKET) {
        std::cerr << "Ошибка создания сокета. Код ошибки: " << WSAGetLastError() << std::endl;
        WSACleanup();
        return -1;
    }

    // Ввод IP-адреса сервера и порта
    std::string ipAddress = "127.0.0.1";  // IP-адрес сервера
    int port = 54000;                     // Порт сервера

    // Заполнение структуры с информацией о сервере
    sockaddr_in hint;
    hint.sin_family = AF_INET;
    hint.sin_port = htons(port);
    inet_pton(AF_INET, ipAddress.c_str(), &hint.sin_addr);

    // Подключение к серверу
    int connResult = connect(sock, (sockaddr*)&hint, sizeof(hint));
    if (connResult == SOCKET_ERROR) {
        std::cerr << "Ошибка подключения к серверу. Код ошибки: " << WSAGetLastError() << std::endl;
        closesocket(sock);
        WSACleanup();
        return -1;
    }

    // Ввод ника пользователя
    std::string userNickname;
    std::cout << "Введите свой ник: ";
    std::getline(std::cin, userNickname);

    // Отправка ника на сервер
    send(sock, userNickname.c_str(), userNickname.size() + 1, 0);

    // Создание потока для приема сообщений от сервера
    std::thread receiveThread(ReceiveMessages, sock);

    // Основной поток для отправки сообщений
    std::string userInput;
    while (true) {
        // Ввод сообщения
        std::getline(std::cin, userInput);

        // Отправка сообщения на сервер
        int sendResult = send(sock, userInput.c_str(), userInput.size() + 1, 0);
        if (sendResult == SOCKET_ERROR) {
            std::cerr << "Ошибка отправки сообщения на сервер. Код ошибки: " << WSAGetLastError() << std::endl;
            break;
        }
    }

    // Закрытие сокета
    closesocket(sock);

    // Ожидание завершения потока приема сообщений
    receiveThread.join();

    // Очистка Winsock
    WSACleanup();

    return 0;
}
