#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <cstring>
#pragma comment (lib, "Ws2_32.lib")
using namespace std;

#define SRV_PORT 1234
#define SRV_HOST "127.0.0.1"
const int NAME_SIZE = 64;

struct PatientData {
    char surname[NAME_SIZE];
    int h;
    int w;
};

int main() {
    setlocale(LC_ALL, "RUS");
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Ошибка WSAStartup: " << WSAGetLastError() << "\n";
        return -1;
    }

    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);

    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_port = htons(SRV_PORT);
    serverAddr.sin_addr.s_addr = inet_addr(SRV_HOST);

    cout << "Готов к отправке данных серверу.\n";

    while (true) {
        PatientData patient;
        cout << "Введите фамилию (или 'Bye' для завершения): ";
        cin.getline(patient.surname, NAME_SIZE);

        if (strcmp(patient.surname, "Bye") == 0) {
            sendto(s, (char*)&patient, sizeof(PatientData), 0,
                (sockaddr*)&serverAddr, sizeof(serverAddr));
            break;
        }

        cout << "Введите рост (см): ";
        cin >> patient.h;
        cout << "Введите вес (кг): ";
        cin >> patient.w;
        cin.ignore();

        sendto(s, (char*)&patient, sizeof(PatientData), 0,
            (sockaddr*)&serverAddr, sizeof(serverAddr));

        char buffer[256] = { 0 };
        int serverAddrSize = sizeof(serverAddr);
        int bytesReceived = recvfrom(s, buffer, sizeof(buffer) - 1, 0,
            (sockaddr*)&serverAddr, &serverAddrSize);
        if (bytesReceived > 0) {
            buffer[bytesReceived] = '\0';
            cout << "Ответ сервера: " << buffer << "\n";
        }
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
