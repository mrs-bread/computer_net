#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <chrono>
#include <thread>
#include <windows.h>

using namespace std;

struct Student {
    string lastName;
    vector<int> grades;
};

struct Result {
    string lastName;
    vector<int> grades;
    int failedSubjects;
    string scholarshipStatus;
};

// Предварительные объявления
string getScholarshipStatus(const vector<int>& grades);
void processStudent(const Student& student, ofstream& outFile);

Student readStudent(ifstream& file) {
    Student student;
    size_t nameLength, numGrades;

    file.read(reinterpret_cast<char*>(&nameLength), sizeof(size_t));
    if (file.gcount() == 0) return student;

    student.lastName.resize(nameLength);
    file.read(&student.lastName[0], nameLength);

    file.read(reinterpret_cast<char*>(&numGrades), sizeof(size_t));
    student.grades.resize(numGrades);
    file.read(reinterpret_cast<char*>(student.grades.data()), numGrades * sizeof(int));

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

void writeResult(ofstream& outFile, const Result& result) {
    // Сериализация фамилии
    size_t nameLength = result.lastName.size();
    outFile.write(reinterpret_cast<const char*>(&nameLength), sizeof(size_t));
    outFile.write(result.lastName.c_str(), nameLength);

    // Сериализация оценок
    size_t numGrades = result.grades.size();
    outFile.write(reinterpret_cast<const char*>(&numGrades), sizeof(size_t));
    outFile.write(reinterpret_cast<const char*>(result.grades.data()),
        numGrades * sizeof(int));

    // Сериализация остальных полей
    outFile.write(reinterpret_cast<const char*>(&result.failedSubjects), sizeof(int));

    size_t statusLength = result.scholarshipStatus.size();
    outFile.write(reinterpret_cast<const char*>(&statusLength), sizeof(size_t));
    outFile.write(result.scholarshipStatus.c_str(), statusLength);
}

void processStudent(const Student& student, ofstream& outFile) {
    Result result;
    result.lastName = student.lastName;
    result.grades = student.grades;
    result.failedSubjects = count(student.grades.begin(), student.grades.end(), 2);
    result.scholarshipStatus = getScholarshipStatus(student.grades);

    writeResult(outFile, result);
}

void processFile(const string& inputFilename, ofstream& outputFile, streampos& lastPos) {
    ifstream input(inputFilename, ios::binary);
    if (input) {
        input.seekg(0, ios::end);
        streampos currentSize = input.tellg();

        if (currentSize > lastPos) {
            input.seekg(lastPos);

            while (true) {
                Student student = readStudent(input);
                if (student.lastName.empty()) break;

                processStudent(student, outputFile);
                cout << "Обработан: " << student.lastName << endl;
            }
            lastPos = currentSize;
        }
        input.close();
    }
}

int main() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);

    string inputFile = "C:/Users/Denis/Desktop/students.bin";
    string outputFile = "C:/Users/Denis/Desktop/results.bin";
    ofstream out(outputFile, ios::binary | ios::app);

    if (!out.is_open()) {
        cerr << "Ошибка открытия файла результатов!" << endl;
        return 1;
    }

    streampos lastPos = 0;

    cout << "Сервер запущен. Для остановки закройте окно.\n";

    while (true) {
        processFile(inputFile, out, lastPos);
        this_thread::sleep_for(chrono::milliseconds(500));
    }

    return 0;
}
