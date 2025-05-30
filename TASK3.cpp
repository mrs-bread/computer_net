//server.cpp
// project1-server.cpp
#include <iostream>
#include <sstream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
    setlocale(LC_ALL, "RUS");
    // 1. Инициализация Winsock (версия 2.2)
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0) {
        cerr << "WSAStartup failed: " << result << "\n";
        return 1;
    }

    // 2. Настраиваем структуру addrinfo для bind
    addrinfo* addr = nullptr;
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;        // IPv4
    hints.ai_socktype = SOCK_STREAM;    // TCP
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;     // для локального bind

    // Будем слушать 127.0.0.1:8000
    result = getaddrinfo("127.0.0.1", "8000", &hints, &addr);
    if (result != 0) {
        cerr << "getaddrinfo failed: " << result << "\n";
        WSACleanup();
        return 1;
    }

    // 3. Создаём слушающий сокет
    SOCKET listen_socket = socket(addr->ai_family, addr->ai_socktype, addr->ai_protocol);
    if (listen_socket == INVALID_SOCKET) {
        cerr << "socket failed: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        WSACleanup();
        return 1;
    }

    // 4. Привязываем сокет к адресу
    result = bind(listen_socket, addr->ai_addr, (int)addr->ai_addrlen);
    if (result == SOCKET_ERROR) {
        cerr << "bind failed: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    freeaddrinfo(addr);  // данные из addr более не нужны

    // 5. Переводим сокет в режим прослушивания
    if (listen(listen_socket, SOMAXCONN) == SOCKET_ERROR) {
        cerr << "listen failed: " << WSAGetLastError() << "\n";
        closesocket(listen_socket);
        WSACleanup();
        return 1;
    }

    cout << "HTTP-Server запущен на 127.0.0.1:8000. Ожидаем подключения...\n";

    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    // 6. Бесконечный цикл приёма и обработки клиентов
    while (true) {
        // 6.1. Принимаем новое соединение
        SOCKET client_socket = accept(listen_socket, nullptr, nullptr);
        if (client_socket == INVALID_SOCKET) {
            cerr << "accept failed: " << WSAGetLastError() << "\n";
            break;
        }

        // 6.2. Читаем из него HTTP-запрос (до BUFFER_SIZE байт)
        int bytesReceived = recv(client_socket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';  // нуль-терминируем
            // 6.3. Формируем тело ответа (HTML), куда вставляем полученные заголовки
            stringstream responseBody;
            responseBody << "<!DOCTYPE html>\n"
                << "<html><head><meta charset=\"utf-8\"><title>Project1 Server</title></head>\n"
                << "<body>\n"
                << "<h1>Простейший C++ HTTP-Сервер</h1>\n"
                << "<h2>Получённый запрос:</h2>\n"
                << "<pre>" << buffer << "</pre>\n"
                << "<hr><em>Server: C++ Winsock HTTP Project</em>\n"
                << "</body></html>\n";

            // 6.4. Собираем строку с заголовками и телом
            string bodyStr = responseBody.str();
            stringstream fullResponse;
            fullResponse << "HTTP/1.1 200 OK\r\n"
                << "Content-Type: text/html; charset=utf-8\r\n"
                << "Content-Length: " << bodyStr.length() << "\r\n"
                << "Connection: close\r\n"
                << "\r\n"
                << bodyStr;

            string responseStr = fullResponse.str();
            // 6.5. Отправляем ответ
            int sendResult = send(client_socket, responseStr.c_str(), (int)responseStr.length(), 0);
            if (sendResult == SOCKET_ERROR) {
                cerr << "send failed: " << WSAGetLastError() << "\n";
            }
        }
        // 6.6. Закрываем соединение с клиентом
        closesocket(client_socket);
    }

    // 7. Завершаем работу
    closesocket(listen_socket);
    WSACleanup();
    return 0;
}
//client.cpp
// project1-client_debug.cpp
#include <iostream>
#include <string>
#include <WinSock2.h>
#include <WS2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")

using namespace std;

