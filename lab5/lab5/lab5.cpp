#include "framework.h"
#include "lab5.h"
#include <windows.h>
#include <dbt.h>
#include <fstream>
#include <ctime>


#define MAX_LOADSTRING 100

std::ofstream logFile;
HWND hWnd;
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
bool flag = true;                               // чтобы 2 раза не показывало окно 

// Отправить объявления функций, включенных в этот модуль кода:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);
std::string GetDateTimeString();

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_LAB5, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);
  

    // Выполнить инициализацию приложения:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LAB5));

    MSG msg;

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

ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LAB5));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_LAB5);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, CW_USEDEFAULT, 400, 300, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   AddClipboardFormatListener(hWnd);
   logFile.open("log.txt", std::ios::app);
   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

int count_usb = 0;
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            switch (wmId)
            {
            case IDM_EXIT:
                if (MessageBox(hWnd, L"Вы действительно хотите закрыть окно?", L"Выход", MB_OKCANCEL) == IDOK) {
                    RemoveClipboardFormatListener(hWnd);
                    DestroyWindow(hWnd);
                }
                return 0;
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DISPLAYCHANGE: // изменении разрешения экрана

        MessageBox(NULL, L"Изменение разрешения экрана", L"Событие", MB_ICONINFORMATION);
        logFile << GetDateTimeString() << "Изменение разрешения экрана" << std::endl;
        break;
    case WM_SETTINGCHANGE:
        if (wParam == SPI_SETDESKWALLPAPER && flag) { // изменение обоев рабочего стола
            flag = false;
            MessageBox(NULL, L"Изменение обоев рабочего стола", L"Событие", MB_ICONINFORMATION);
            logFile << GetDateTimeString() << "Изменение обоев рабочего стола" << std::endl;
            flag = true;
        }
        break;
    case WM_INPUTLANGCHANGE: // язык клавиатуры
        MessageBox(NULL, L"Изменение языка клавиатуры", L"Событие", MB_ICONINFORMATION);
        logFile << GetDateTimeString() << "Изменение языка клавиатуры" << std::endl;
        break;

    case WM_DEVICECHANGE: // USB
        count_usb++;
        if (count_usb % 2 == 0) {
            MessageBox(NULL, L"USB", L"Событие", MB_ICONINFORMATION);
            logFile << GetDateTimeString() << "USB" << std::endl;
        }
        break;
        
    case WM_CLIPBOARDUPDATE: // буфер
        MessageBox(NULL, L"Изменения в буфере обмена", L"Событие", MB_ICONINFORMATION);
        logFile << GetDateTimeString() << "Изменения в буфере обмена" << std::endl;
        break;
        
    case WM_TIMECHANGE: // время
        MessageBox(NULL, L"Изменение системного времени", L"Событие", MB_ICONINFORMATION);
        logFile << GetDateTimeString() << "Изменение системного времени" << std::endl;
        break;

    case WM_POWERBROADCAST:
        if (wParam == PBT_APMPOWERSTATUSCHANGE) {  // зарядка батареи
            MessageBox(NULL, L"Изменение состояния зарядки", L"Событие", MB_ICONINFORMATION);
            logFile << GetDateTimeString() << "Изменение состояния зарядки" << std::endl;
        }
        break;
    case WM_CLOSE:
        if (MessageBox(hWnd, L"Вы действительно хотите закрыть окно?", L"Выход", MB_OKCANCEL) == IDOK) {
            RemoveClipboardFormatListener(hWnd);
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

std::string GetDateTimeString() {
    time_t currentTime = time(0);
    struct tm now;
    localtime_s(&now, &currentTime);
    char dateTime[50];
    strftime(dateTime, sizeof(dateTime), "[%Y-%m-%d %H:%M:%S]", &now);
    return std::string(dateTime);
}
