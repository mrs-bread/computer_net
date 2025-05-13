// client.cpp
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

// Цветовые атрибуты консоли
const WORD COLOR_DEFAULT = 7;
const WORD COLOR_MESSAGE = 10;       // светло-зеленый
const WORD COLOR_PRIVATE = 13;       // светло-пурпурный
const WORD COLOR_NOTIFICATION = 14;  // светло-желтый

void setColor(WORD color) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), color);
}

void printColored(WORD color, const string& text) {
    setColor(color);
    cout << text << endl;
    setColor(COLOR_DEFAULT);
}

enum PacketType {
    PT_Message,
    PT_PrivateMessage,
    PT_SetNickname,
    PT_UserJoined,
    PT_UserLeft
};

void recvThread(SOCKET s) {
    while (true) {
        PacketType type;
        int ret = recv(s, (char*)&type, sizeof(type), 0);
        if (ret <= 0) break;
        int nick_len;
        recv(s, (char*)&nick_len, sizeof(nick_len), 0);
        string sender(nick_len, '\0');
        recv(s, &sender[0], nick_len, 0);
        int target_len;
        recv(s, (char*)&target_len, sizeof(target_len), 0);
        string target(target_len, '\0');
        if (target_len)
            recv(s, &target[0], target_len, 0);
        int msg_len;
        recv(s, (char*)&msg_len, sizeof(msg_len), 0);
        string msg(msg_len, '\0');
        recv(s, &msg[0], msg_len, 0);
        switch (type) {
        case PT_Message:
            printColored(COLOR_MESSAGE, sender + ": " + msg);
            break;
        case PT_PrivateMessage:
            printColored(COLOR_PRIVATE, "[PM] " + sender + "->" + target + ": " + msg);
            break;
        case PT_UserJoined:
            printColored(COLOR_NOTIFICATION, msg);
            break;
        case PT_UserLeft:
            printColored(COLOR_NOTIFICATION, msg);
            break;
        }
    }
    printColored(COLOR_NOTIFICATION, "Disconnected from server.");
    closesocket(s);
    exit(0);
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(12345);
    if (connect(s, (sockaddr*)&addr, sizeof(addr)) != 0) {
        cerr << "Unable to connect to server\n";
        return 1;
    }
    thread t(recvThread, s);

    string nick;
    cout << "Enter your nickname: ";
    getline(cin, nick);
    PacketType pt = PT_SetNickname;
    int len = nick.size();
    send(s, (char*)&pt, sizeof(pt), 0);
    send(s, (char*)&len, sizeof(len), 0);
    send(s, nick.c_str(), len, 0);

    while (true) {
        string line;
        getline(cin, line);
        if (line == "/exit") {
            printColored(COLOR_NOTIFICATION, "Exiting chat...");
            closesocket(s);
            exit(0);
        }
        if (line.rfind("/pm ", 0) == 0) {
            auto pos = line.find(' ', 4);
            string target = line.substr(4, pos - 4);
            string msg = line.substr(pos + 1);
            pt = PT_PrivateMessage;
            int nl = nick.size();
            int tl = target.size();
            int ml = msg.size();
            send(s, (char*)&pt, sizeof(pt), 0);
            send(s, (char*)&nl, sizeof(nl), 0); send(s, nick.c_str(), nl, 0);
            send(s, (char*)&tl, sizeof(tl), 0); send(s, target.c_str(), tl, 0);
            send(s, (char*)&ml, sizeof(ml), 0); send(s, msg.c_str(), ml, 0);
        }
        else {
            pt = PT_Message;
            int nl = nick.size();
            int ml = line.size();
            send(s, (char*)&pt, sizeof(pt), 0);
            send(s, (char*)&nl, sizeof(nl), 0); send(s, nick.c_str(), nl, 0);
            int zero = 0;
            send(s, (char*)&zero, sizeof(zero), 0);
            send(s, (char*)&ml, sizeof(ml), 0); send(s, line.c_str(), ml, 0);
        }
    }

    t.join();
    WSACleanup();
    return 0;
}
