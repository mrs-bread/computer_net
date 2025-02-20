#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <windows.h>

using namespace std;

void writeToFile(const string& filename, const string& data) {
    ofstream file(filename, ios::app);
    if (file.is_open()) {
        file << data << endl;
        file.flush();
        file.close();
    }
    else {
        cerr << "Не удалось открыть файл: " << filename << endl;
    }
}

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    string filename = "C:/Users/Denis/Desktop/students.txt";
    string lastName;
    vector<int> grades(4);

    while (true) {
        cout << "Введите фамилию студента (или 'выход' для завершения): ";
        cin >> lastName;

        if (lastName == "выход") {
            break;
        }

        cout << "Введите 4 оценки (2, 3, 4 или 5), разделенные пробелами: ";
        for (int i = 0; i < 4; ++i) {
            cin >> grades[i];
            if (grades[i] < 2 || grades[i] > 5) {
                cout << "Неверная оценка. Пожалуйста, введите 2, 3, 4 или 5." << endl;
                i--;
            }
        }

        string data = lastName;
        for (int grade : grades) {
            data += " " + to_string(grade);
        }

        writeToFile(filename, data);
        cout << "Данные студента сохранены." << endl;
    }

    return 0;
}
