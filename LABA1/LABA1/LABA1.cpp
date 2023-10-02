#include "framework.h"
#include "LABA1.h"
#include <string>
#include <commdlg.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <commctrl.h>


#define MAX_LOADSTRING 100
#define IDC_TABCONTROL 1101
#define IDC_EDIT_TAB_START 2000

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
// старое HWND textEdit;									// для хранения дескриптора текстового поля в интерфейсе
HWND tabControl;	                            // дескриптер вкладок в интерфейсе  				
int tabIndexCounter = 0;							// счетчик вкладок 

ATOM                MyRegisterClass(HINSTANCE hInstance); // будет регистрировать класс окна
BOOL                InitInstance(HINSTANCE, int); // будет инициализировать экземпляр приложения.
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM); // обработка сообщений для главного окна
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);  // обрабатывать сообщения для диалогового окна "О программе".

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance, //предыдущий экземпляр 
	_In_ LPWSTR    lpCmdLine, // аргументы командной строки
	_In_ int       nCmdShow) // флаги отображения окна
{
	UNREFERENCED_PARAMETER(hPrevInstance); //предназначены для предотвращения
	UNREFERENCED_PARAMETER(lpCmdLine); //предупреждений о неиспользуемых параметрах функции.

	// Инициализация глобальных строк
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_LABA1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Выполнить инициализацию приложения:
	if (!InitInstance(hInstance, nCmdShow)) // создания и отображения главного окна приложения
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_LABA1));

	MSG msg; // хранение сообщений

	// Цикл основного сообщения:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}

//  ФУНКЦИЯ: MyRegisterClass()
//  ЦЕЛЬ: Регистрирует класс окна.

ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex; //  информация о классе окна

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW; //окно должно быть перерисовано, если его горизонтальный и вертикальный размер изменяется.
	wcex.lpfnWndProc = WndProc; //оконная процедуру, которая будет обрабатывать сообщения для окна
	wcex.cbClsExtra = 0; //  Резервированные поля для
	wcex.cbWndExtra = 0; // дополнительных данных класса и окна
	wcex.hInstance = hInstance; // Установка экземпляра приложения, полученного как параметр функции
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_LABA1)); //значок приложения в окне
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW); // курсор мыши
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1); // фон окна
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_LABA1); // меню для окна
	wcex.lpszClassName = szWindowClass; // именя класса окна
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL)); // маленький значок приложения

	return RegisterClassExW(&wcex); // Регистрация класса окна и возврат атома класса
}

//   ФУНКЦИЯ: InitInstance(HINSTANCE, int)
//   ЦЕЛЬ: Сохраняет маркер экземпляра и создает главное окно
//   КОММЕНТАРИИ:
//        В этой функции маркер экземпляра сохраняется в глобальной переменной, а также
//        создается и выводится главное окно программы.

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // Сохранить маркер экземпляра в глобальной переменной

	HWND hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 500, 500, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)  // Проверка, было ли успешно создано окно
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);

	return TRUE;
}


