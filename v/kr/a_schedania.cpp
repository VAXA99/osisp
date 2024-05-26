#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <unordered_set>
#include <cctype>
#include <windows.h>
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

// Set to store filenames
std::unordered_set<std::string> fileNamesSet;
clock_t startTime;
clock_t endTime;

// Function to check if a character is a vowel
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

void processFile(const std::string &inputFilename, const std::string &outputFilename)
{
    FileProcessingTime processingTime;
    clock_t startFileTime, endFileTime;

    startFileTime = clock();

    processingTime.filename = inputFilename;

    std::ifstream inFile(inputFilename);
    if (!inFile.is_open())
    {
        std::cerr << "Error: Couldn't open input file '" << inputFilename << "'." << std::endl;
        return;
    }

    std::ofstream outFile(outputFilename);
    if (!outFile.is_open())
    {
        std::cerr << "Error: Couldn't open output file '" << outputFilename << "'." << std::endl;
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

    endFileTime = clock();

    processingTime.start_time = startFileTime;
    processingTime.end_time = endFileTime;
    fileProcessingTimes.push_back(processingTime);

    std::cout << "Text processed and saved to '" << outputFilename << "'." << std::endl;
}

void writeToJSON(const std::string& filename)
{
    std::string outputFilename = "files/out/" + filename; // Add the directory path
    json jsonData;

    json programTimes;
    programTimes["start_program_time"] = startTime;
    programTimes["end_program_time"] = endTime;

    jsonData["program_times"] = programTimes;

    // Create a JSON array to hold file data
    json filesArray = json::array();

    for (const auto& processingTime : fileProcessingTimes)
    {
        json fileData;
        fileData["filename"] = processingTime.filename;
        fileData["start_time"] = processingTime.start_time;
        fileData["end_time"] = processingTime.end_time;

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
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    startTime = clock();

    if (argc < 2)
    {
        std::cerr << "Usage: " << argv[0] << " <filename1> [<filename2> ... <filenameN>]" << std::endl;
        return 1;
    }

    for (int i = 1; i < argc; ++i)
    {
        std::string inputFilename = std::string("files/in/") + argv[i];
        if (fileNamesSet.find(inputFilename) != fileNamesSet.end())
        {
            std::cout << "File '" << inputFilename << "' already processed. Skipping." << std::endl;
            continue;
        }
        fileNamesSet.insert(inputFilename);

        std::string outputFilename = std::string("files/out/") + "out_" + argv[i];

        processFile(inputFilename, outputFilename);
    }

    endTime = clock();

    writeToJSON("file_processing_times.json");

    return 0;
}
