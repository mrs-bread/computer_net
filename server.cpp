#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream>
#include <thread>
#include <chrono>
#include <filesystem>
#include <locale>

using namespace std;
struct Student {
    string surname;
    vector<int> grades;
};

void processStudent(const Student& student, ofstream& outFile) {
    setlocale(LC_ALL, "RUS");
    outFile << "Фамилия: " << student.surname << "\n";
    outFile << "Оценки: ";
    int countTwos = 0;
    bool hasFour = false;
    bool hasThree = false;

    for (int grade : student.grades) {
        outFile << grade << " ";
        if (grade == 2) {
            countTwos++;
        }
        else if (grade == 4) {
            hasFour = true;
        }
        else if (grade ==3) {
            hasThree = true;
        }

        if (countTwos > 0)
            hasThree = true;
    }
    outFile << "\n";

    if (countTwos > 0) {
        outFile << "Количество задолженностей: " << countTwos << "\n";
    }

    if (hasThree) {
        outFile << "Студент не имеет стипендии.\n";
    }
    else if (hasFour) {
        outFile << "Студент получает обычную стипендию.\n";
    }
    else {
        outFile << "Студент получает повышенную стипендию.\n";
    }

    outFile << "\n";
}

void monitorFileForChanges() {
    setlocale(LC_ALL, "RUS");
    string filename = "C:/Users/Denis/Desktop/students.txt";
    string resultFilename = "C:/Users/Denis/Desktop/results.txt";
    ofstream outFile(resultFilename, ios::app);

    if (!outFile) {
        cerr << "Ошибка открытия файла для записи результата." << endl;
        return;
    }

    // Устанавливаем локаль для корректной работы с русскими символами
    outFile.imbue(locale("ru_RU.UTF-8"));

    // Читаем текущий размер файла
    size_t lastFileSize = 0;
    if (filesystem::exists(filename)) {
        lastFileSize = filesystem::file_size(filename);
    }

    while (true) {
        if (filesystem::exists(filename)) {
            size_t currentFileSize = filesystem::file_size(filename);
            if (currentFileSize > lastFileSize) {
                ifstream inFile(filename);
                if (!inFile) {
                    cerr << "Ошибка открытия файла для чтения." << endl;
                    continue;
                }

                inFile.seekg(lastFileSize); // Перемещаемся к концу последнего прочитанного блока
                string line;
                while (getline(inFile, line)) {
                    istringstream iss(line);
                    Student student;
                    iss >> student.surname;

                    int grade;
                    while (iss >> grade) {
                        student.grades.push_back(grade);
                    }

                    processStudent(student, outFile);
                }

                lastFileSize = currentFileSize;
            }
        }

        this_thread::sleep_for(chrono::seconds(1)); // Ждем 1 секунду перед следующей проверкой
    }

    outFile.close();
}

int main() {
    setlocale(LC_ALL, "RUS");
    thread serverThread(monitorFileForChanges);
    serverThread.join();

    return 0;
}
