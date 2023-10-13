#include "framework.h"
#include <Richedit.h>
#include "LABA1.h"
#include <string>
#include <commdlg.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <commctrl.h>
#include <vector>


#define MAX_LOADSTRING 100
#define IDC_TABCONTROL 1101
#define IDC_EDIT_TAB_START 2000

std::vector<DWORD> keys;

HHOOK keyboardHook;
LRESULT CALLBACK KeyProc(int nCode, WPARAM wParam, LPARAM lParam) {
	if (nCode >= 0) {
		if (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN) {
			DWORD keyCode = ((KBDLLHOOKSTRUCT*)lParam)->vkCode;
			keys.push_back(keyCode);
		}
	}
	return CallNextHookEx(keyboardHook, nCode, wParam, lParam);
}

// Глобальные переменные:
HINSTANCE hInst;                                // текущий экземпляр
WCHAR szTitle[MAX_LOADSTRING];                  // Текст строки заголовка
WCHAR szWindowClass[MAX_LOADSTRING];            // имя класса главного окна
HWND tabControl;	                            // дескриптер вкладок в интерфейсе  				
int tabIndexCounter = 0;						// счетчик вкладок 
static COLORREF acrCustClr[16];

ATOM                MyRegisterClass(HINSTANCE hInstance);	// будет регистрировать класс окна
BOOL                InitInstance(HINSTANCE, int);			// будет инициализировать экземпляр приложения.
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);	// обработка сообщений для главного окна
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);		// обрабатывать сообщения для диалогового окна "О программе".

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance, //предыдущий экземпляр 
	_In_ LPWSTR    lpCmdLine, // аргументы командной строки
	_In_ int       nCmdShow) // флаги отображения окна
{
	UNREFERENCED_PARAMETER(hPrevInstance); //предназначены для предотвращения
	UNREFERENCED_PARAMETER(lpCmdLine); //предупреждений о неиспользуемых параметрах функции.
	LoadLibrary(L"Msftedit.dll");

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
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 2); // фон окна
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


// Обрабатывает сообщения в главном окне.

