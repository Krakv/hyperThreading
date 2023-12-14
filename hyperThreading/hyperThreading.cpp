#include <iostream>
#include <fstream>
#include <Windows.h>
#include <mutex>
#include <thread>
#include <synchapi.h>
using namespace System::Threading;
static int perem = 100;

const int arraySize = 10;
static int array[arraySize];
static bool inf[10] = {true, true, true, true,  true, true,  true, true,  true, true };
static bool s = false;
static int sticks = 5;
static std::timed_mutex mutex;
HANDLE hSemSpace;
HANDLE hSemData;
HANDLE hSemReader;
HANDLE hSemWriter;
HANDLE hSemWriterFile;
int indexWriter = 0;
int indexReader = 0;

public ref class Example {
public:
    static void Writer() {
        
        std::cout << "Write: ";
        std::cin >> perem;
    }
    static void Reader() {
        std::cout << "perem: " << perem << std::endl;
    }
};

public ref class ExampleSecond {
public:
    static void Writer() {
        for (int i = 0; i < 10; i++) {
            while (!inf[i]) {
                Thread::Sleep(300);
            }
            s = true;
            array[i] = i * 123 / 31;
            s = false;
            inf[i] = false;
        }
    }
    static void Reader() {
        for (int i = 0; i < 10; i++) {
            while (s || inf[i]) {
                Thread::Sleep(300);
            }
            std::cout << i << ": " << array[i] << std::endl;
            inf[i] = true;
        }
    }
};

public ref class ExampleThinker {
public:
    static void Thinker() {
        while (!Take());
        std::cout << "Working...\n";
        Thread::Sleep(1000);
        sticks += 2;
        std::cout << "Done\n";
    }

    static bool Take() {
        bool res = false;
        while (!mutex.try_lock());
        if (sticks >= 2) {
            sticks -= 1;
            sticks -= 1;
            res++;
        }
        mutex.unlock();
        return res;
    }
};

public ref class ExampleFourth {
public:
    static void Writer1(char name) {
        while (indexWriter < 20) {
            HANDLE h[] = { hSemSpace, hSemWriter };
            WaitForMultipleObjects(2, h, true, INFINITE);
            array[indexWriter % arraySize] = indexWriter * 121 / 32;
            indexWriter++;
            ReleaseSemaphore(hSemData, 1, NULL);
            ReleaseSemaphore(hSemWriter, 1, NULL);
        } 
    }
    static void Writer2(char name) {
        while (indexWriter < 20) {
            HANDLE h[] = { hSemSpace, hSemWriter };
            WaitForMultipleObjects(2, h, true, INFINITE);
            array[indexWriter % arraySize] = indexWriter * 121 / 32;
            indexWriter++;
            ReleaseSemaphore(hSemData, 1, NULL);
            ReleaseSemaphore(hSemWriter, 1, NULL);
        }
    }
    static void Reader1(char name) {
        while (indexReader < 20) {
            HANDLE h[] = { hSemData, hSemReader };
            WaitForMultipleObjects(2, h, true, INFINITE);
            std::cout << name << " : " << indexReader << ": " << array[indexReader % arraySize] << std::endl;
            indexReader++;
            ReleaseSemaphore(hSemSpace, 1, NULL);
            ReleaseSemaphore(hSemReader, 1, NULL);
        }
    }
    static void Reader2(char name) {
        while (indexReader < 20) {
            HANDLE h[] = { hSemData, hSemReader };
            WaitForMultipleObjects(2, h, true, INFINITE);
            std::cout << name << " : " << indexReader << ": " << array[indexReader % arraySize] << std::endl;
            indexReader++;
            ReleaseSemaphore(hSemSpace, 1, NULL);
            ReleaseSemaphore(hSemReader, 1, NULL);
        }
    }
};

