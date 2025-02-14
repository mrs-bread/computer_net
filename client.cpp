#include <iostream>
#include <fstream>
#include <string>
#include <locale>
using namespace std;

int main() {
    setlocale(LC_ALL, "RUS");
    ofstream outFile("C:/Users/Denis/Desktop/students.txt", ios::app);
    if (!outFile) {
        cerr << "Ошибка открытия файла для записи." << endl;
        return 1;
    }

    outFile.imbue(locale("ru_RU.UTF-8"));

    int numStudents;
    cout << "Введите количество студентов: ";
    cin >> numStudents;

    for (int i = 0; i < numStudents; ++i) {
        string surname;
        int grades[4];

        cout << "Введите фамилию студента " << i + 1 << ": ";
        cin >> surname;

        cout << "Введите 4 оценки (2, 3, 4 или 5) для студента " << i + 1 << ": ";
        for (int j = 0; j < 4; ++j) {
            cin >> grades[j];
            while (grades[j] < 2 || grades[j] > 5) {
                cout << "Оценка должна быть от 2 до 5. Введите снова: ";
                cin >> grades[j];
            }
        }

        outFile << surname << " ";
        for (int j = 0; j < 4; ++j) {
            outFile << grades[j] << " ";
        }
        outFile << "\n";
    }

    outFile.close();
    cout << "Данные успешно записаны." << endl;

    return 0;
}
