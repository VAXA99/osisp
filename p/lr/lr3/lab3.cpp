#include <iostream>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <string>
#include <iomanip>

std::string R = "ABCDE";
clock_t delayTime;
HANDLE hThread;
HANDLE semaphore;
DWORD threadID;

struct LogJournal
{
    int operationNumber;
    int pressedKeyCode;
    DWORD ThreadID;
    clock_t enteringCriticalSectionTime;
    clock_t leavingCriticalSectionTime;
    clock_t operationStartTime;
    clock_t operationEndTime;
    clock_t waitingTime;
    std::string R;
    std::string Rc;
} logJournal[1000];

void delay(clock_t delayTime)
{
    clock_t start = clock();
    while (clock() - start < delayTime)
        ;
}

bool isKeyPressed()
{
    return (GetAsyncKeyState(VK_LEFT) & 0x8000) || (GetAsyncKeyState(VK_RIGHT) & 0x8000);
}

int getKeyCode()
{
    int currentKey = 0;

    if (GetAsyncKeyState(VK_LEFT) & 0x8000)
    {
        currentKey = VK_LEFT;

        while (GetAsyncKeyState(VK_LEFT) & 0x8000)
        {
            Sleep(50);
        }
    }
    else if (GetAsyncKeyState(VK_RIGHT) & 0x8000)
    {
        currentKey = VK_RIGHT;
        while (GetAsyncKeyState(VK_RIGHT) & 0x8000)
        {
            Sleep(50);
        }
    }

    return currentKey;
}

std::string shiftString(const std::string &str, bool isLeft)
{
    if (str.empty())
        return str;

    if (isLeft)
    {
        return str.substr(1) + str[0];
    }
    else
    {
        return str.back() + str.substr(0, str.size() - 1);
    }
}

DWORD WINAPI operationThreadProc(LPVOID param)
{
    WaitForSingleObject(semaphore, INFINITE);

    LogJournal *logEntry = static_cast<LogJournal *>(param);
    logEntry->enteringCriticalSectionTime = clock();
    int currentKeyCode = logEntry->pressedKeyCode;
    bool isLeft = (currentKeyCode == VK_LEFT);

    std::string buffer = R;

    delay(delayTime);
    buffer = shiftString(buffer, isLeft);
    delay(delayTime);

    R = buffer;

    logEntry->R = R;
    logEntry->operationEndTime = clock();
    logEntry->waitingTime = logEntry->operationEndTime - logEntry->operationStartTime;
    logEntry->leavingCriticalSectionTime = clock();

    ReleaseSemaphore(semaphore, 1, NULL);

    return 0;
}

void displayLogJournalHeader()
{
    std::cout << std::setw(5) << "No" << " | "
              << std::setw(4) << "Key" << " | "
              << std::setw(10) << "Thread ID" << " | "
              << std::setw(11) << "Start Time" << " | "
              << std::setw(9) << "End Time" << " | "
              << std::setw(10) << "Enter CS" << " | "
              << std::setw(10) << "Leave CS" << " | "
              << std::setw(10) << "Wait Time" << " | "
              << std::setw(7) << "R" << " | "
              << std::setw(7) << "Rc" << std::endl;
    std::cout << "--------------------------------------------------------------------------------------------------------------------" << std::endl;
}

void displayLogJournalEntry(const LogJournal &entry)
{
    std::cout << std::setw(5) << entry.operationNumber << " | "
              << std::setw(4) << entry.pressedKeyCode << " | "
              << std::setw(10) << entry.ThreadID << " | "
              << std::setw(11) << entry.operationStartTime << " | "
              << std::setw(9) << entry.operationEndTime << " | "
              << std::setw(10) << entry.enteringCriticalSectionTime << " | "
              << std::setw(10) << entry.leavingCriticalSectionTime << " | "
              << std::setw(10) << entry.waitingTime << " | "
              << std::setw(7) << entry.R << " | "
              << std::setw(7) << entry.Rc << std::endl;
}

int main(int argc, char *argv[])
{
    SetConsoleCP(1251);
    SetConsoleOutputCP(1251);

    if (argc != 2)
    {
        printf("Работа программы: %s <время задержки>\n", argv[0]);
        return 1;
    }

    delayTime = atoi(argv[1]);
    semaphore = CreateSemaphore(NULL, 1, 1, NULL);

    int operationIndex = 0;
    int displayIndex = 1;
    bool running = true;
    std::string controlString = R;
    logJournal[1].operationEndTime = 0;

    displayLogJournalHeader();

    while (running && operationIndex < 1000)
    {
        if (isKeyPressed())
        {
            int currentKeyCode = getKeyCode();

            operationIndex++;
            logJournal[operationIndex].operationNumber = operationIndex;
            logJournal[operationIndex].pressedKeyCode = currentKeyCode;
            logJournal[operationIndex].operationEndTime = 0;
            logJournal[operationIndex].R = "";
            logJournal[operationIndex].operationStartTime = clock();

            hThread = CreateThread(NULL, 0, operationThreadProc, &logJournal[operationIndex], CREATE_SUSPENDED, &threadID);
            bool isLeft = (currentKeyCode == VK_LEFT);
            controlString = shiftString(controlString, isLeft);

            logJournal[operationIndex].ThreadID = threadID;
            logJournal[operationIndex].Rc = controlString;
            ResumeThread(hThread);
        }

        if (logJournal[displayIndex].operationEndTime != 0)
        {
            displayLogJournalEntry(logJournal[displayIndex]);
            displayIndex++;
        }
    }

    CloseHandle(semaphore);
    return 0;
}
