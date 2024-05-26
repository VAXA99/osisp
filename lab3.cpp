#include <iostream>
#include <conio.h>
#include <windows.h>
#include <ctime>
#include <string>
#include <iomanip>

// Глобальные переменные
std::string R = "**";
clock_t delayTime;
HANDLE hThread;
DWORD IDThread;

CRITICAL_SECTION cs;

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
    std::string R;  // Разделяемая переменная
    std::string Rc; // Результат операции в главном потоке
} myLogger[1000];

void delayDuration(clock_t delayTime)
{
    clock_t tn;
    tn = clock();
    while (clock() - tn < delayTime)
        ;
}

bool checkForKeyPress()
{
    for (int i = 1; i <= 9; ++i)
    {
        if ((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState(VK_F1 + (i - 1)) & 0x8000))
        {
            return true;
        }
        else if (GetAsyncKeyState('1' + (i - 1)) & 0x8000)
        {
            return true;
        }
    }
    return false;
}

int getCurrentKeyCode()
{
    int currentKey = 0;

    for (int i = 1; i <= 9; ++i)
    {
        if ((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState(VK_F1 + (i - 1)) & 0x8000))
        {
            currentKey = VK_F1 + (i - 1);

            while ((GetAsyncKeyState(VK_MENU) & 0x8000) && (GetAsyncKeyState(VK_F1 + (i - 1)) & 0x8000))
            {
                Sleep(50);
            }
        }
        else if (GetAsyncKeyState('1' + (i - 1)) & 0x8000)
        {
            currentKey = '1' + (i - 1);
            while (GetAsyncKeyState('1' + (i - 1)) & 0x8000)
            {
                Sleep(50);
            }
        }
    }

    return currentKey;
}

int calculateAdjustmentSymbolsCount(int keyCode)
{
    if (keyCode >= '1' && keyCode <= '9')
    {
        return keyCode - '0';
    }
    else if (keyCode >= VK_F1 && keyCode <= VK_F9)
    {
        return keyCode - VK_F1 + 1;
    }
    return 0;
}

std::string addSymbols(const std::string &stringVariable, int adjustmentSymbolsCount)
{
    return stringVariable + std::string(adjustmentSymbolsCount, '*');
}

std::string eraseSymbols(const std::string &stringVariable, int adjustmentSymbolsCount)
{
    if (stringVariable.length() > adjustmentSymbolsCount)
    {
        return stringVariable.substr(0, stringVariable.length() - adjustmentSymbolsCount);
    }
    return "";
}

LPTHREAD_START_ROUTINE FirstOperationThreadProc(LPVOID lpParam)
{
    EnterCriticalSection(&cs);
    LogJournal *p = (LogJournal *)lpParam;
    p->enteringCriticalSectionTime = clock();
    std::string Rbuf;
    int currentKeyCode = p->pressedKeyCode;

    Rbuf = R;

    int adjustmentSymbolsCount = calculateAdjustmentSymbolsCount(currentKeyCode);

    delayDuration(delayTime);

    Rbuf = addSymbols(Rbuf, adjustmentSymbolsCount);

    delayDuration(delayTime);

    R = Rbuf;
    p->R = R;
    p->operationEndTime = clock();
    p->waitingTime = p->operationEndTime - p->operationStartTime;
    p->leavingCriticalSectionTime = clock();
    LeaveCriticalSection(&cs);

    return 0;
}

LPTHREAD_START_ROUTINE SecondOperationThreadProc(LPVOID lpParam)
{
    EnterCriticalSection(&cs);
    LogJournal *p = (LogJournal *)lpParam;
    p->enteringCriticalSectionTime = clock();
    std::string Rbuf;
    int currentKeyCode = p->pressedKeyCode;

    Rbuf = R;

    int adjustmentSymbolsCount = calculateAdjustmentSymbolsCount(currentKeyCode);

    delayDuration(delayTime);

    Rbuf = eraseSymbols(Rbuf, adjustmentSymbolsCount);

    delayDuration(delayTime);

    R = Rbuf;
    p->R = R;
    p->operationEndTime = clock();
    p->waitingTime = p->operationEndTime - p->operationStartTime;
    p->leavingCriticalSectionTime = clock();
    LeaveCriticalSection(&cs);

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
        printf("Использование: %s <время задержки>\n", argv[0]);
        return 1;
    }

    InitializeCriticalSection(&cs);

    delayTime = atoi(argv[1]);

    int i = 0;
    int j = 1;
    int exitFlag = 1;
    std::string Rcontr;
    myLogger[1].operationEndTime = 0;
    Rcontr = R;

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
            myLogger[i].R = "";
            myLogger[i].operationStartTime = clock();

            if (currentKeyCode < 58)
            {
                int adjustmentSymbolsCount = calculateAdjustmentSymbolsCount(currentKeyCode);

                hThread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(FirstOperationThreadProc), &myLogger[i], CREATE_SUSPENDED, &IDThread);
                Rcontr = addSymbols(Rcontr, adjustmentSymbolsCount);
            }
            else
            {
                int adjustmentSymbolsCount = calculateAdjustmentSymbolsCount(currentKeyCode);

                hThread = CreateThread(NULL, 0, LPTHREAD_START_ROUTINE(SecondOperationThreadProc), &myLogger[i], CREATE_SUSPENDED, &IDThread);
                Rcontr = eraseSymbols(Rcontr, adjustmentSymbolsCount);
            }

            if (hThread == NULL)
            {
                std::cerr << "Error: Unable to create thread." << std::endl;
                DeleteCriticalSection(&cs);
                return 1;
            }

            myLogger[i].pressedKeyCode = currentKeyCode;
            myLogger[i].Rc = Rcontr;
            myLogger[i].ThreadID = IDThread;
            ResumeThread(hThread);
        }

        if (myLogger[j].operationEndTime != 0)
        {
            displayLogJournalEntry(myLogger[j]);
            j++;
        }
    }
    DeleteCriticalSection(&cs);

    return 0;
}