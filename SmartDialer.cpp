// TrayIcon.cpp : Defines the entry point for the application.
//
#include "stdafx.h"
#include "resource.h"
#include "advras.h"

#define		NUM_ICON_FOR_ANIMATION	2
#define		MAX_LOADSTRING			100
#define		MYMSG_NOTIFYICON		(WM_APP + 100)
#define		ID_MENUITEM_1			327
#define		MAX_CONNECTIONS	10
#define		MAINWINDOW_WIDTH		600
#define		MAINWINDOW_HEIGHT		600

// Global Variables:
HINSTANCE		hInst;								// current instance
TCHAR			szTitle[MAX_LOADSTRING];			// The title bar text
TCHAR			szWindowClass[MAX_LOADSTRING];		// The title bar text
HWND			hWindowHandle;
TCHAR			gszDefaultEntryName[MAX_ENTRY_NAME_LEN];
bool gbRotating = FALSE;
MENUITEMINFO	MenuItemInfo[MAX_PHONE_ENTRY];
int				mDefaultMenuItem = 0;
// Global Vraibles
// Keep track of the last displayed icon
int				nCounter = 0; 
// List of Icon that is created and to be animated
static int		IconResourceArray[NUM_ICON_FOR_ANIMATION] = {IDR_MAINFRAME, IDI_GRAY};
// Current icon displayed index values.
static	int		m_nCounter = 0;
HWND hEdit;

// Foward declarations of functions included in this code module:
ATOM				MyRegisterClass(HINSTANCE hInstance);
BOOL				InitInstance(HINSTANCE, int);
LRESULT CALLBACK	WndProc(HWND, UINT, WPARAM, LPARAM);


VOID AnimateIcon(HINSTANCE hInstance, HWND hWnd, DWORD dwMsgType,UINT nIndexOfIcon);
VOID TimerAnimationIcon(HINSTANCE hInst, HWND hWnd);
LONG OnTrayNotification(HINSTANCE hInstance, HWND hWnd, WPARAM wParam, LPARAM lParam);


int APIENTRY WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow)
{
 	// TODO: Place code here.
	MSG msg;
	HACCEL hAccelTable;

	// Initialize global strings
	LoadString(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadString(hInstance, IDC_TRAYICON, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// Perform application initialization:
	if (!InitInstance (hInstance, nCmdShow)) 
	{
		return FALSE;
	}

	hAccelTable = LoadAccelerators(hInstance, (LPCTSTR)IDC_TRAYICON);

	// Main message loop:
	while (GetMessage(&msg, NULL, 0, 0)) 
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg)) 
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return msg.wParam;
}

void Connect()
{
	AddRoute();
	Dial(gszDefaultEntryName);
	SendMessage(hEdit,
		WM_SETTEXT,
		NULL,
		(LPARAM)gszLogData);
}

void Disconnect()
{
	BYTE RasConnData[MAX_CONNECTIONS*sizeof(RASCONN)];
	LPRASCONN pRasConn = (LPRASCONN) RasConnData;
	if (GetCurrentConnections(RasConnData) > 0 ) {

		pRasConn = (LPRASCONN)RasConnData;
		Hangup(pRasConn->hrasconn);
		DelRoute();
	}
}
//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
//  COMMENTS:
//
//    This function and its usage is only necessary if you want this code
//    to be compatible with Win32 systems prior to the 'RegisterClassEx'
//    function that was added to Windows 95. It is important to call this function
//    so that the application will get 'well formed' small icons associated
//    with it.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX); 

	wcex.style			= CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc	= (WNDPROC)WndProc;
	wcex.cbClsExtra		= 0;
	wcex.cbWndExtra		= 0;
	wcex.hInstance		= hInstance;
	wcex.hIcon			= LoadIcon(hInstance, (LPCTSTR)IDI_TRAYICON);
	wcex.hCursor		= LoadCursor(NULL, IDC_ARROW);
	wcex.hbrBackground	= (HBRUSH)(COLOR_WINDOW+1);
	wcex.lpszMenuName	= (LPCSTR)IDC_TRAYICON;
	wcex.lpszClassName	= szWindowClass;
	wcex.hIconSm		= LoadIcon(wcex.hInstance, (LPCTSTR)IDI_TRAYICON);

	return RegisterClassEx(&wcex);
}

