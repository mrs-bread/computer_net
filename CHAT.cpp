//server.cpp
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <winsock2.h>
#include <windows.h>
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <string>
#pragma comment(lib, "ws2_32.lib")

using namespace std;

enum PacketType {
    PT_Message,
    PT_PrivateMessage,
    PT_SetNickname,
    PT_UserJoined,
    PT_UserLeft,
    PT_ListRequest,      // <<< ДОБАВЛЕНО: клиент попросил список пользователей
    PT_ListResponse      // <<< ДОБАВЛЕНО: сервер отправляет список пользователей
};

struct ClientInfo {
    SOCKET socket;
    string nickname;
};

vector<ClientInfo> clients;
mutex clientsMutex;

// Универсальный broadcast (без фильтра)
void broadcast(PacketType type, const string& sender, const string& target, const string& msg) {
    lock_guard<mutex> lock(clientsMutex);
    for (auto& ci : clients) {
        if (type == PT_PrivateMessage && ci.nickname != target)
            continue;
        int nick_len = (int)sender.size();
        int target_len = (int)target.size();
        int msg_len = (int)msg.size();
        send(ci.socket, (char*)&type, sizeof(type), 0);
        send(ci.socket, (char*)&nick_len, sizeof(nick_len), 0);
        send(ci.socket, sender.c_str(), nick_len, 0);
        send(ci.socket, (char*)&target_len, sizeof(target_len), 0);
        if (target_len) send(ci.socket, target.c_str(), target_len, 0);
        send(ci.socket, (char*)&msg_len, sizeof(msg_len), 0);
        send(ci.socket, msg.c_str(), msg_len, 0);
    }
}

// Отправить список пользователей одному клиенту
void sendUserList(SOCKET s) {
    string list;
    {
        lock_guard<mutex> lock(clientsMutex);
        for (auto& c : clients) {
            if (!c.nickname.empty()) {
                list += c.nickname + "\n";
            }
        }
    }
    PacketType type = PT_ListResponse;
    int sender_len = 0;           // мы можем оставить sender пустым
    int target_len = 0;           // не нужно
    int msg_len = (int)list.size();
    send(s, (char*)&type, sizeof(type), 0);
    send(s, (char*)&sender_len, sizeof(sender_len), 0);
    // sender отсутствует
    send(s, (char*)&target_len, sizeof(target_len), 0);
    // target отсутствует
    send(s, (char*)&msg_len, sizeof(msg_len), 0);
    send(s, list.c_str(), msg_len, 0);
}

void handleClient(ClientInfo ci) {
    SOCKET s = ci.socket;
    string nickname;
    while (true) {
        PacketType type;
        int ret = recv(s, (char*)&type, sizeof(type), 0);
        if (ret <= 0) break;

        if (type == PT_SetNickname) {
            int len;
            recv(s, (char*)&len, sizeof(len), 0);
            vector<char> buf(len + 1);
            recv(s, buf.data(), len, 0);
            nickname = buf.data();
            {
                lock_guard<mutex> lock(clientsMutex);
                for (auto& c : clients)
                    if (c.socket == s) { c.nickname = nickname; break; }
            }
            broadcast(PT_UserJoined, nickname, "", nickname + " joined the chat.");
        }
        else if (type == PT_ListRequest) {         // <<< ДОБАВЛЕНО: запрос списка юзеров
            sendUserList(s);
        }
        else if (type == PT_Message || type == PT_PrivateMessage) {
            int len;
            recv(s, (char*)&len, sizeof(len), 0);
            vector<char> tmp(len + 1);
            recv(s, tmp.data(), len, 0);

            int target_len;
            recv(s, (char*)&target_len, sizeof(target_len), 0);
            string target;
            if (target_len) {
                vector<char> tbuf(target_len + 1);
                recv(s, tbuf.data(), target_len, 0);
                target = tbuf.data();
            }

            int msg_len;
            recv(s, (char*)&msg_len, sizeof(msg_len), 0);
            vector<char> mbuf(msg_len + 1);
            recv(s, mbuf.data(), msg_len, 0);
            string message = mbuf.data();

            if (type == PT_Message)
                broadcast(PT_Message, nickname, "", message);
            else
                broadcast(PT_PrivateMessage, nickname, target, message);
        }
    }

    // Удаляем клиента из списка и уведомляем всех
    {
        lock_guard<mutex> lock(clientsMutex);
        for (auto it = clients.begin(); it != clients.end(); ++it) {
            if (it->socket == s) {
                nickname = it->nickname;
                clients.erase(it);
                break;
            }
        }
    }
    broadcast(PT_UserLeft, nickname, "", nickname + " left the chat.");
    closesocket(s);
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET listenSock = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(12345);

    bind(listenSock, (sockaddr*)&addr, sizeof(addr));
    listen(listenSock, SOMAXCONN);
    cout << "Server listening on port 12345...\n";

    while (true) {
        SOCKET clientSock = accept(listenSock, nullptr, nullptr);
        if (clientSock == INVALID_SOCKET) continue;
        ClientInfo ci{ clientSock, "" };

        {
            lock_guard<mutex> lock(clientsMutex);
            clients.push_back(ci);
        }
        thread(handleClient, ci).detach();
    }

    WSACleanup();
    return 0;
}


