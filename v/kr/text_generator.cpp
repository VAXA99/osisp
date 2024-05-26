#include <iostream>
#include <fstream>
#include <string>
#include <cstdlib>
#include <ctime>

const int NUM_FILES = 10;            
const int TARGET_FILE_SIZE = 1000000; 

std::string generateText()
{
    std::string vowels = "aeiouAEIOU";                                     
    std::string consonants = "bcdfghjklmnpqrstvwxyzBCDFGHJKLMNPQRSTVWXYZ";

    std::string text;
    int fileSize = 0;

    while (fileSize < TARGET_FILE_SIZE)
    {
        int length = rand() % 50 + 10; 

        for (int i = 0; i < length; ++i)
        {
            if (i == 0 || (i > 0 && text[i - 1] == ' '))
            {
                text += consonants[rand() % consonants.length()];
            }
            else
            {
                text += vowels[rand() % vowels.length()];
            }
        }

        fileSize += length;
    }

    return text;
}

int main()
{
    srand(time(nullptr)); 

    std::string directory = "files/in/"; 

    for (int i = 1; i <= NUM_FILES; ++i)
    {
        std::string filename = directory + std::to_string(i) + ".txt";
        std::ofstream file(filename);

        if (!file.is_open())
        {
            std::cerr << "Error: Unable to open file " << filename << std::endl;
            return 1;
        }

        std::string text = generateText();
        file << text;

        file.close();
        std::cout << "File " << filename << " created and filled with text." << std::endl;
    }

    return 0;
}
