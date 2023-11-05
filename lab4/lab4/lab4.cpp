#include <windows.h>
#include <commctrl.h>
#include <sstream>
#include <TlHelp32.h>
#include <psapi.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>

#define IDC_SHOW 2001
#define IDC_PROCESS_LIST 2002
#define IDC_CLOSE_PROCESS 2003
#define IDC_SPAWN 2008
#define IDC_HIGH_PRIORITY 2009
#define IDC_COMMON_PRIORITY 2010
#define IDC_LOW_PRIORITY 2011

HANDLE hProcess = NULL; // дискриптер процесса
#pragma comment(lib, "comctl32.lib")
HWND hListBox; // Дескриптор элемента управления ListBox

void Update();
void Spawn();
void CloseProcess();
void SetHignPriority();
void SetCommonPriority();
void SetLowPriority();

struct ProcessInfo {
    int processId;
    std::wstring name;
    SIZE_T memoryUsage;
    ProcessInfo(int id, const std::wstring& n, SIZE_T mem) : processId(id), name(n), memoryUsage(mem) {}
};

std::vector<ProcessInfo> processInfoList;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    static MEMORYSTATUS memoryStatus; // структура для хранения информации о системной памяти
    static HWND hProgressBar;
 
    switch (message)
    {
    case WM_CREATE:
        CreateWindow(L"BUTTON", L"Показать процессы", WS_VISIBLE | WS_CHILD, 20, 20, 220, 30, hWnd, (HMENU)IDC_SHOW, NULL, NULL);
        CreateWindow(L"BUTTON", L"Закрыть процесс", WS_VISIBLE | WS_CHILD, 20, 70, 220, 30, hWnd, (HMENU)IDC_CLOSE_PROCESS, NULL, NULL);
        CreateWindow(L"BUTTON", L"Порождение", WS_VISIBLE | WS_CHILD, 20, 120, 220, 30, hWnd, (HMENU)IDC_SPAWN, NULL, NULL);
        CreateWindow(L"BUTTON", L"Установить высокий приоритет", WS_VISIBLE | WS_CHILD, 20, 170, 220, 30, hWnd, (HMENU)IDC_HIGH_PRIORITY, NULL, NULL);
        CreateWindow(L"BUTTON", L"Установить общий приоритет", WS_VISIBLE | WS_CHILD, 20, 220, 220, 30, hWnd, (HMENU)IDC_COMMON_PRIORITY, NULL, NULL);
        CreateWindow(L"BUTTON", L"Установить низкий приоритет", WS_VISIBLE | WS_CHILD, 20, 270, 220, 30, hWnd, (HMENU)IDC_LOW_PRIORITY, NULL, NULL);
        
        hListBox = CreateWindow(L"LISTBOX", L"", WS_VISIBLE | WS_CHILD | LBS_DISABLENOSCROLL | WS_VSCROLL | WS_HSCROLL | WS_BORDER ,
            340, 10, 490, 480, hWnd, (HMENU)IDC_PROCESS_LIST, NULL, NULL);

        GlobalMemoryStatus(&memoryStatus); //получения информации о системной памяти и сохранение ее в структуру 
        SetTimer(hWnd, 1, 1000, NULL); // таймер для обновления каждую секунду
        InitCommonControls(); // инициализация библиотеки для прогресс-бар 

        hProgressBar = CreateWindowEx(0, PROGRESS_CLASS, NULL, WS_CHILD | WS_VISIBLE | PBS_SMOOTH,
            10, 410, 300, 30, hWnd, NULL, NULL, NULL);

        SendMessage(hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, 100)); // диапазон прогресс-бара (0-100)
        break;
    case WM_COMMAND:
    {
        int wmId = LOWORD(wParam);
        int wmEvent = HIWORD(wParam);

        if (wmId == IDC_SHOW && wmEvent == BN_CLICKED) {
            Update();
        }
        if (wmId == IDC_CLOSE_PROCESS && wmEvent == BN_CLICKED) {
            CloseProcess();
        }
        else if (wmId == IDC_SPAWN && wmEvent == BN_CLICKED) {
            Spawn();
        }
        else if (wmId == IDC_HIGH_PRIORITY && wmEvent == BN_CLICKED) {
            SetHignPriority();
        }
        else if (wmId == IDC_COMMON_PRIORITY && wmEvent == BN_CLICKED) {
            SetCommonPriority();
        }
        else if (wmId == IDC_LOW_PRIORITY && wmEvent == BN_CLICKED) {
            SetLowPriority();
        }
    }
    break;
    case WM_TIMER:
        GlobalMemoryStatus(&memoryStatus); //  обновляем информацию о системной памяти и сохраняем в структуре
        SendMessage(hProgressBar, PBM_SETPOS, memoryStatus.dwMemoryLoad, 0); // обновляем состояние прогресс-бара
        InvalidateRect(hWnd, NULL, TRUE); // Обновляем окно
        break;
    case WM_PAINT:
    {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hWnd, &ps);
        std::wstringstream ss; // строковый поток
        ss << L"Общая память: " << memoryStatus.dwTotalPhys / (1024 * 1024) << L" MB\n";
        // Рисуем "Общая память"
        TextOut(hdc, 10, 330, ss.str().c_str(), ss.str().length());
        // Вычисляем высоту "Общая память" текста и добавляем отступ
        int textHeight = DrawText(hdc, ss.str().c_str(), -1, &ps.rcPaint, DT_CALCRECT);
        int yOffset = textHeight + 10; // смещение, чтобы создать отступ
        // Отображаем "Доступная память" под "Общая память"
        ss.str(L""); // Очищаем stringstream
        ss << L"Доступная память: " << memoryStatus.dwAvailPhys / (1024 * 1024) << L" MB\n";
        TextOut(hdc, 10, 330 + yOffset, ss.str().c_str(), ss.str().length());
        
    }
    break;
    case WM_CLOSE:
        if (MessageBox(hWnd, L"Вы действительно хотите закрыть окно?", L"Выход", MB_OKCANCEL) == IDOK) {
            KillTimer(hWnd, 1); // Уничтожаем таймер перед выходом
            DestroyWindow(hWnd);
        }
        return 0;
        break;
    case WM_DESTROY:
        if (hProcess) {
            TerminateProcess(hProcess, 0); // Завершаем процесс, если он был создан
            CloseHandle(hProcess); // Закрываем дескриптор процесса
        }
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);
    const wchar_t* className = L"MyMemoryMonitorClass";
    WNDCLASSEX wcex;
    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = WndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = hInstance;
    wcex.hIcon = LoadIcon(hInstance, IDI_APPLICATION);
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW +1);
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = className;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);
    if (!RegisterClassEx(&wcex))
    {
        MessageBox(NULL, L"Cannot register class", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }

    HWND hWnd = CreateWindow(className, L"Диспетчер задач", WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT,
        880, 550, NULL, NULL, hInstance, NULL);
    
    if (!hWnd)
    {
        MessageBox(NULL, L"Cannot create window", L"Error", MB_OK | MB_ICONERROR);
        return 1;
    }
    ShowWindow(hWnd, nCmdShow);
    UpdateWindow(hWnd);
    MSG msg;
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    return (int)msg.wParam;
}


