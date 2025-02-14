
#include <iostream>
#include <fstream>
#include <string>
#include <io.h> // Для _setmode
#include <fcntl.h> // Для _O_U16TEXT

using namespace std;

struct Student {
    char surname[25];
    int grades[4];
};

void checkScholarship(Student student) {
    int countTwo = 0;
    bool allFive = true;
    bool hasFour = false;
    bool hasThree = false;
    bool hasTwo = false;
    for (int i = 0; i < 4; i++) {
        if (student.grades[i] == 2) {
            countTwo++;
            hasTwo = true;
        }
        if (student.grades[i] < 5) {
            allFive = false;
        }
        if (student.grades[i] == 3) {
            hasThree = true;
        }
        if (student.grades[i] >= 4 && student.grades[i] <= 5) {
            hasFour = true;
        }
    }
    wcout << L"Фамилия: " << student.surname << endl;
    wcout << L"Оценки: ";
    for (int i = 0; i < 4; i++) {
        wcout << student.grades[i] << L" ";
    }
    wcout << endl;
    if (allFive) {
        wcout << L"Стипендия: повышенная" << endl;
    }
    else if (hasThree || hasTwo) {
        wcout << L"Стипендии нет" << endl;
    }
    else if (hasFour) {
        wcout << L"Стипендия: обычная" << endl;
    }
    wcout << L"Количество задолженностей: " << countTwo << endl;
}

int main() {
    _setmode(_fileno(stdout), _O_U16TEXT); // Установка кодировки UTF-16 для stdout
    _setmode(_fileno(stdin), _O_U16TEXT); // Установка кодировки UTF-16 для stdin
    _setmode(_fileno(stderr), _O_U16TEXT); // Установка кодировки UTF-16 для stderr

    Student student;
    ifstream inputFile("C:/Users/Denis/Desktop/students.txt");
    if (inputFile.is_open()) {
        while (inputFile >> student.surname >> student.grades[0] >> student.grades[1] >> student.grades[2] >> student.grades[3]) {
            checkScholarship(student);
        }
        inputFile.close();
    }
    else {
        wcout << L"Не удалось открыть файл." << endl;
    }
    return 0;
}
