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

int main() {
    while (true) {
        Student student;
        cout << "Enter name (or type 'exit' to quit): ";
        //cin.ignore(numeric_limits<streamsize>::max(), '\n');
        cin.getline(student.name, 50);
        if (string(student.name) == "exit") break;

        cout << "Enter height (m): ";
        cin >> student.height;
        while (cin.fail() || student.height <= 0 || student.height > 3.0) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid height. Enter again: ";
            cin >> student.height;
        }

        cout << "Enter weight (kg): ";
        cin >> student.weight;
        while (cin.fail() || student.weight <= 0 || student.weight > 500) {
            cin.clear();
            cin.ignore(numeric_limits<streamsize>::max(), '\n');
            cout << "Invalid weight. Enter again: ";
            cin >> student.weight;
        }
        cin.ignore(numeric_limits<streamsize>::max(), '\n');

        string conFilePath = BASE_PATH + "coni.txt";
        ofstream conFile(conFilePath, ios::app);
        conFile << student.name << endl;
        conFile.close();

        string fileName = BASE_PATH + string(student.name) + ".bin";
        ofstream outFile(fileName, ios::binary);
        outFile.write(reinterpret_cast<char*>(&student), sizeof(Student));
        outFile.close();

        cout << "Data sent to server. Waiting for response..." << endl;

        string result;
        while (true) {
            ifstream inFile(fileName, ios::binary);
            if (inFile) {
                getline(inFile, result);
                if (!result.empty()) {
                    cout << "Server response: " << result << endl;
                    break;
                }
            }
            inFile.close();
            this_thread::sleep_for(chrono::seconds(1));
        }
    }
    return 0;
}
