#include <iostream>
#include <fstream>
#include <string>
#include <thread>
#include <chrono>

using namespace std;

const string BASE_PATH = "C:/Users/Denis/Desktop/";

struct Student {
    char name[50];
    float height;
    float weight;
};

string analyze(float height, float weight) {
    float bmi = weight / (height * height);
    if (bmi < 18.5) return "Underweight";
    if (bmi > 25.0) return "Overweight";
    return "Normal";
}

void handleClient(const string& clientName) {
    string clientFile = BASE_PATH + clientName + ".bin";
    ifstream inFile(clientFile, ios::binary);
    if (!inFile) {
        cerr << "Error: Unable to open file for client: " << clientName << endl;
        return;
    }

    Student student;
    inFile.read(reinterpret_cast<char*>(&student), sizeof(Student));
    inFile.close();

    cout << "Processing client: " << student.name << " Height: " << student.height << " Weight: " << student.weight << endl;

    string result = analyze(student.height, student.weight);
    ofstream outFile(clientFile, ios::binary | ios::trunc);
    outFile.write(result.c_str(), result.size());
    outFile.close();

    cout << "Processed client: " << student.name << " - Result: " << result << endl;
}

int main() {
    string conFilePath = BASE_PATH + "coni.txt";
    ofstream(conFilePath, ios::trunc).close();
    cout << "Server started..." << endl;
    while (true) {
        ifstream conFile(conFilePath);
        string clientName;
        while (getline(conFile, clientName)) {
            if (!clientName.empty()) {
                handleClient(clientName);
            }
        }
        conFile.close();
        ofstream clearCon(conFilePath, ios::trunc);
        clearCon.close();
        this_thread::sleep_for(chrono::seconds(1));
    }
    return 0;
}