//client.cpp
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

// Цветовые атрибуты консоли
const WORD COLOR_DEFAULT = 7;
const vector<WORD> USER_COLORS = { 1,2,3,4,5,6,9,10,11,12,13,14 };
const WORD COLOR_PRIVATE_BG = BACKGROUND_RED | BACKGROUND_BLUE;

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
    setColor(14);
    string border(msg.size() + 4, '-');
    cout << border << "\n";
    cout << "| " << msg << " |\n";
    cout << border << "\n";
    setColor(COLOR_DEFAULT);
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

enum PacketType {
    PT_Message,
    PT_PrivateMessage,
    PT_SetNickname,
    PT_UserJoined,
    PT_UserLeft,
    PT_ListRequest,    // <<< ДОБАВЛЕНО
    PT_ListResponse    // <<< ДОБАВЛЕНО
};

void recvThread(SOCKET s) {
    while (true) {
        PacketType type;
        int ret = recv(s, (char*)&type, sizeof(type), 0);
        if (ret <= 0) break;

        int nl;
        recv(s, (char*)&nl, sizeof(nl), 0);
        string sender(nl, '\0');
        if (nl) recv(s, &sender[0], nl, 0);

        int tl;
        recv(s, (char*)&tl, sizeof(tl), 0);
        string target(tl, '\0');
        if (tl) recv(s, &target[0], tl, 0);

        int ml;
        recv(s, (char*)&ml, sizeof(ml), 0);
        string msg(ml, '\0');
        if (ml) recv(s, &msg[0], ml, 0);

        switch (type) {
        case PT_Message:
            printUserMessage(sender, msg);
            break;
        case PT_PrivateMessage:
            printPrivateMessage(sender, target, msg);
            break;
        case PT_UserJoined:
        case PT_UserLeft:
            printSystem(msg);
            break;
        case PT_ListResponse:    // <<< ДОБАВЛЕНО: сервер прислал список
            printSystem("Online users:\n" + msg);
            break;
        }
    }
    printSystem("Disconnected from server.");
    closesocket(s);
    exit(0);
}

int main() {
    WSADATA wsa;
    WSAStartup(MAKEWORD(2, 2), &wsa);

    SOCKET s = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("192.168.0.17");
    addr.sin_port = htons(12345);
    if (connect(s, (sockaddr*)&addr, sizeof(addr)) != 0) {
        cerr << "Unable to connect to server\n";
        return 1;
    }

    // Сначала вводим никнейм, потом запускаем приём сообщений
    cout << "Enter your nickname: ";
    string nick;
    getline(cin, nick);

    PacketType pt = PT_SetNickname;
    int nl = (int)nick.size();
    send(s, (char*)&pt, sizeof(pt), 0);
    send(s, (char*)&nl, sizeof(nl), 0);
    send(s, nick.c_str(), nl, 0);

    thread(recvThread, s).detach();

    // Основной цикл ввода
    while (true) {
        string line;
        getline(cin, line);
        if (line == "/exit") {
            printSystem("Exiting chat...");
            closesocket(s);
            break;
        }
        else if (line == "/list") {        // <<< ДОБАВЛЕНО: если пользователь ввёл /list
            pt = PT_ListRequest;
            int zero = 0;                   // sender и target нам не нужны
            send(s, (char*)&pt, sizeof(pt), 0);
            send(s, (char*)&zero, sizeof(zero), 0);
            // не отправляем sender
            send(s, (char*)&zero, sizeof(zero), 0);
            // не отправляем target
            send(s, (char*)&zero, sizeof(zero), 0);
            // не отправляем msg
        }
        else if (line.rfind("/pm ", 0) == 0) {
            size_t pos = line.find(' ', 4);
            string tgt = line.substr(4, pos - 4);
            string m = line.substr(pos + 1);

            pt = PT_PrivateMessage;
            int tl = (int)tgt.size();
            int ml = (int)m.size();
            send(s, (char*)&pt, sizeof(pt), 0);
            send(s, (char*)&nl, sizeof(nl), 0);
            send(s, nick.c_str(), nl, 0);
            send(s, (char*)&tl, sizeof(tl), 0);
            send(s, tgt.c_str(), tl, 0);
            send(s, (char*)&ml, sizeof(ml), 0);
            send(s, m.c_str(), ml, 0);
        }
        else {
            pt = PT_Message;
            int ml = (int)line.size();
            int zero = 0;
            send(s, (char*)&pt, sizeof(pt), 0);
            send(s, (char*)&nl, sizeof(nl), 0);
            send(s, nick.c_str(), nl, 0);
            send(s, (char*)&zero, sizeof(zero), 0); // target = ""
            send(s, (char*)&ml, sizeof(ml), 0);
            send(s, line.c_str(), ml, 0);
        }
    }

    WSACleanup();
    return 0;
}