void Update()
{
    SendMessage(hListBox, LB_RESETCONTENT, 0, 0); // Очищаем список процессов
    HANDLE hProcessSnap; //снимок процессов (моментальное состояние)
    PROCESSENTRY32 pe32; // структура для хранения инфы о процессах
    // Создаем снимок всех процессов в системе
    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE)
    {
        MessageBox(NULL, L"Не удалось создать снимок процесса", L"Ошибка", MB_ICONERROR);
        return;
    }
    pe32.dwSize = sizeof(PROCESSENTRY32);
    // Получаем информацию о первом процессе
    if (!Process32First(hProcessSnap, &pe32))
    {
        CloseHandle(hProcessSnap);
        MessageBox(NULL, L"Не удалось получить информацию о процессе", L"Ошибка", MB_ICONERROR);
        return;
    }
    // Вектор для хранения информации о процессах
    std::vector<ProcessInfo> processInfoList;
    // Перебираем процессы и добавляем их в вектор
    do
    {
        // PROCESS_QUERY_INFORMATION флаг на права получения информации о процессе
        HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION, FALSE, pe32.th32ProcessID);
        if (hProcess)
        {
            PROCESS_MEMORY_COUNTERS_EX pmc; // структура  для хранения информации о памяти процесса
            if (GetProcessMemoryInfo(hProcess, (PROCESS_MEMORY_COUNTERS*)&pmc, sizeof(pmc)))
            {
                ProcessInfo processInfo(pe32.th32ProcessID, pe32.szExeFile, pmc.WorkingSetSize);
                processInfoList.push_back(processInfo);
            }
            CloseHandle(hProcess); // закрытие дескриптора процесса после получения информации о нем.
        }
    } while (Process32Next(hProcessSnap, &pe32));

    CloseHandle(hProcessSnap);
    // Сортируем вектор по количеству занимаемой памяти
    std::sort(processInfoList.begin(), processInfoList.end(), [](const ProcessInfo& a, const ProcessInfo& b) {
        return a.memoryUsage > b.memoryUsage;
        });
    // Заполняем ListBox отсортированными данными
    for (const ProcessInfo& process : processInfoList) {
        std::wstring processInfo = L"ID: " + std::to_wstring(process.processId) + L", Name: " + process.name + L", Memory: " + std::to_wstring(process.memoryUsage / (1024 * 1024)) + L" MB";
        SendMessage(hListBox, LB_ADDSTRING, 0, (LPARAM)processInfo.c_str());
    }
}

