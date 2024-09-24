#include <iostream>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <string>
#include <iomanip>

int R1 = 0;
int R2 = 0;
clock_t delayTime;
HANDLE hThread;
DWORD IDThread;

struct LogJournal
{
    int operationNumber;
    int pressedKeyCode;
    DWORD ThreadID;
    clock_t operationStartTime;
    clock_t operationEndTime;
    int R1; 
    int R1c;
    int R2;
    int R2c;
} myLogger[1000];

void delayDuration(clock_t delayTime)
{
    clock_t tn;
    tn = clock();
    while (clock() - tn < delayTime);
}

bool checkForKeyPress()
{
    if (GetAsyncKeyState('1') & 0x8000)
    {
        return true;
    }
    else if (GetAsyncKeyState('2') & 0x8000)
    {
        return true;
    }
    return false;
}

int getCurrentKeyCode()
{
    if (GetAsyncKeyState('1') & 0x8000)
    {
        while (GetAsyncKeyState('1') & 0x8000)
        {
            Sleep(50);
        }
        return '1';
    }
    else if (GetAsyncKeyState('2') & 0x8000)
    {
        while (GetAsyncKeyState('2') & 0x8000)
        {
            Sleep(50);
        }
        return '2';
    }

    return 0;
}


LPTHREAD_START_ROUTINE ThreadProc(LPVOID lpParam)
{
    LogJournal *p = (LogJournal *)lpParam;

    int R1buf = R1;
    int R2buf = R2;

    delayDuration(delayTime);
    if (p->pressedKeyCode == 49) R1buf++; 
    else if (p->pressedKeyCode == 50) R2buf++; 
    delayDuration(delayTime);

    R1 = R1buf;
    R2 = R2buf;
    p->R1 = R1;
    p->R2 = R2;
    p->operationEndTime = clock();

    return 0;
}

void displayLogJournalHeader()
{
    std::cout << std::setw(5) << "No" << " | "
              << std::setw(4) << "Key" << " | "
              << std::setw(10) << "Thread ID" << " | "
              << std::setw(11) << "Start Time" << " | "
              << std::setw(9) << "End Time" << " | "
              << std::setw(6) << "R1" << " | "
              << std::setw(6) << "R1c" << " | "
              << std::setw(6) << "R2" << " | "
              << std::setw(6) << "R2c" << std::endl;
    std::cout << "----------------------------------------------------------------------------------------" << std::endl;
}

void displayLogJournalEntry(const LogJournal &entry)
{
    std::cout << std::setw(5) << entry.operationNumber << " | "
              << std::setw(4) << entry.pressedKeyCode << " | "
              << std::setw(10) << entry.ThreadID << " | "
              << std::setw(11) << entry.operationStartTime << " | "
              << std::setw(9) << entry.operationEndTime << " | "
              << std::setw(6) << entry.R1 << " | "
              << std::setw(6) << entry.R1c << " | "
              << std::setw(6) << entry.R2 << " | "
              << std::setw(6) << entry.R2c << std::endl;
}

int main(int argc, char *argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    if (argc != 2)
    {
        printf("Usage: %s <delay time in ms>\n", argv[0]);
        return 1;
    }


    delayTime = atoi(argv[1]);

    int i = 0;
    int j = 1;
    int exitFlag = 1;
    int R1contr;
    int R2contr;
    myLogger[1].operationEndTime = 0;
    R1contr = R1;
    R2contr = R2;

    displayLogJournalHeader();

    while (exitFlag && i < 1000)
    {
        bool neededButtonsPressed = checkForKeyPress();

        if (neededButtonsPressed)
        {
            int currentKeyCode = getCurrentKeyCode();

            i++;
            myLogger[i].operationNumber = i;
            myLogger[i].pressedKeyCode = currentKeyCode;
            myLogger[i].operationEndTime = 0;
            myLogger[i].R1 = R1;
            myLogger[i].R2 = R2;
            myLogger[i].operationStartTime = clock();

            hThread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(ThreadProc), &myLogger[i], CREATE_SUSPENDED, &IDThread);
            
            if (currentKeyCode == 49) R1contr++;
            if (currentKeyCode == 50) R2contr++;


            if (hThread == NULL)
            {
                std::cerr << "Error: Unable to create thread." << std::endl;
                return 1;
            }

            myLogger[i].pressedKeyCode = currentKeyCode;
            myLogger[i].R1c = R1contr;
            myLogger[i].R2c = R2contr;
            myLogger[i].ThreadID = IDThread;
            ResumeThread(hThread);
        }

        if (myLogger[j].operationEndTime != 0)
        {
            displayLogJournalEntry(myLogger[j]);
            j++;
        }
    }

    return 0;
}