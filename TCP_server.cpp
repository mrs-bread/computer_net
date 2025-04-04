#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <cstring>
#pragma comment (lib, "Ws2_32.lib")
using namespace std;
#define SRV_PORT 1234
const int NAME_SIZE = 64;

struct PatientData {
    char surname[NAME_SIZE];
    int h;
    int w;
};
string IMT(int h, int w) {
    double hM = h / 100.0;
    double imt = w / (hM * hM);
    if (imt < 18.5)
        return "Нехватка";
    else if (imt < 25.0)
        return "Норма";
    else
        return "Избыток";
}
int main() {
    setlocale(LC_ALL, "RUS");
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        cerr << "Ошибка WSAStartup: " << WSAGetLastError() << "\n";
        return -1;
    }
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SRV_PORT);
    bind(s, (sockaddr*)&serverAddr, sizeof(serverAddr));
    listen(s, SOMAXCONN);
    cout << "Сервер запущен. Ожидание подключений...\n";
    while (true) {
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        SOCKET clientSock = accept(s, (sockaddr*)&clientAddr, &clientAddrLen);
        cout << "Новый клиент подключён!\n";
        while (true) {
            PatientData patient;
            int bytesReceived = recv(clientSock, (char*)&patient, sizeof(PatientData), 0);
            if (strcmp(patient.surname, "Bye") == 0) {
                cout << "Клиент завершил сессию.\n";
                break;
            }
            string surname = patient.surname;
            string result = IMT(patient.h, patient.w);
            cout << "Получены данные пациента:\n"
                << "Фамилия: " << patient.surname << "\n"
                << "Рост: " << patient.h << " см\n"
                << "Вес: " << patient.w << " кг\n"
                << "ИМТ: " << result << "\n";
            string answer = "\nВаши данные обработаны:" + surname + "\nВаш ИМТ: " + result;
            send(clientSock, answer.c_str(), answer.size(), 0);
        }
        closesocket(clientSock);
        cout << "Клиент обработан, ожидаем следующего...\n";
    }
    closesocket(s);
    WSACleanup();
    return 0;
}
