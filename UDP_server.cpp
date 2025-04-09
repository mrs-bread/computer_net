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

    SOCKET s = socket(AF_INET, SOCK_DGRAM, 0);
    sockaddr_in serverAddr;
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(SRV_PORT);

    bind(s, (sockaddr*)&serverAddr, sizeof(serverAddr));
    cout << "UDP-сервер запущен. Ожидание сообщений...\n";

    while (true) {
        sockaddr_in clientAddr;
        int clientAddrLen = sizeof(clientAddr);
        PatientData patient;

        int bytesReceived = recvfrom(s, (char*)&patient, sizeof(PatientData), 0,
            (sockaddr*)&clientAddr, &clientAddrLen);

        if (bytesReceived <= 0) continue;

        if (strcmp(patient.surname, "Bye") == 0) {
            cout << "Клиент завершил сессию.\n";
            continue;
        }

        string surname = patient.surname;
        string result = IMT(patient.h, patient.w);

        cout << "\nПолучены данные пациента:\n"
            << "Фамилия: " << patient.surname << "\n"
            << "Рост: " << patient.h << " см\n"
            << "Вес: " << patient.w << " кг\n"
            << "ИМТ: " << result << "\n";

        string answer = "Ваши данные обработаны: " + surname + "\nВаш ИМТ: " + result;
        sendto(s, answer.c_str(), answer.size(), 0,
            (sockaddr*)&clientAddr, clientAddrLen);
    }

    closesocket(s);
    WSACleanup();
    return 0;
}
