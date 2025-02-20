#include <limits>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#define NOMINMAX
#include <windows.h>

using namespace std;

struct Student {
    string lastName;
    vector<int> grades;
};

void writeToFile(const string& filename, const Student& student) {
    ofstream file(filename, ios::binary | ios::app);
    if (file.is_open()) {
        // Сериализация фамилии
        size_t nameLength = student.lastName.size();
        file.write(reinterpret_cast<const char*>(&nameLength), sizeof(size_t));
        file.write(student.lastName.c_str(), nameLength);

        // Сериализация оценок
        size_t numGrades = student.grades.size();
        file.write(reinterpret_cast<const char*>(&numGrades), sizeof(size_t));
        file.write(reinterpret_cast<const char*>(student.grades.data()),
            numGrades * sizeof(int));

        file.flush();
    }
}

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    string filename = "C:/Users/Denis/Desktop/students.bin";
    Student student;

    while (true) {
        cout << "Введите фамилию студента (или 'выход' для завершения): ";
        cin >> student.lastName;

        if (student.lastName == "выход") break;

        student.grades.resize(4);
        cout << "Введите 4 оценки (2-5): ";
        for (int i = 0; i < 4; ++i) {
            while (!(cin >> student.grades[i]) || student.grades[i] < 2 || student.grades[i] > 5) {
                cin.clear();
                cin.ignore(numeric_limits<streamsize>::max(), '\n');
                cout << "Некорректная оценка. Повторите ввод: ";
            }
        }

        writeToFile(filename, student);
        cout << "Данные сохранены.\n";
    }

    return 0;
}
