#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <chrono>
#include <thread>
#include <windows.h>

using namespace std;

struct Student {
    string lastName;
    vector<int> grades;
};

Student readStudent(ifstream& file) {
    Student student;
    size_t nameLength, numGrades;

    // Чтение фамилии
    file.read(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
    if (file.gcount() == 0) return student; // Конец файла

    vector<char> buffer(nameLength + 1);
    file.read(buffer.data(), nameLength);
    buffer[nameLength] = '\0';
    student.lastName = buffer.data();

    // Чтение оценок
    file.read(reinterpret_cast<char*>(&numGrades), sizeof(size_t));
    student.grades.resize(numGrades);
    file.read(reinterpret_cast<char*>(student.grades.data()), numGrades * sizeof(int));

    return student;
}

// Функции getScholarshipStatus и processStudent остаются без изменений

string getScholarshipStatus(const vector<int>& grades) {
    bool hasThree = false;
    bool hasFour = false;
    bool allFive = true;

    for (int grade : grades) {
        if (grade == 3) hasThree = true;
        if (grade == 4) hasFour = true;
        if (grade != 5) allFive = false;
    }

    if (hasThree) return "Без стипендии";
    if (allFive) return "Повышенная стипендия";
    if (hasFour) return "Обычная стипендия";
    return "Без стипендии";
}

void processStudent(const Student& student, ofstream& outFile) {
    int failedSubjects = count(student.grades.begin(), student.grades.end(), 2);
    string scholarshipStatus = getScholarshipStatus(student.grades);

    // Добавляем явную проверку открытия файла
    if (!outFile.is_open()) {
        cerr << "Ошибка: файл результатов не открыт!" << endl;
        return;
    }

    outFile << "Фамилия: " << student.lastName << endl;
    outFile << "Оценки: ";
    for (int grade : student.grades) {
        outFile << grade << " ";
    }
    outFile << endl;
    outFile << "Количество задолженностей: " << failedSubjects << endl;
    outFile << "Статус стипендии: " << scholarshipStatus << endl;
    outFile << endl;

    // Явный сброс буфера
    outFile.flush();
}


void processFile(const string& inputFilename, ofstream& outputFile, streampos& lastPos) {
    ifstream input(inputFilename, ios::binary);
    if (input) {
        input.seekg(lastPos);

        while (true) {
            Student student = readStudent(input);
            if (student.lastName.empty()) break;

            // Обработка студента
            processStudent(student, outputFile);
            cout << "Обработан: " << student.lastName << endl;
        }

        lastPos = input.tellg();
    }
}

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    string inputFile = "C:/Users/Denis/Desktop/students.bin";
    string outputFile = "C:/Users/Denis/Desktop/results.txt";
    ofstream out(outputFile, ios::app);

    streampos lastPos = 0;

    cout << "Сервер запущен. Для остановки закройте окно.\n";

    while (true) {
        processFile(inputFile, out, lastPos);
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    return 0;
}
