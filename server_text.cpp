// server.cpp (исправленная версия)
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

Student parseStudentData(const string& line) {
    Student student;
    istringstream iss(line);
    iss >> student.lastName;
    int grade;
    while (iss >> grade) {
        student.grades.push_back(grade);
    }
    return student;
}

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

void processFile(const string& inputFilename, ofstream& outputFile, streampos& lastFileSize) {
    ifstream inputFile(inputFilename);
    if (inputFile.is_open()) {
        inputFile.seekg(0, ios::end); // Важное исправление: получаем текущий размер файла
        streampos currentSize = inputFile.tellg();

        if (currentSize > lastFileSize) {
            inputFile.seekg(lastFileSize);

            string line;
            while (getline(inputFile, line)) {
                Student student = parseStudentData(line);
                processStudent(student, outputFile);
                cout << "Обработаны новые данные для студента: " << student.lastName << endl;
            }
            lastFileSize = currentSize;
        }
        inputFile.close();
    }
    else {
        cerr << "Не удалось открыть файл: " << inputFilename << endl;
    }
}

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    string inputFilename = "C:/Users/Denis/Desktop/students.txt";
    string outputFilename = "C:/Users/Denis/Desktop/results.txt";

    ofstream outputFile(outputFilename, ios::app);
    if (!outputFile.is_open()) {
        cerr << "Ошибка: не удалось открыть файл результатов!" << endl;
        return 1;
    }

    streampos lastFileSize = 0;

    cout << "Сервер запущен. Для завершения работы закройте это окно." << endl;

    // Бесконечный цикл проверки файла
    while (true) {
        processFile(inputFilename, outputFile, lastFileSize);
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    // Теоретически недостижимый код
    outputFile.close();
    return 0;
}
