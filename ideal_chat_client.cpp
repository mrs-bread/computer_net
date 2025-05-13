#define _WINSOCK_DEPRECATED_NO_WARNINGS
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <string>
#include <map>
#include <vector>
#include <chrono>
#include <ctime>
#include <iomanip>
#include <sstream>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

const WORD COLOR_DEFAULT = 7;
const vector<WORD> USER_COLORS = { 1, 2, 3, 4, 5, 6, 9, 10, 11, 12, 13, 14 };
const WORD COLOR_PRIVATE_BG = BACKGROUND_BLUE | BACKGROUND_RED;

map<string, WORD> userColorMap;
int nextColorIndex = 0;

WORD getUserColor(const string& user) {
    if (!userColorMap.count(user)) {
        userColorMap[user] = USER_COLORS[nextColorIndex % USER_COLORS.size()];
        nextColorIndex++;
    }
    return userColorMap[user];
}

void setColor(WORD attrs) {
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), attrs);
}

string currentTime() {
    auto now = chrono::system_clock::now();
    time_t t = chrono::system_clock::to_time_t(now);
    tm local_tm;
    localtime_s(&local_tm, &t);
    ostringstream oss;
    oss << put_time(&local_tm, "[%H:%M:%S]");
    return oss.str();
}

void printSystem(const string& msg) {
    // рамка
    string border(msg.size() + 4, '-');
    cout << border << "\n";
    cout << "| " << msg << " |" << "\n";
    cout << border << "\n";
}

void printUserMessage(const string& sender, const string& msg) {
    WORD color = getUserColor(sender);
    setColor(color);
    cout << currentTime() << " " << sender << ": " << msg << endl;
    setColor(COLOR_DEFAULT);
}

void printPrivateMessage(const string& sender, const string& target, const string& msg) {
    setColor(COLOR_PRIVATE_BG | FOREGROUND_RED | FOREGROUND_GREEN | FOREGROUND_BLUE);
    cout << currentTime() << " [PM] " << sender << "->" << target << ": " << msg << endl;
    setColor(COLOR_DEFAULT);
}

enum PacketType { PT_Message, PT_PrivateMessage, PT_SetNickname, PT_UserJoined, PT_UserLeft };

void recvThread(SOCKET s) {
    while (true) {
        PacketType type;
        int ret = recv(s, (char*)&type, sizeof(type), 0);
        if (ret <= 0) break;
        int nl; recv(s, (char*)&nl, sizeof(nl), 0);
        string sender(nl, '\0'); recv(s, &sender[0], nl, 0);
        int tl; recv(s, (char*)&tl, sizeof(tl), 0);
        string target(tl, '\0'); if (tl) recv(s, &target[0], tl, 0);
        int ml; recv(s, (char*)&ml, sizeof(ml), 0);
        string msg(ml, '\0'); recv(s, &msg[0], ml, 0);
        switch (type) {
        case PT_Message:
            printUserMessage(sender, msg);
            break;
        case PT_PrivateMessage:
            printPrivateMessage(sender, target, msg);
            break;
        case PT_UserJoined:
        case PT_UserLeft:
            setColor(14);
            printSystem(msg);
            setColor(COLOR_DEFAULT);
            break;
        }
    }
    setColor(14);
    printSystem("Disconnected from server.");
    closesocket(s);
    exit(0);
}

int main() {
    WSADATA wsa; WSAStartup(MAKEWORD(2, 2), &wsa);
    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(12345);

    if (connect(s, (sockaddr*)&addr, sizeof(addr)) != 0) { cerr << "Unable to connect"; return 1; }
    thread(recvThread, s).detach();

    cout << "Enter your nickname: ";
    string nick; getline(cin, nick);
    PacketType pt = PT_SetNickname; int nl = nick.size();
    send(s, (char*)&pt, sizeof(pt), 0);
    send(s, (char*)&nl, sizeof(nl), 0);
    send(s, nick.c_str(), nl, 0);

    while (true) {
        string line; getline(cin, line);
        if (line == "/exit") { printSystem("Exiting chat..."); closesocket(s); break; }
        if (line.rfind("/pm ", 0) == 0) {
            size_t pos = line.find(' ', 4);
            string tgt = line.substr(4, pos - 4);
            string m = line.substr(pos + 1);
            pt = PT_PrivateMessage;
            int tl = tgt.size(), ml = m.size();
            send(s, (char*)&pt, sizeof(pt), 0);
            send(s, (char*)&nl, sizeof(nl), 0); send(s, nick.c_str(), nl, 0);
            send(s, (char*)&tl, sizeof(tl), 0); send(s, tgt.c_str(), tl, 0);
            send(s, (char*)&ml, sizeof(ml), 0); send(s, m.c_str(), ml, 0);
        }
        else {
            pt = PT_Message;
            int ml = line.size(); int zero = 0;
            send(s, (char*)&pt, sizeof(pt), 0);
            send(s, (char*)&nl, sizeof(nl), 0); send(s, nick.c_str(), nl, 0);
            send(s, (char*)&zero, sizeof(zero), 0);
            send(s, (char*)&ml, sizeof(ml), 0); send(s, line.c_str(), ml, 0);
        }
    }
    WSACleanup();
    return 0;
}