public ref class ExampleFifth {
public:
    static void Writer1(char name) {
        while (indexWriter < 20) {
            HANDLE h[] = { hSemSpace, hSemWriter };
            WaitForMultipleObjects(2, h, true, INFINITE);
            array[indexWriter % arraySize] = indexWriter * 121 / 32;
            indexWriter++;
            if ((indexWriter % arraySize == 0 && indexWriter / arraySize >= 1))
                ReleaseSemaphore(hSemWriterFile, 1, NULL);
            else
                ReleaseSemaphore(hSemWriter, 1, NULL);
            ReleaseSemaphore(hSemData, 1, NULL);
        } 
    }
    static void Writer2(char name) {
        while (indexWriter < 20) {
            HANDLE h[] = { hSemSpace, hSemWriter };
            WaitForMultipleObjects(2, h, true, INFINITE);
            array[indexWriter % arraySize] = indexWriter * 121 / 32;
            indexWriter++;
            if ((indexWriter % arraySize == 0 && indexWriter / arraySize >= 1))
                ReleaseSemaphore(hSemWriterFile, 1, NULL);
            else
                ReleaseSemaphore(hSemWriter, 1, NULL);
            ReleaseSemaphore(hSemData, 1, NULL);
        }
    }
    static void Reader1(char name) {
        while (indexReader < 20) {
            HANDLE h[] = { hSemData, hSemReader };
            WaitForMultipleObjects(2, h, true, INFINITE);
            std::cout << name << " : " << indexReader << " : " << array[indexReader % arraySize] << std::endl;
            indexReader++;
            ReleaseSemaphore(hSemSpace, 1, NULL);
            ReleaseSemaphore(hSemReader, 1, NULL);
        }
    }
    static void Reader2(char name) {
        while (indexReader < 20) {
            HANDLE h[] = { hSemData, hSemReader };
            WaitForMultipleObjects(2, h, true, INFINITE);
            std::cout << name << " : " << indexReader << " : " << array[indexReader % arraySize] << std::endl;
            indexReader++;
            ReleaseSemaphore(hSemSpace, 1, NULL);
            ReleaseSemaphore(hSemReader, 1, NULL);
        }
    }
    static void WriteFile() {
        std::ofstream of;
        of.open("journal.txt", std::ios::out);
        of.close();
        while (indexReader < 20) {
            DWORD word = WaitForSingleObject(hSemWriterFile, 200);
            if (word != WAIT_TIMEOUT) {
                std::ofstream of;
                of.open("journal.txt", std::ios::app);
                for (int i = 0; i < arraySize; i++) {
                    of << array[i] << std::endl;
                }
                of.close();
            }
            ReleaseSemaphore(hSemWriter, 1, NULL);
        }
    }
};

void First() {
    while (perem != 0) {
        Thread^ oThread = gcnew Thread(gcnew ThreadStart(&Example::Writer));
        Thread^ iThread = gcnew Thread(gcnew ThreadStart(&Example::Reader));
        oThread->Start();
        oThread->Join();
        iThread->Start();
        iThread->Join();
    }
}

void Second() {
    Thread^ oThread = gcnew Thread(gcnew ThreadStart(&ExampleSecond::Writer));
    Thread^ iThread = gcnew Thread(gcnew ThreadStart(&ExampleSecond::Reader));
    oThread->Start();
    iThread->Start();
}

void Third() {
    Thread^ thread1 = gcnew Thread(gcnew ThreadStart(&ExampleThinker::Thinker));
    Thread^ thread2 = gcnew Thread(gcnew ThreadStart(&ExampleThinker::Thinker));
    Thread^ thread3 = gcnew Thread(gcnew ThreadStart(&ExampleThinker::Thinker));
    Thread^ thread4 = gcnew Thread(gcnew ThreadStart(&ExampleThinker::Thinker));
    Thread^ thread5 = gcnew Thread(gcnew ThreadStart(&ExampleThinker::Thinker));
    thread1->Start();
    thread2->Start();
    thread3->Start();
    thread4->Start();
    thread5->Start();
    
}

void Fourth() {
    hSemSpace = CreateSemaphore(NULL, arraySize, arraySize, NULL);
    hSemData = CreateSemaphore(NULL, 0, arraySize, NULL);
    hSemReader = CreateSemaphore(NULL, 1, 2, NULL);
    hSemWriter = CreateSemaphore(NULL, 1, 2, NULL);
    std::thread oT1(ExampleFourth::Writer1, 'A');
    std::thread oT2(ExampleFourth::Writer2, 'B');
    std::thread iT1(ExampleFourth::Reader1, 'A');
    std::thread iT2(ExampleFourth::Reader2, 'B');

    oT1.join();
    oT2.join();
    iT1.join();
    iT2.join();
}

void Fifth() {
    hSemSpace = CreateSemaphore(NULL, arraySize, arraySize, NULL);
    hSemData = CreateSemaphore(NULL, 0, arraySize, NULL);
    hSemReader = CreateSemaphore(NULL, 1, 2, NULL);
    hSemWriter = CreateSemaphore(NULL, 1, 2, NULL);
    hSemWriterFile = CreateSemaphore(NULL, 0, 2, NULL);
    std::thread oT1(ExampleFifth::Writer1, 'A');
    std::thread oT2(ExampleFifth::Writer2, 'B');
    std::thread iT1(ExampleFifth::Reader1, 'A');
    std::thread iT2(ExampleFifth::Reader2, 'B');
    std::thread iT3(ExampleFifth::WriteFile);

    oT1.join();
    oT2.join();
    iT1.join();
    iT2.join();
    iT3.join();
}

int main()
{
    //First();
    //Second();
    //Third();
    //Fourth();
    Fifth();
}