// server.cpp
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
    PT_Message,         // broadcast message
    PT_PrivateMessage,  // private message
    PT_SetNickname,     // client sets nickname
    PT_UserJoined,      // server notifies join
    PT_UserLeft         // server notifies leave
};

struct ClientInfo {
    SOCKET socket;
    string nickname;
};

vector<ClientInfo> clients;
mutex clientsMutex;

void broadcast(PacketType type, const string& sender, const string& target, const string& msg) {
    lock_guard<mutex> lock(clientsMutex);
    for (auto& ci : clients) {
        if (type == PT_PrivateMessage && ci.nickname != target)
            continue;
        int nick_len = sender.size();
        int target_len = target.size();
        int msg_len = msg.size();
        send(ci.socket, (char*)&type, sizeof(type), 0);
        send(ci.socket, (char*)&nick_len, sizeof(nick_len), 0);
        send(ci.socket, sender.c_str(), nick_len, 0);
        send(ci.socket, (char*)&target_len, sizeof(target_len), 0);
        if (target_len) send(ci.socket, target.c_str(), target_len, 0);
        send(ci.socket, (char*)&msg_len, sizeof(msg_len), 0);
        send(ci.socket, msg.c_str(), msg_len, 0);
    }
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
                for (auto& c : clients) {
                    if (c.socket == s) { c.nickname = nickname; break; }
                }
            }
            broadcast(PT_UserJoined, nickname, "", nickname + " joined the chat.");
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
        thread t(handleClient, ci);
        t.detach();
    }
    WSACleanup();
    return 0;
}
