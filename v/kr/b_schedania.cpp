#include <windows.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <cctype>
#include <vector>
#include <time.h>
#include "json.hpp"

using json = nlohmann::json;

struct FileProcessingTime
{
    std::string filename;
    clock_t start_time;
    clock_t end_time;
};

std::vector<FileProcessingTime> fileProcessingTimes;
std::unordered_set<std::string> fileNamesSet;
clock_t startTime;
clock_t endTime;
CRITICAL_SECTION cs;

std::string ssdDirectoryIn = "C:\\CodingProjects\\C++\\osisp\\v\\kr\\files\\in\\";
std::string ssdDirectoryOut = "C:\\CodingProjects\\C++\\osisp\\v\\kr\\files\\out\\";

std::string flashDirectoryIn = "E:\\files\\in\\";
std::string flashDirectoryOut = "E:\\files\\out\\";

bool isVowel(char c)
{
    c = std::tolower(c);
    return (c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u' ||
            c == '?' || c == '?' || c == '?' || c == '?' || c == '?' ||
            c == '?' || c == '?' || c == '?' || c == '?' || c == '?');
}

// Function to remove vowels from a string
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

DWORD WINAPI processFileThread(LPVOID lpParam)
{
    std::pair<std::string, std::string> *filePair = reinterpret_cast<std::pair<std::string, std::string> *>(lpParam);
    std::string inputFilename = filePair->first;
    std::string outputFilename = filePair->second;
    delete filePair;

    FileProcessingTime processingTime;
    clock_t startFileTime, endFileTime;

    startFileTime = clock();

    processingTime.filename = inputFilename;

    std::ifstream inFile(inputFilename);
    if (!inFile.is_open())
    {
        std::cerr << "Error: Couldn't open input file '" << inputFilename << "'." << std::endl;
        return 1;
    }

    std::ofstream outFile(outputFilename);
    if (!outFile.is_open())
    {
        std::cerr << "Error: Couldn't open output file '" << outputFilename << "'." << std::endl;
        inFile.close();
        return 1;
    }

    std::string line;
    while (std::getline(inFile, line))
    {
        std::string processedLine = removeVowels(line);
        outFile << processedLine << '\n';
    }

    inFile.close();
    outFile.close();

    endFileTime = clock();

    processingTime.start_time = startFileTime;
    processingTime.end_time = endFileTime;

    EnterCriticalSection(&cs);
    fileProcessingTimes.push_back(processingTime);
    LeaveCriticalSection(&cs);

    std::cout << "Text processed and saved to '" << outputFilename << "'." << std::endl;

    return 0;
}

void writeToJSON(const std::string &filename)
{
    std::string outputFilename = ssdDirectoryOut + filename; // Add the directory path
    json jsonData;

    json programTimes;
    programTimes["start_program_time"] = static_cast<double>(startTime) / CLOCKS_PER_SEC;
    programTimes["end_program_time"] = static_cast<double>(endTime) / CLOCKS_PER_SEC;
    programTimes["elapse_program_time"] = static_cast<double>(endTime - startTime) / CLOCKS_PER_SEC;

    jsonData["program_times"] = programTimes;

    // Create a JSON array to hold file data
    json filesArray = json::array();

    for (const auto &processingTime : fileProcessingTimes)
    {
        json fileData;
        fileData["filename"] = processingTime.filename;
        fileData["start_time_seconds"] = static_cast<double>(processingTime.start_time) / CLOCKS_PER_SEC;
        fileData["end_time_seconds"] = static_cast<double>(processingTime.end_time) / CLOCKS_PER_SEC;
        fileData["elapse_time_seconds"] = static_cast<double>(processingTime.end_time - processingTime.start_time) / CLOCKS_PER_SEC;

        filesArray.push_back(fileData);
    }

    jsonData["file_processing_times"] = filesArray;

    std::ofstream outFile(outputFilename);
    if (outFile.is_open())
    {
        outFile << std::setw(4) << jsonData << std::endl;
        outFile.close();
        std::cout << "Data written to JSON file '" << outputFilename << "'." << std::endl;
    }
    else
    {
        std::cerr << "Error writing to file '" << outputFilename << "'." << std::endl;
    }
}

int main(int argc, char *argv[])
{
    startTime = clock();

    InitializeCriticalSection(&cs);

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename1> [<filename2> ... <filenameN>]" << std::endl;
        return 1;
    }

    std::vector<HANDLE> threadHandles;

    for (int i = 1; i < argc; ++i)
    {
        std::string inputFilename = std::string(ssdDirectoryIn) + argv[i];
        if (fileNamesSet.find(inputFilename) != fileNamesSet.end())
        {
            std::cout << "File '" << inputFilename << "' reoccurs. Skipping." << std::endl;
            continue;
        }
        fileNamesSet.insert(inputFilename);

        std::string outputFilename = std::string(ssdDirectoryOut) + "out_" + argv[i];

        std::pair<std::string, std::string> *filePair = new std::pair<std::string, std::string>(inputFilename, outputFilename);

        HANDLE hThread = CreateThread(NULL, 0, processFileThread, filePair, 0, NULL);
        if (hThread == NULL)
        {
            std::cerr << "Error creating thread for file '" << inputFilename << "'." << std::endl;
            delete filePair;
        }
        else
        {
            threadHandles.push_back(hThread);
        }
    }

    WaitForMultipleObjects(threadHandles.size(), threadHandles.data(), TRUE, INFINITE);

    for (auto &hThread : threadHandles)
    {
        CloseHandle(hThread);
    }

    endTime = clock();

    writeToJSON("file_processing_times_b.json");

    return 0;
}
