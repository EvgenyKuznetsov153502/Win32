#include <thread>
#include <atomic>
#include "framework.h"
#include "laba3.h"
#include <string>
#include <iostream>
#include <commdlg.h>
#include <fstream>
#include <sstream>
#include <commctrl.h>


#define MAX_LOADSTRING 100
#define IDM_START    1001
#define IDM_PAUSE    1002
#define IDM_RESUME   1003
#define WM_APP_UPDATE_UI (WM_APP + 1)

//HANDLE hPauseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

HANDLE hPauseEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
HANDLE hCopyEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
HWND hWnd;

struct CopyFileParams {
    UINT msgId;
    std::wstring source;
    std::wstring target;
};

// Глобальные переменные:
HINSTANCE hInst;                                       // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING] = L"Копирование";          // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];                   // имя класса главного окна
std::atomic<bool> isCopying{ false };  // Флаг копирования
std::atomic<bool> isPaused{ false };   // Флаг приостановки
std::wstring sourceFilePath;
std::wstring targetFilePath;

//void CopyFileAsync(HWND hWnd, const std::wstring& source, const std::wstring& target);

//DWORD WINAPI CopyFileAsyncThread(LPVOID lpParam);
//void CopyFileAsync(HWND hWnd, const std::wstring& source, const std::wstring& target);
DWORD WINAPI CopyFileAsync(LPVOID lpParam);


// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    //LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LABA3, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABA3));

    MSG msg;

    // Цикл основного сообщения:
    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}



//
//  ФУНКЦИЯ: MyRegisterClass()
//
//  ЦЕЛЬ: Регистрирует класс окна.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LABA3));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW + 2);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LABA3);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//   КОММЕНТАРИИ:
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
       CW_USEDEFAULT, CW_USEDEFAULT, 460, 480, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_CREATE: {
        CreateWindowW(L"BUTTON", L"Старт", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            42, 15, 100, 30, hWnd, (HMENU)IDM_START, hInst, nullptr);

        CreateWindowW(L"BUTTON", L"Приостановить", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            160, 15, 120, 30, hWnd, (HMENU)IDM_PAUSE, hInst, nullptr);

        CreateWindowW(L"BUTTON", L"Возобновить ", WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON,
            297, 15, 100, 30, hWnd, (HMENU)IDM_RESUME, hInst, nullptr);

        SendMessage(hWnd, WM_APP_UPDATE_UI, 0, 0);
    }
    break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_START:
                if (!isCopying) {
                    
                    OPENFILENAME ofn;
                    TCHAR szFile[MAX_PATH] = L"";

                    ZeroMemory(&ofn, sizeof(ofn));
                    ofn.lStructSize = sizeof(ofn);
                    ofn.hwndOwner = hWnd;
                    ofn.lpstrFile = szFile;
                    ofn.nMaxFile = sizeof(szFile);
                    ofn.lpstrFilter = L"All Files\0*.*\0";
                    ofn.nFilterIndex = 1;
                    ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

                    if (GetOpenFileName(&ofn)) {
                        sourceFilePath = szFile;

                        // Вызов диалога выбора целевого файла
                        ZeroMemory(&ofn, sizeof(ofn));
                        ofn.lStructSize = sizeof(ofn);
                        ofn.hwndOwner = hWnd;
                        ofn.lpstrFile = szFile;
                        ofn.nMaxFile = sizeof(szFile);
                        ofn.lpstrFilter = L"All Files\0*.*\0";
                        ofn.nFilterIndex = 1;
                        ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;

                        if (GetSaveFileName(&ofn)) {
                            targetFilePath = szFile;
                            isCopying = true;
                            SendMessage(hWnd, WM_APP_UPDATE_UI, 0, 0);
                            
                            CopyFileParams params;
                            //params.hWnd = hWnd;
                            params.msgId = WM_APP_UPDATE_UI;
                            params.source = sourceFilePath;
                            params.target = targetFilePath;

                            // Создаем поток
                            HANDLE hThread = CreateThread(NULL, 0, CopyFileAsync, &params, 0, NULL);
                            if (hThread == NULL) {
                                MessageBox(hWnd, L"Не удается создать поток.", L"Ошибка", MB_OK | MB_ICONERROR);
                                isCopying = false;
                                PostMessage(hWnd, WM_APP_UPDATE_UI, 0, 0);
                            }
                            else {
                                
                            }

                        }
                    }

                }
                break;
            case IDM_PAUSE:
                if (isCopying) {
                    isPaused = true;
                    // Отправить сообщение об изменении состояния
                    SendMessage(hWnd, WM_APP_UPDATE_UI, 0, 0);
                }
                break;
            case IDM_RESUME:
                if (isCopying) {
                    isPaused = false;
                    // Отправить сообщение об изменении состояния
                    SendMessage(hWnd, WM_APP_UPDATE_UI, 0, 0);
                }
                break;
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                if (MessageBox(hWnd, L"Вы действительно хотите закрыть окно?", L"Выход", MB_OKCANCEL) == IDOK) {
                    DestroyWindow(hWnd);
                }
                return 0;
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_APP_UPDATE_UI:
        
        EnableWindow(GetDlgItem(hWnd, IDM_START), !isCopying);
        EnableWindow(GetDlgItem(hWnd, IDM_PAUSE), isCopying && !isPaused);
        EnableWindow(GetDlgItem(hWnd, IDM_RESUME), isCopying && isPaused);

        break;

    case WM_CLOSE:
        if (MessageBox(hWnd, L"Вы действительно хотите закрыть окно?", L"Выход", MB_OKCANCEL) == IDOK) {
            DestroyWindow(hWnd);
        }
        return 0;
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Обработчик сообщений для окна "О программе".
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}