//
//   FUNCTION: InitInstance(HANDLE, int)
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
   HWND hWnd;

   hInst = hInstance; // Store instance handle in our global variable

   hWnd = CreateWindow(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      			100,
				100,
				600,
				600,
				NULL, NULL, hInstance, NULL);

   if (!hWnd)
   {
      return FALSE;
   }

  // ShowWindow(hWnd, nCmdShow);
   ShowWindow(hWnd, SW_HIDE);
   UpdateWindow(hWnd);

   // Add icon on system tray during initialzation
   AnimateIcon(hInstance, hWnd, NIM_ADD, 0);
   // Set a timer functionality to animate the icon in a certain interval
   SetTimer(hWnd, 1, 500, NULL);
   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, unsigned, WORD, LONG)
//
//  PURPOSE:  Processes messages for the main window.
//
//  WM_COMMAND	- process the application menu
//  WM_PAINT	- Paint the main window
//  WM_DESTROY	- post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int wmId, wmEvent;

	BYTE RasConnData[MAX_CONNECTIONS*sizeof(RASCONN)];

	PHONE_ENTRY phoneEntries[MAX_PHONE_ENTRY];
	int count = GetPhoneBookEntries(phoneEntries);

	switch (message) 
	{
		case WM_CREATE:
			ReadDefaultPhonebookEntry();
			hEdit=CreateWindowEx(WS_EX_CLIENTEDGE,
				"EDIT",
				"",
				WS_CHILD|WS_VISIBLE|
				ES_MULTILINE|ES_AUTOVSCROLL|ES_AUTOHSCROLL,
				0,
				0,
				MAINWINDOW_WIDTH,
				MAINWINDOW_HEIGHT,
				hWnd,
				NULL,
				GetModuleHandle(NULL),
				NULL);

			if (GetCurrentConnections(RasConnData) > 0) {
					AnimateIcon(hInst, hWnd, NIM_MODIFY,0);
				}
			else {
					AnimateIcon(hInst, hWnd, NIM_MODIFY,1);
					Connect();
				}
			break;
		case WM_COMMAND:
			wmId    = LOWORD(wParam); 
			wmEvent = HIWORD(wParam); 
			// Parse the menu selections:
			for (int i=0; i<count; i++) {
				if (wmId == MenuItemInfo[i].wID) {
					MenuItemInfo[i].fState=MFS_CHECKED ;
					strcpy_s(gszDefaultEntryName, phoneEntries[i].entryName);
					WriteDefaultPhonebookEntry();
					mDefaultMenuItem = i;
					return 0;
				}
			}
			switch (wmId)
			{
				case IDM_EXIT:
				case ID_FILE_KILLPROCESS:
				   DestroyWindow(hWnd);
				   break;
				case ID_TRAYICON_MINIMISE:
					ShowWindow(hWnd, SW_MINIMIZE);
					break;
				case ID_TRAYICON_HIDEWINDOW:
					ShowWindow(hWnd, SW_HIDE);
					break;
				case ID_TRAYICON_MAXIMIZEWINDOW:
					ShowWindow(hWnd, SW_MAXIMIZE);
					break;
				case ID_DISCONNECT:
					Disconnect();
					break;
				case ID_TRAYICON_CONNECT:
					
					Connect();
					break;
				case ID_TRAYICON_SHOWLOG:
					SendMessage(hEdit,
						WM_SETTEXT,
						NULL,
						(LPARAM)gszLogData);
					ShowWindow(hWnd, SW_SHOW);
				default:
				   return DefWindowProc(hWnd, message, wParam, lParam);
			}
			break;
		case WM_CLOSE:
		case WM_DESTROY:
		case WM_QUIT:
			KillTimer(hWnd, 1);
			AnimateIcon(hInst, hWnd, NIM_DELETE, m_nCounter);
			Disconnect();
			PostQuitMessage(0);
			break;
		case WM_TIMER:
			if (gbRotating == TRUE) {
				TimerAnimationIcon(hInst, hWnd);
			}
			else {
				
				if (GetCurrentConnections(RasConnData) > 0) {
					AnimateIcon(hInst, hWnd, NIM_MODIFY,0);
				}
				else {
					AnimateIcon(hInst, hWnd, NIM_MODIFY,1);
				}
					
				//KillTimer(hWnd, 1);
			}
		case MYMSG_NOTIFYICON:
			// System Tray Icon notification will be executed..
			OnTrayNotification(hInst, hWnd, wParam, lParam);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
   }
   return 0;
}