void CloseProcess()
{
    int selectedIndex = SendMessage(hListBox, LB_GETCURSEL, 0, 0); // Получаем выделенный элемент в списке
    if (selectedIndex != LB_ERR) {
        // Получаем длину строки
        int textLength = SendMessage(hListBox, LB_GETTEXTLEN, selectedIndex, 0);
        if (textLength != LB_ERR) {
            // Выделяем буфер для текста
            wchar_t* buffer = new wchar_t[textLength + 1];
            if (SendMessage(hListBox, LB_GETTEXT, selectedIndex, (LPARAM)buffer) != LB_ERR) { // получаем текст и сохраняем его в буфер

                std::wstring text(buffer); // Текст из буфера конвертируется в строку типа wstring

                // Разбиваем текст на части, чтобы получить ID процесса
                std::wistringstream parser(text);
                std::wstring idStr, name;
                parser >> idStr; // "ID:"
                parser >> idStr; // Собственно ID
                int processId = _wtoi(idStr.c_str()); // конвертируется в int

                // Завершаем процесс
                // PROCESS_TERMINATE - права на завершения процесса
                HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, processId);
                if (hProcess != NULL) {
                    if (TerminateProcess(hProcess, 0)) {
                        MessageBox(NULL, L"Процесс завершен успешно", L"Успешно", MB_ICONINFORMATION);
                    }
                    else {
                        MessageBox(NULL, L"Не удалось завершить процесс", L"Ошибка", MB_ICONERROR);
                    }
                    CloseHandle(hProcess);
                }
                else {
                    MessageBox(NULL, L"Не удалось открыть процесс для завершения", L"Ошибка", MB_ICONERROR);
                }
            }
            delete[] buffer;
        }
    }
    else {
        MessageBox(NULL, L"Пожалуйста, выберите процесс для завершения", L"Ошибка", MB_ICONWARNING);
    }
    Update();
}

