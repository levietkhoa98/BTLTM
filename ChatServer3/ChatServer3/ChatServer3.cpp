// ChatServer3.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include <iostream>
#include "ChatServer3.h"
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include "winsock2.h"

#define WM_SOCKET WM_USER + 1

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

SOCKET clients[64];
int numClients = 0;
SOCKET registeredClients[64]; 
int numRegisteredClients = 0;
char ids[64][64];
int ret;
char buf[1024];
char cmd[64];
char id[64];
char errorMsg[] = "Loi cu phap. Hay nhap lai\n";
char sendBuf[256];
char targetId[64];

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
	WSADATA wsa;
	WSAStartup(MAKEWORD(2, 2), &wsa);

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_CHATSERVER3, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_CHATSERVER3));

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
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_CHATSERVER3));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_CHATSERVER3);
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
				for (; j < numRegisteredClients; j++)
					if (wParam == registeredClients[j])
						break;
				if (j == numRegisteredClients)
				{
					ret = sscanf(buf, "%s %s", cmd, id);
					if (ret == 2)
					{
						if (strcmp(cmd, "client_id:") == 0)
						{
							char okMsg[] = "Dung cu phap. Hay nhap thong diep muon gui.\n";
							send(wParam, okMsg, strlen(okMsg), 0);
							registeredClients[numRegisteredClients] = wParam;
							strcpy(ids[numRegisteredClients], id);
							numRegisteredClients++;
							char clientBuf[32];
							strcpy(clientBuf, buf);

							SendDlgItemMessageA(hWnd, IDC_LIST_CLIENT, LB_ADDSTRING,
								0, (LPARAM)clientBuf);
						}
						else
						{
							send(wParam, errorMsg, strlen(errorMsg), 0);
						}
					}
					else 
					{
						send(wParam, errorMsg, strlen(errorMsg), 0);
					}
				}
				else
				{
					ret = sscanf(buf, "%s", targetId);
					SendDlgItemMessageA(hWnd, IDC_LIST_CHAT, LB_ADDSTRING,0, (LPARAM)buf);
					if (ret == 1)
					{
						if (strcmp(targetId, "all") == 0)
						{
							sprintf(sendBuf, "%s: %s", ids[j], buf + strlen(targetId) + 1);

							for (int j = 0; j < numRegisteredClients; j++)
								if (registeredClients[j] != wParam)
									send(registeredClients[j], sendBuf, strlen(sendBuf), 0);
						}
						else
						{
							sprintf(sendBuf, "%s: %s", ids[j], buf + strlen(targetId) + 1);

							for (int j = 0; j < numRegisteredClients; j++)
								if (strcmp(ids[j], targetId) == 0)
									send(registeredClients[j], sendBuf, strlen(sendBuf), 0);
						}
					}
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