// Функция для асинхронного копирования файла
DWORD WINAPI CopyFileAsync(LPVOID lpParam) {
    isCopying = true;
    isPaused = false;

    CopyFileParams* params = reinterpret_cast<CopyFileParams*>(lpParam);

    HANDLE hSourceFile = CreateFileW(sourceFilePath.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    
    if (hSourceFile == INVALID_HANDLE_VALUE) {
        isCopying = false;
        PostMessage(hWnd, params->msgId, 0, 0);
        MessageBox(hWnd, L"Не удается открыть исходный файл.", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    HANDLE hTargetFile = CreateFileW(targetFilePath.c_str(), GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hTargetFile == INVALID_HANDLE_VALUE) {
        CloseHandle(hSourceFile);
        isCopying = false;
        PostMessage(hWnd, WM_APP_UPDATE_UI, 0, 0);
        MessageBox(hWnd, L"Не удается создать целевой файл.", L"Ошибка", MB_OK | MB_ICONERROR);
        return 1;
    }

    char buffer[4096];
    DWORD bytesRead, bytesWritten;

    while (ReadFile(hSourceFile, buffer, sizeof(buffer), &bytesRead, NULL) && bytesRead > 0) {
        if (isPaused) {
            // Если операция приостановлена, ждем, пока не будет возобновлена
            while (isPaused) {
                Sleep(100);
            }
        }

        if (!WriteFile(hTargetFile, buffer, bytesRead, &bytesWritten, NULL) || bytesRead != bytesWritten) {
            // Произошла ошибка записи
            CloseHandle(hSourceFile);
            CloseHandle(hTargetFile);
            isCopying = false;
            PostMessage(hWnd, WM_APP_UPDATE_UI, 0, 0);
            MessageBox(hWnd, L"Произошла ошибка записи в целевой файл.", L"Ошибка", MB_OK | MB_ICONERROR);
            return 1;
        }
    }

    isCopying = false;
  
    PostMessage(hWnd, WM_APP_UPDATE_UI, 0, 0);
    MessageBox(hWnd, L"Копирование завершено.", L"Информация", MB_OK | MB_ICONINFORMATION);
    
    CloseHandle(hSourceFile);
    CloseHandle(hTargetFile);
    ExitThread(0);

    return 0;
}

