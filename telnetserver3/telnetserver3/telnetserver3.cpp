// telnetserver3.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <iostream>
#include "telnetserver3.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"

#define MAX_LOADSTRING 100
#define WM_SOCKET WM_USER + 1

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name
int totalUser = 0;
char errorMsg[] = "Loi cu phap. Hay nhap lai\n";
char clientdata[64][64];
char clientname[64][64];
char clientpass[64][64];
int ret;

SOCKET clients[64];
int numClients = 0;

char buf[256];

SOCKET passed[64];
int numPassed = 0;

char pass[64];
char name[64];
char cmdBuf[256];
char fileBuf[256];

// Forward declarations of functions included in this code module:
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

    // TODO: Place code here.
	FILE* inp;
	inp = fopen("test.txt", "r");
	int i = 0;
	while (1) {
		char r = (char)fgetc(inp);
		int k = 0;
		while (r != '\n' && !feof(inp)) {
			clientdata[i][k++] = r;
			r = (char)fgetc(inp);
		}
		clientdata[i][k] = 0;
		if (feof(inp)) {
			break;
		}
		i++;
	}
	totalUser = i;

	for (int j = 0; j <= totalUser; j++) {
		sscanf(clientdata[j], "%s %s", clientname[j], clientpass[j]);
	}
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_TELNETSERVER3, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_TELNETSERVER3));

    MSG msg;

    // Main message loop:
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
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TELNETSERVER3));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_TELNETSERVER3);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }
   CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), TEXT(""),
	   WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL,
	   10, 10, 160, 350, hWnd, (HMENU)IDC_LIST_CLIENT, GetModuleHandle(NULL), NULL);

   CreateWindowEx(WS_EX_CLIENTEDGE, TEXT("LISTBOX"), TEXT(""),
	   WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOVSCROLL,
	   170, 10, 160, 350, hWnd, (HMENU)IDC_LIST_CHAT, GetModuleHandle(NULL), NULL);

   SOCKET listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

   SOCKADDR_IN addr;
   addr.sin_family = AF_INET;
   addr.sin_addr.s_addr = htonl(INADDR_ANY);
   addr.sin_port = htons(9000);

   bind(listener, (SOCKADDR *)&addr, sizeof(addr));
   listen(listener, 5);

   WSAAsyncSelect(listener, hWnd, WM_SOCKET, FD_ACCEPT);

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
	case WM_SOCKET:
	{
		if (WSAGETSELECTERROR(lParam))
		{
			closesocket(wParam);
			return DefWindowProc(hWnd, message, wParam, lParam);
		}

		if (WSAGETSELECTEVENT(lParam) == FD_ACCEPT)
		{
			SOCKADDR_IN clientAddr;
			int clientAddrLen = sizeof(clientAddr);

			SOCKET client = accept(wParam, (SOCKADDR *)&clientAddr, &clientAddrLen);

			char msg[] = "Hello client.Please insert your client_id:\n";
			send(client, msg, strlen(msg), 0);
			clients[numClients] = client;
			numClients++;

			WSAAsyncSelect(client, hWnd, WM_SOCKET, FD_READ | FD_CLOSE);
		}
		else if (WSAGETSELECTEVENT(lParam) == FD_READ)
		{
			int ret = recv(wParam, buf, sizeof(buf), 0);
			buf[ret] = 0;
			int j = 0;
			for (; j < numPassed; j++)
				if (wParam == passed[j])
					break;
			if (j == numPassed)
			{
				ret = sscanf(buf, "%s %s", name, pass);
				if (ret == 2)
				{
					for (int j = 0; j <= totalUser; j++)
					{
						if (strcmp(name, clientname[j]) == 0 && strcmp(pass, clientpass[j]) == 0)
						{
							char welcome[] = "You are connected to the server. You can now chat\n";
							send(wParam, welcome, strlen(welcome), 0);
							passed[numPassed] = wParam;
							numPassed++;
							char clientBuf[32];
							strcpy(clientBuf, name);

							SendDlgItemMessageA(hWnd, IDC_LIST_CLIENT, LB_ADDSTRING,
								0, (LPARAM)clientBuf);
						}
						else {
							char error[] = "Error please try again\n";
							send(wParam, error, strlen(error), 0);
						}
					}
				}
				else
				{
					send(wParam, errorMsg, strlen(errorMsg), 0);
				}
			}
			else
			{
				ret = recv(wParam, buf, sizeof(buf), 0);
				buf[ret] = 0;
				// thuc hien gui 
				if (buf[ret - 1] == '\n')
					buf[ret - 1] = 0;

				sprintf(cmdBuf, "%s > out.txt", buf);
				system(cmdBuf);

				FILE *f = fopen("out.txt", "r");
				while (fgets(fileBuf, sizeof(fileBuf), f))
				{
					send(wParam, fileBuf, strlen(fileBuf), 0);
					SendDlgItemMessageA(hWnd, IDC_LIST_CHAT, LB_ADDSTRING, 0, (LPARAM)fileBuf);
				}
				fclose(f);
			}
		}
	}
	break;
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
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
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
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