//  ФУНКЦИЯ: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  ЦЕЛЬ: Обрабатывает сообщения в главном окне.
//
//  WM_COMMAND  - обработать меню приложения
//  WM_PAINT    - Отрисовка главного окна
//  WM_DESTROY  - отправить сообщение о выходе и вернуться
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_CREATE:
	{
		// старое 
		//textEdit = CreateWindow(L"EDIT", L"Текстовый редактор", WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT |
			//ES_MULTILINE | ES_AUTOVSCROLL,
			//10, 10, 460, 420, hWnd, nullptr, nullptr, nullptr);
		tabControl = CreateWindowEx(0, WC_TABCONTROL, L"", WS_CHILD | WS_VISIBLE, 
			10, 10, 460, 30, hWnd, (HMENU)IDC_TABCONTROL, (HINSTANCE)GetWindowLong(hWnd, GWLP_HINSTANCE), nullptr);
		// (HINSTANCE)GetWindowLong(hWnd, GWLP_HINSTANCE)
		if (tabControl)
		{
			TCITEM tie;
			tie.mask = TCIF_TEXT;
			tabIndexCounter++;

			// Создайте первую вкладку
			tie.pszText = (LPWSTR)L"Tab 1";
			TabCtrl_InsertItem(tabControl, 0, &tie);

			// Создайте первое текстовое поле для первой вкладки
			CreateWindow(L"EDIT", L"", WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT |
				ES_MULTILINE | ES_AUTOVSCROLL, 10, 50, 460, 380, hWnd, (HMENU)IDC_EDIT_TAB_START, hInst, nullptr);
		}


	}
	break;
	case WM_NOTIFY:
	{
		NMHDR* pnmhdr = (NMHDR*)lParam;
		if (pnmhdr->code == TCN_SELCHANGE)
		{
			int currentTab = TabCtrl_GetCurSel(tabControl);
			

			// Скрыть все текстовые поля
			for (int i = 0; i < tabIndexCounter; i++)
			{
				ShowWindow(GetDlgItem(hWnd, IDC_EDIT_TAB_START + i), SW_HIDE);
			}

			// Показать текстовое поле для выбранной вкладки
			ShowWindow(GetDlgItem(hWnd, IDC_EDIT_TAB_START + currentTab), SW_SHOW);
			
		}
		break;
	}
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam); // Извлечение идентификатора команды 
		int wmEvent = HIWORD(wParam); 

		
		if (wmId == IDC_TABCONTROL) {
			// Обработка событий переключения вкладок
			int currentTab = TabCtrl_GetCurSel(tabControl);

			// Скрыть все текстовые поля
			for (int i = 0; i < tabIndexCounter; i++)
			{
				ShowWindow(GetDlgItem(hWnd, IDC_EDIT_TAB_START + i), SW_HIDE);
			}

			// Показать текстовое поле для выбранной вкладки
			ShowWindow(GetDlgItem(hWnd, IDC_EDIT_TAB_START + currentTab), SW_SHOW);

		}
		else
		{
			switch (wmId)
			{
			case IDM_ABOUT:
				DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
				break;
			case IDM_EXIT:
				if (MessageBox(hWnd, L"Вы действительно хотите закрыть окно?", L"Выход", MB_OKCANCEL) == IDOK) {
					DestroyWindow(hWnd);
				}
				return 0;
				break;
			case ID_32772: // SAVE
			{
				int currentTab = TabCtrl_GetCurSel(tabControl);
				int controlId = IDC_EDIT_TAB_START + currentTab;
				HWND textEdit = GetDlgItem(hWnd, controlId);

				static std::wstring fileName(MAX_PATH, L'\0'); // хранение имени файла
				OPENFILENAME ofn{}; // структура для настройки диалогового окна сохранения файла
				ofn.lStructSize = sizeof(OPENFILENAME); // размер структуры
				ofn.hwndOwner = hWnd; // дескриптор окна-владельца (главного окна приложения)
				ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
				ofn.lpstrFile = &fileName[0]; // указатель на строку, в которую будет сохранено имя выбранного файла.
				ofn.nMaxFile = MAX_PATH; //  максимальная длина имени файла
				ofn.Flags = OFN_OVERWRITEPROMPT; // флаг на перезапись файла (если файл с таким именем существует)


				if (!GetSaveFileName(&ofn)) {
					MessageBox(hWnd, L"Не удалось получить имя файла", L"Ошибка", MB_ICONINFORMATION);
					return 0;
				}

				// открытие файла для записи
				std::wofstream file(fileName, std::ios::binary);

				if (file.is_open()) {
					//Получения текста из элемента textEdit
					int textLength = GetWindowTextLength(textEdit);
					std::wstring text;
					text.resize(textLength + 1);
					file.imbue(std::locale("ru_RU.utf8"));
					GetWindowText(textEdit, &text[0], textLength + 1);

					// Запись текста в файл
					file << text;
					file.close();

					MessageBox(hWnd, L"Данные успешно сохранены!", L"Сохранение", MB_OK);
				}
				else {
					MessageBox(hWnd, L"Не удалось открыть файл для записи!", L"Ошибка", MB_OK | MB_ICONERROR);
				}
			}
			break;
			case ID_32771: // OPEN
			{
				int currentTab = TabCtrl_GetCurSel(tabControl);
				int controlId = IDC_EDIT_TAB_START + currentTab;
				HWND textEdit = GetDlgItem(hWnd, controlId);

				static std::wstring fileName(MAX_PATH, L'\0');
				OPENFILENAME ofn{};
				ofn.lStructSize = sizeof(OPENFILENAME);
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = L"Text Files (*.txt)\0*.txt\0All Files (*.*)\0*.*\0";
				ofn.lpstrFile = &fileName[0];
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST | OFN_HIDEREADONLY;
				// флаги: выбор существующего файла и пути (2й флаг) и скрыть файлы только для чтения 

				if (!GetOpenFileName(&ofn)) {
					MessageBox(hWnd, L"Не удалось получить имя файла", L"Ошибка", MB_ICONINFORMATION);
					return 0;
				}

				std::wifstream file(fileName, std::ios::binary);
				std::wstringstream buf;
				std::wstring file_content;

				if (file.is_open()) {
					int textLength = GetWindowTextLength(textEdit);
					std::wstring text;
					text.resize(textLength + 1);
					file.imbue(std::locale("ru_RU.utf8"));
					GetWindowText(textEdit, &text[0], textLength + 1);

					buf << file.rdbuf();
					file.close();
					file_content = buf.str();

					SetWindowTextW(textEdit, file_content.c_str());
				}
			}
			break;
			case ID_32775:  // new tab
			{
				// Добавление новой вкладки
					TCITEM tie;
					tie.mask = TCIF_TEXT;
					tabIndexCounter++;

					WCHAR tabText[64];
					swprintf_s(tabText, L"Tab %d", tabIndexCounter);
					tie.pszText = tabText;
					TabCtrl_InsertItem(tabControl, tabIndexCounter - 1, &tie); // вставка новой вкладки

					// Создание нового текстового поля для новой вкладки
					CreateWindow(L"EDIT", L"", WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_LEFT |
						ES_MULTILINE | ES_AUTOVSCROLL, 10, 50, 460, 380, hWnd,
						(HMENU)(IDC_EDIT_TAB_START + tabIndexCounter - 1), hInst, NULL);
					// Переключение на новую вкладку
					TabCtrl_SetCurSel(tabControl, tabIndexCounter - 1);
					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_TABCONTROL, 0), 0);
			}
			break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
			}

		}
	}
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