void Spawn()
{
    //структуры для создания нового процесса и сохранения информации о нем
    STARTUPINFO si;
    PROCESS_INFORMATION pi;
    ZeroMemory(&si, sizeof(si));
    si.cb = sizeof(si);
    ZeroMemory(&pi, sizeof(pi));
    const wchar_t* fileName = L"notepad.exe";  // исполняемый файл блокнота
    const wchar_t* filePath = L"D:\\osisp_lab4_example.txt";  // Путь к файлу, который вы хотите открыть
    // Составляем команду, включая путь к исполняемому файлу и путь к файлу для открытия
    const int bufferSize = MAX_PATH + MAX_PATH;
    wchar_t commandLine[bufferSize];
    swprintf(commandLine, bufferSize, L"%s %s", fileName, filePath);
    if (CreateProcess(NULL, commandLine, NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi))
    {
        CloseHandle(pi.hThread);
        CloseHandle(pi.hProcess);
    }
    else
    {
        MessageBox(NULL, L"Не удалось создать новый процесс.", L"Ошибка", MB_ICONERROR);
    }
}
// получение идентификатора процесса блокнота
DWORD GetNotepadProcessId()
{
    DWORD processId = 0;
    PROCESSENTRY32 pe32; //  структура для получения информации о процессах
    pe32.dwSize = sizeof(PROCESSENTRY32);
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPALL, 0); // снимок процессов 
    if (Process32First(hSnapshot, &pe32)) { 
        do {
            // сравниваем имя исполняемого файла процесса
            if (lstrcmpi(pe32.szExeFile, L"notepad.exe") == 0) {
                processId = pe32.th32ProcessID;
                break;
            }
        } while (Process32Next(hSnapshot, &pe32));
    }
    CloseHandle(hSnapshot);
    return processId;
}
void SetHignPriority()
{
    // Получение идентификатора процесса блокнота
    DWORD notepadProcessId = GetNotepadProcessId();
    // Установка высокого приоритета для блокнота
    // PROCESS_SET_INFORMATION - право на изменение информации
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, notepadProcessId);
    if (hProcess != NULL) {
        if (SetPriorityClass(hProcess, HIGH_PRIORITY_CLASS)) {
            MessageBox(NULL, L"Приоритет установлен на «Высокий» для Блокнота.", L"Успешно", MB_ICONINFORMATION);
        }
        else {
            MessageBox(NULL, L"Не удалось установить приоритет «Высокий» для Блокнота.", L"Ошибка", MB_ICONERROR);
        }
        CloseHandle(hProcess);
    }
    else
    {
        MessageBox(NULL, L"Откройте блокнот", L"Ошибка", MB_ICONERROR);
    }
}

void SetCommonPriority()
{
    // Получение идентификатора процесса блокнота
    DWORD notepadProcessId = GetNotepadProcessId();
    // Установка обычного приоритета для блокнота
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, notepadProcessId);
    if (hProcess != NULL) {
        if (SetPriorityClass(hProcess, NORMAL_PRIORITY_CLASS)) {
            MessageBox(NULL, L"Приоритет установлен на «Обычный» для Блокнота", L"Успешно", MB_ICONINFORMATION);
        }
        else {
            MessageBox(NULL, L"Не удалось установить приоритет «Обычный» для Блокнота.", L"Ошибка", MB_ICONERROR);
        }
        CloseHandle(hProcess);
    }
    else
    {
        MessageBox(NULL, L"Откройте блокнот", L"Ошибка", MB_ICONERROR);
    }
}

void SetLowPriority()
{
    // Получение идентификатора процесса блокнота
    DWORD notepadProcessId = GetNotepadProcessId();
    // Установка низкого приоритета для блокнота
    HANDLE hProcess = OpenProcess(PROCESS_SET_INFORMATION, FALSE, notepadProcessId);
    if (hProcess != NULL) {
        if (SetPriorityClass(hProcess, IDLE_PRIORITY_CLASS)) {
            MessageBox(NULL, L"Приоритет установлен на «Низкий» для Блокнота", L"Успешно", MB_ICONINFORMATION);
        }
        else {
            MessageBox(NULL, L"Не удалось установить приоритет «Низкий» для Блокнота.", L"Ошибка", MB_ICONERROR);
        }
        CloseHandle(hProcess);
    }
    else
    {
        MessageBox(NULL, L"Откройте блокнот", L"Ошибка", MB_ICONERROR);
    }
}