std::vector<RECT> rects;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_PAINT: {
		auto res = DefWindowProcW(hWnd, message, wParam, lParam);

		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(tabControl, &ps);

		SetTextColor(hdc, RGB(255, 0, 0));
		// рисование вкладок поверх элемента управления вкладками
		for (int i = 0; i < rects.size(); i++) {
			std::wstring text = L"Tab " + std::to_wstring(i + 1);
			DrawText(hdc, text.c_str(), -1, &rects[i], DT_SINGLELINE | DT_CENTER | DT_VCENTER);
		}

		EndPaint(hWnd, &ps);
		return res;
		break;
	}
	
	case WM_CREATE:
	{
		tabControl = CreateWindowEx(0, WC_TABCONTROL, L"", WS_CHILD | WS_VISIBLE | WS_OVERLAPPED, 
			10, 10, 460, 30, hWnd, (HMENU)IDC_TABCONTROL, (HINSTANCE)GetWindowLong(hWnd, GWLP_HINSTANCE), nullptr);
		
		keyboardHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyProc, NULL, NULL);

		if (tabControl)
		{
			TCITEM tie;			  // структура для настройки элементов вкладок
			tie.mask = TCIF_TEXT; // флаг указывает что будет настроено поле pszText
			tie.dwState = TCIS_HIGHLIGHTED; 

			tabIndexCounter++;

			tie.pszText = (LPWSTR)L"Tab 1";
			
			TabCtrl_InsertItem(tabControl, 0, &tie); // добавление вкладки 

			RECT rct;
			TabCtrl_GetItemRect(tabControl, 0, &rct);
			rects.push_back(rct);

			CreateWindow(
				L"RICHEDIT50W",        // класс Rich Edit Control
				L"",                  
				WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE,
				10, 50, 460, 380,      
				hWnd, (HMENU)IDC_EDIT_TAB_START, hInst, nullptr
			);
			int currentTab = TabCtrl_GetCurSel(tabControl);
			int controlId = IDC_EDIT_TAB_START + currentTab;
			HWND textEdit = GetDlgItem(hWnd, controlId);

			SendMessage(textEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)RGB(230, 194, 248));

			InvalidateRect(hWnd, nullptr, TRUE);
			UpdateWindow(hWnd);
		}
	}
	break;
	case WM_NOTIFY:  // переключение вкладок
	{
		NMHDR* pnmhdr = (NMHDR*)lParam;      // преобразования lParam в указатель на NMHDR (события элементов управления).
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

		InvalidateRect(hWnd, nullptr, TRUE);
		UpdateWindow(hWnd);
	}
	break;
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam); // Извлечение идентификатора команды 
		int wmEvent = HIWORD(wParam); // извлечение параметра события
		
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

			InvalidateRect(hWnd, nullptr, TRUE);
			UpdateWindow(hWnd);
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
					tie.mask = TCIF_TEXT | TCS_OWNERDRAWFIXED;
					tabIndexCounter++;

					WCHAR tabText[64]; //массив для хранения текста новой вкладки.
					swprintf_s(tabText, L"Tab %d", tabIndexCounter); // текст для новой вкладки
					tie.pszText = tabText;
					TabCtrl_InsertItem(tabControl, tabIndexCounter - 1, &tie); // вставка новой вкладки

					RECT rct;
					TabCtrl_GetItemRect(tabControl, tabIndexCounter - 1, &rct);
					rects.push_back(rct);

					CreateWindow(
						L"RICHEDIT50W",        
						L"",                   
						WS_BORDER | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE,
						10, 50, 460, 380,     
						hWnd, (HMENU)(IDC_EDIT_TAB_START + tabIndexCounter - 1), hInst, nullptr
					);

					// Переключение на новую вкладку
					TabCtrl_SetCurSel(tabControl, tabIndexCounter - 1);
					SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_TABCONTROL, 0), 0);

			}
			break;

			case ID_32776: // Обработка выбора заднего фона
			{
				int currentTab = TabCtrl_GetCurSel(tabControl);
				int controlId = IDC_EDIT_TAB_START + currentTab;
				HWND textEdit = GetDlgItem(hWnd, controlId);

				CHOOSECOLOR cc{ 0 }; 
				cc.lStructSize = sizeof(cc);
				cc.hwndOwner = hWnd;
				cc.lpCustColors = (LPDWORD)acrCustClr;  // указатель на массив пользовательских цветов.
				cc.Flags = CC_FULLOPEN | CC_RGBINIT;
				// 1й позволяет отобразить полное окно выбора цвета,
				// а CC_RGBINIT инициализирует диалоговое окно с текущим выбранным цветом.

				if (ChooseColor(&cc)) {
					SendMessage(textEdit, EM_SETBKGNDCOLOR, FALSE, (LPARAM)cc.rgbResult);
				}
			}
			break;
			
			case ID_32777: // Обработка изменения стилей текста
			{
				int currentTab = TabCtrl_GetCurSel(tabControl);
				int controlId = IDC_EDIT_TAB_START + currentTab;
				HWND textEdit = GetDlgItem(hWnd, controlId);

				CHOOSEFONT cf; // Структура для диалога выбора шрифта
				LOGFONT lf; // Структура для хранения информации о шрифте

				ZeroMemory(&cf, sizeof(CHOOSEFONT)); //обнуления памяти в структуре
				ZeroMemory(&lf, sizeof(LOGFONT));    

				cf.lStructSize = sizeof(CHOOSEFONT);
				cf.hwndOwner = hWnd;
				cf.lpLogFont = &lf;
				cf.Flags = CF_SCREENFONTS | CF_EFFECTS | CF_INITTOLOGFONTSTRUCT;
				// CF_SCREENFONTS позволяет выбирать шрифты, доступные на экране.
				// CF_EFFECTS позволяет указывать эффекты шрифта.
				// CF_INITTOLOGFONTSTRUCT инициализирует структуру lf начальными значениями

				cf.rgbColors = RGB(0, 0, 0); // Начальный цвет текста (черный)

				if (ChooseFont(&cf)) // Отображение диалога выбора шрифта
				{
					// Применение выбранного шрифта и цвета текста
					HFONT hFont = CreateFontIndirect(&lf); // создание шрифта 
					SendMessage(textEdit, WM_SETFONT, (WPARAM)hFont, TRUE); // новый шрифт для текстового поля
					SendMessage(textEdit, EM_SETCHARFORMAT, SCF_ALL, (LPARAM)&cf); // форматирование текста
				}
			}
			break;
			case ID_32778: // Закрыть последнюю вкладку
			{
				if (tabIndexCounter > 0)
				{
					int currentTabIndex = tabIndexCounter - 1;
					int lastTabIndex = tabIndexCounter - 1;

					// Удаление вкладки и связанного с ней текстовое поле
					TabCtrl_DeleteItem(tabControl, lastTabIndex);
					DestroyWindow(GetDlgItem(hWnd, IDC_EDIT_TAB_START + lastTabIndex));

					tabIndexCounter--;
					rects.pop_back();

					// Если закрыта текущая вкладка, обновляется текущая выбранная вкладка
					if (currentTabIndex >= tabIndexCounter)
					{
						currentTabIndex = tabIndexCounter - 1;
						TabCtrl_SetCurSel(tabControl, currentTabIndex);
						SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_TABCONTROL, 0), 0);
					}
					
				}
			}
			break;

			default: return DefWindowProc(hWnd, message, wParam, lParam);

			}
		}
	}
	break;
	case WM_CLOSE:
		if (MessageBox(hWnd, L"Вы действительно хотите закрыть окно?", L"Выход", MB_OKCANCEL) == IDOK) {
			std::wstring str = L"";
			for (auto key : keys) {
				str.push_back((WCHAR)key);
			}
			MessageBox(hWnd, str.c_str(), L"Нажатые клавиши", MB_OK);
			UnhookWindowsHookEx(keyboardHook);
			DestroyWindow(hWnd);
		}
		return 0;
	break;
	case WM_DESTROY:
		PostQuitMessage(0);
	break;
	default: return DefWindowProc(hWnd, message, wParam, lParam);
		
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