int main() {
    setlocale(LC_ALL, "RUS");
    // 1. Инициализация Winsock
    WSADATA wsData;
    int wsResult = WSAStartup(MAKEWORD(2, 2), &wsData);
    if (wsResult != 0) {
        cerr << "WSAStartup failed: " << wsResult << "\n";
        cin.get();
        return 1;
    }
    cout << "[DEBUG] Winsock инициализирован.\n";

    // 2. Создаём TCP-сокет
    SOCKET clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET) {
        cerr << "[ERROR] socket failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        cin.get();
        return 1;
    }
    cout << "[DEBUG] Сокет создан.\n";

    // 3. Получаем адрес сервера (127.0.0.1:8000)
    addrinfo* addr = nullptr;
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;      // IPv4
    hints.ai_socktype = SOCK_STREAM;  // TCP
    hints.ai_protocol = IPPROTO_TCP;  // TCP IP
    //для первого пункта
    int result = getaddrinfo("127.0.0.1", "8000", &hints, &addr);
    //для второго пункта
    /*int result = getaddrinfo("www.json.org", "80", &hints, &addr);*/
    if (result != 0) {
        cerr << "[ERROR] getaddrinfo failed: " << result << "\n";
        closesocket(clientSocket);
        WSACleanup();
        cin.get();
        return 1;
    }
    cout << "[DEBUG] Адрес 127.0.0.1:8000 разрешён.\n";

    // 4. Пытаемся подключиться
    cout << "[DEBUG] Пытаемся подключиться к 127.0.0.1:8000...\n";
    if (connect(clientSocket, addr->ai_addr, (int)addr->ai_addrlen) == SOCKET_ERROR) {
        cerr << "[ERROR] connect failed: " << WSAGetLastError() << "\n";
        freeaddrinfo(addr);
        closesocket(clientSocket);
        WSACleanup();
        cin.get();
        return 1;
    }
    freeaddrinfo(addr);
    cout << "[DEBUG] Успешно подключились к серверу.\n";

    // 5. Формируем HTTP-запрос пункт 1
    /*string httpRequest =
        "GET / HTTP/1.1\r\n"
        "Host: localhost:8000\r\n"
        "Connection: close\r\n"
        "\r\n";*/
    //Формируем запрос для пункта 2:
     /*string httpRequest =
    "GET /json-ru.html HTTP/1.1\r\n"
        "Host: www.json.org\r\n"
        "Connection: close\r\n"
        "\r\n";*/

    // 6. Отправляем запрос
    cout << "[DEBUG] Отправляем запрос:\n" << httpRequest << "\n";
    int sendRes = send(clientSocket, httpRequest.c_str(), (int)httpRequest.length(), 0);
    if (sendRes == SOCKET_ERROR) {
        cerr << "[ERROR] send failed: " << WSAGetLastError() << "\n";
        closesocket(clientSocket);
        WSACleanup();
        cin.get();
        return 1;
    }
    cout << "[DEBUG] Запрос отправлен, байт отправлено: " << sendRes << "\n";

    // 7. Принимаем ответ и печатаем в консоль
    const int BUFFER_SIZE = 4096;
    char buffer[BUFFER_SIZE];
    int bytesRead = 0;
    cout << "[DEBUG] Ожидаем ответ от сервера...\n\n";
    do {
        bytesRead = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);
        if (bytesRead > 0) {
            buffer[bytesRead] = '\0';
            cout << buffer;
        }
        else if (bytesRead == SOCKET_ERROR) {
            cerr << "[ERROR] recv failed: " << WSAGetLastError() << "\n";
            break;
        }
    } while (bytesRead > 0);

    // 8. Закрываем сокет и выгружаем Winsock
    cout << "\n\n[DEBUG] Закрываем соединение.\n";
    closesocket(clientSocket);
    WSACleanup();
    cout << "[DEBUG] Winsock завершён.\n";

    // Делаем паузу, чтобы окно не закрылось сразу
    cout << "\nНажмите Enter, чтобы выйти...";
    cin.get();
    return 0;
}

//telnet: telnet 127.0.0.1 8000
//GET / HTTP / 1.1
//Host: localhost:8000
//
//