/*	Function Name   : TimerAnimationIcon
	Description		: Function which will keep track of the current displayed icons, based on that
					  this will select the next icon to be displayed by sending a modify message type
					  to the system tray icon.
	Function Called	: AnimateIcon
	Global Variable	: m_nCounter - Keep track of the icon which is currently dispalyed
					  nCounter   - Incremented one by one to select the next icon from the icon list.
*/
VOID TimerAnimationIcon(HINSTANCE hInst, HWND hWnd)
{
	AnimateIcon(hInst, hWnd, NIM_MODIFY, nCounter);
	m_nCounter = nCounter; //Store it here to keep track of the last displayed icon
	nCounter++;
	nCounter = nCounter%(NUM_ICON_FOR_ANIMATION);
}

/*	Function Name   : AnimateIcon
	Description		: Function which will act based on the message type that is received as parameter
					  like ADD, MODIFY, DELETE icon in the system tray. Also send a message to display
					  the icon in title bar as well as in the task bar application.
	Function Called	: Shell_NotifyIcon	-	API which will Add, Modify, Delete icon in tray.
					  SendMessage - Send a message to windows
	Variable		: NOTIFYICONDATA - Structure which will have the details of the tray icons
*/
void AnimateIcon(HINSTANCE hInstance, HWND hWnd, DWORD dwMsgType,UINT nIndexOfIcon)
{
	HICON hIconAtIndex = LoadIcon(hInstance, (LPCTSTR) MAKEINTRESOURCE(IconResourceArray[nIndexOfIcon]));

	NOTIFYICONDATA IconData;

	memset(&IconData, 0, sizeof(NOTIFYICONDATA));
	IconData.cbSize = sizeof(NOTIFYICONDATA);
	IconData.hIcon  = hIconAtIndex;
	IconData.hWnd   = hWnd;
	lstrcpyn(IconData.szTip,"Smart Dialer", (int) strlen("Smart Dialer")+1);
	IconData.uCallbackMessage = MYMSG_NOTIFYICON;
	IconData.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
	
	Shell_NotifyIcon(dwMsgType, &IconData);
	SendMessage(hWnd, WM_SETICON, NULL, (long) hIconAtIndex);

	if(hIconAtIndex)
		DestroyIcon(hIconAtIndex);
}

/*	Function Name   : OnTrayNotification
	Description		: CALLBACK routine which will be get executed when a notification is identified on the
					  system tray. 
	Function Called	: LoadMenu	-	Load the menu into the application
*/
LONG OnTrayNotification(HINSTANCE hInstance, HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	PHONE_ENTRY phoneEntries[MAX_PHONE_ENTRY];
	int count = GetPhoneBookEntries(phoneEntries);

	HMENU hMenu;
	
	BYTE RasConnData[MAX_CONNECTIONS *sizeof(RASCONN)];

	for(int i=0;i<count;i++) {
		MenuItemInfo[i].cbSize=sizeof(MENUITEMINFO);
		MenuItemInfo[i].fMask=MIIM_STATE|MIIM_TYPE|MIIM_ID;
		MenuItemInfo[i].fType=MFT_STRING;
		MenuItemInfo[i].fState=MFS_ENABLED;
		MenuItemInfo[i].wID=ID_MENUITEM_1+i;
		MenuItemInfo[i].hSubMenu=NULL;
		MenuItemInfo[i].hbmpUnchecked=NULL;
		MenuItemInfo[i].dwItemData=0;
		MenuItemInfo[i].dwTypeData=phoneEntries[i].entryName;
		MenuItemInfo[i].cch=strlen(phoneEntries[i].entryName);
		if (strcmp(gszDefaultEntryName, phoneEntries[i].entryName) == 0) {
			mDefaultMenuItem = i;
		}

	}
	MenuItemInfo[mDefaultMenuItem].fState=MFS_CHECKED ;
	strcpy_s(gszDefaultEntryName, phoneEntries[mDefaultMenuItem].entryName);


	switch(lParam)
	{
		case WM_RBUTTONDOWN:
		{
			/** Load and verify the menu**/
			if(hMenu = LoadMenu(hInstance, (LPCTSTR) MAKEINTRESOURCE(IDC_TRAYICON)))
			{
				hMenu = GetSubMenu(hMenu, 0);
				for(int i=0;i<count;i++) {
					InsertMenuItem(hMenu, 0, TRUE, &MenuItemInfo[i]);
				}
				if(hMenu != NULL)
				{
					POINT pt;
					GetCursorPos(&pt);
					SetForegroundWindow(hWnd);
					TrackPopupMenu(hMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON, pt.x, pt.y, 0, hWnd, NULL);
				}
			}
			
			break ;
		}
	}
	return 0;
}