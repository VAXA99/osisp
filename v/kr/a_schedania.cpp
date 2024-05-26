#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <cctype>
#include <windows.h>
#include <time.h>

// Глобальные переменные
std::unordered_set<std::string> fileNamesSet;
clock_t startTime;
clock_t endTime;

// Функция для проверки, является ли символ гласной буквой
bool isVowel(char c)
{
    c = std::tolower(c);
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
            c == 'а' || c == 'е' || c == 'ё' || c == 'и' || c == 'о' ||
            c == 'у' || c == 'ы' || c == 'э' || c == 'ю' || c == 'я');
}

// Функция для исключения гласных из строки
std::string removeVowels(const std::string &text)
{
    std::string result;
    for (char c : text)
    {
        if (!isVowel(c))
        {
            result += c;
        }
    }
    return result;
}

void processFile(const std::string &inputFilename, const std::string &outputFilename)
{

    std::ifstream inFile(inputFilename);
    if (!inFile.is_open())
    {
        std::cerr << "Ошибка: Не удалось открыть файл " << inputFilename << std::endl;
        return;
    }

    std::ofstream outFile(outputFilename);
    if (!outFile.is_open())
    {
        std::cerr << "Ошибка: Не удалось создать файл " << outputFilename << std::endl;
        inFile.close();
        return;
    }

    std::string line;
    while (std::getline(inFile, line))
    {
        std::string processedLine = removeVowels(line);
        outFile << processedLine << '\n';
    }

    inFile.close();
    outFile.close();


    std::cout << "Текст без гласных сохранен в " << outputFilename << std::endl;
}

int main(int argc, char *argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    startTime = clock();

    if (argc < 2)
    {
        std::cerr << "Использование: " << argv[0] << " <filename1> [<filename2> ... <filenameN>]" << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i)
    {
        std::string inputFilename = std::string("files/in/") + argv[i];
        if (fileNamesSet.find(inputFilename) != fileNamesSet.end())
        {
            std::cout << "Файла " << inputFilename << " повторяется в списке аргументов. Пропускается" << std::endl;
            continue;
        }
        fileNamesSet.insert(inputFilename);

        std::string outputFilename = std::string("files/out/") + "out_" + argv[i];

        processFile(inputFilename, outputFilename);
    }

    endTime = clock();

    return 0;
}
