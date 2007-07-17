// Copyright (c) 2007 Michael Chapman
//
// Permission is hereby granted, free of charge, to any person obtaining
// a copy of this software and associated documentation files (the
// "Software"), to deal in the Software without restriction, including
// without limitation the rights to use, copy, modify, merge, publish,
// distribute, sublicense, and/or sell copies of the Software, and to
// permit persons to whom the Software is furnished to do so, subject to
// the following conditions:
//
// The above copyright notice and this permission notice shall be
// included in all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
// EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
// MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
// IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY
// CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT,
// TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE
// SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.


#include "stdafx.h"
#include "resource.h"
#include "./FuzzyHook/FuzzyHook.h"
#include <ShellAPI.h>

#define MAX_LOADSTRING 100

#define SHELLTRAY_WNDCLASS _T("Shell_TrayWnd")
#define TRAYNOTIFY_WNDCLASS _T("TrayNotifyWnd")
#define TRAYCLOCK_WNDCLASS _T("TrayClockWClass")


HINSTANCE g_hInstance;
HWND g_hWnd;
TCHAR g_szTitle[MAX_LOADSTRING];
TCHAR g_szWindowClass[MAX_LOADSTRING];
NOTIFYICONDATAW g_nid;

const UINT RWM_TRAYICON = RegisterWindowMessage( _T("RWM_TRAYICON__C363ED38_3BEA_477b_B407_2A235F89F4E7") );


ATOM RegisterWindow( HINSTANCE );
BOOL InitInstance( HINSTANCE, int );
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
BOOL AddTrayIcon( HWND, LPCWSTR, HICON, UINT );
BOOL RemoveTrayIcon();
LRESULT OnTrayIcon( WPARAM, LPARAM );
HWND GetTrayClock();


int APIENTRY _tWinMain( HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPTSTR    lpCmdLine,
                        int       nCmdShow )
{
	UNREFERENCED_PARAMETER( hPrevInstance );
	UNREFERENCED_PARAMETER( lpCmdLine );

   HANDLE hMutexSingleInstance = CreateMutex( NULL, FALSE, _T("FuzzyClock__8E5405E2_BD48_41cd_AAF4_C7183EA13CBB") );

   if ( GetLastError() == ERROR_ALREADY_EXISTS ||
        GetLastError() == ERROR_ACCESS_DENIED )
   {
      return FALSE;
   }

	LoadString( hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING );
	LoadString( hInstance, IDC_FUZZYCLOCK, g_szWindowClass, MAX_LOADSTRING );

	RegisterWindow( hInstance );

	if ( !InitInstance( hInstance, nCmdShow ) )
	{
		return FALSE;
	}

   Hook( GetTrayClock() );

   MSG msg;

   HICON hTrayIcon = reinterpret_cast<HICON>( LoadImage( hInstance, MAKEINTRESOURCE( IDI_SMALL ), IMAGE_ICON, 0, 0, 0 ) );
   
   wchar_t szName[64];
   GetApplicationName( szName, 64 );
   AddTrayIcon( g_hWnd, szName, hTrayIcon, IDR_TRAYMENU );

	while ( GetMessage( &msg, NULL, 0, 0 ) )
	{
		TranslateMessage( &msg );
		DispatchMessage( &msg );
	}

   RemoveTrayIcon();
   DestroyIcon( hTrayIcon );

   Unhook();

	return static_cast<int>( msg.wParam );
}


ATOM RegisterWindow( HINSTANCE hInstance )
{
	WNDCLASSEX wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style              = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc        = WndProc;
	wcex.cbClsExtra         = 0;
	wcex.cbWndExtra         = 0;
	wcex.hInstance          = hInstance;
	wcex.hIcon              = LoadIcon( hInstance, MAKEINTRESOURCE( IDI_FUZZYCLOCK ) );
	wcex.hCursor            = LoadCursor( NULL, IDC_ARROW );
	wcex.hbrBackground      = reinterpret_cast<HBRUSH>( COLOR_WINDOW + 1 );
	wcex.lpszMenuName       = MAKEINTRESOURCE(IDC_FUZZYCLOCK);
	wcex.lpszClassName      = g_szWindowClass;
   wcex.hIconSm            = reinterpret_cast<HICON>( LoadImage( hInstance, MAKEINTRESOURCE( IDI_SMALL ), IMAGE_ICON, 0, 0, 0 ) );

	return RegisterClassEx( &wcex );
}


BOOL InitInstance( HINSTANCE hInstance, int nCmdShow )
{
   g_hInstance = hInstance;

   g_hWnd = CreateWindow( g_szWindowClass, g_szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, NULL, NULL, hInstance, NULL );

   if ( !g_hWnd )
   {
      return FALSE;
   }

   ShowWindow( g_hWnd, SW_HIDE );
   UpdateWindow( g_hWnd );

   return TRUE;
}


LRESULT CALLBACK WndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
	switch ( message )
	{
	case WM_COMMAND:

      {
		   int wmId = LOWORD( wParam );
		   int wmEvent = HIWORD( wParam );

         switch ( wmId )
		   {
		   case IDM_EXIT:

			   DestroyWindow( hWnd );
			   break;

		   default:

			   return DefWindowProc( hWnd, message, wParam, lParam );
		   }
      }
		break;

	case WM_PAINT:

      {
         PAINTSTRUCT ps;
         HDC hdc = BeginPaint( hWnd, &ps );
         EndPaint( hWnd, &ps );
      }
		break;

	case WM_DESTROY:

		PostQuitMessage( 0 );
		break;
   }

   if ( message == RWM_TRAYICON )
   {
      return OnTrayIcon( wParam, lParam );
   }

	return DefWindowProc( hWnd, message, wParam, lParam );
}


BOOL AddTrayIcon( HWND hWnd, LPCWSTR lpszToolTip, HICON hIcon, UINT uID )
{
   ZeroMemory( &g_nid, sizeof( NOTIFYICONDATA ) );

   g_nid.cbSize = sizeof( NOTIFYICONDATA );
   g_nid.hWnd = hWnd;
   g_nid.hIcon = hIcon;
   g_nid.uID = uID;
   g_nid.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP;
   g_nid.uCallbackMessage = RWM_TRAYICON;

   wcscpy_s( g_nid.szTip, sizeof( g_nid.szTip ), lpszToolTip );

   return Shell_NotifyIconW( NIM_ADD, &g_nid );
}


BOOL RemoveTrayIcon()
{
   g_nid.uFlags = 0;

   return Shell_NotifyIconW( NIM_DELETE, &g_nid );
}


LRESULT OnTrayIcon( WPARAM wParam, LPARAM lParam )
{
   if ( wParam != g_nid.uID )
   {
      return 0;
   }

   if ( LOWORD( lParam ) == WM_RBUTTONUP )
   {
      HMENU hMenu = LoadMenu( g_hInstance, MAKEINTRESOURCE( IDR_TRAYMENU ) );

      if ( NULL == hMenu )
      {
         return 0;
      }

      SetForegroundWindow( g_hWnd );

      HMENU hPopupMenu = GetSubMenu( hMenu, 0 );

      if ( NULL != hPopupMenu )
      {
         POINT cursorPos;
         GetCursorPos( &cursorPos );

         SetMenuDefaultItem( hPopupMenu, 0, TRUE );

         wchar_t buffer[64];
         GetExitText( buffer, 64 );

         MENUITEMINFOW mii;
         mii.cbSize = sizeof( MENUITEMINFOW );

         GetMenuItemInfoW( hPopupMenu, 0, TRUE, &mii );

         mii.fMask = MIIM_TYPE;
         mii.fType = MFT_STRING;
         mii.dwTypeData = buffer;
         mii.cch = (UINT)wcslen( buffer );

         SetMenuItemInfoW( hPopupMenu, 0, TRUE, &mii );

         TrackPopupMenu( hPopupMenu, TPM_LEFTALIGN, cursorPos.x, cursorPos.y, 0, g_hWnd, NULL );

         SendMessage( g_hWnd, WM_NULL, 0, 0 );
      }

      DestroyMenu( hMenu );
   }
   else if ( LOWORD( lParam ) == WM_LBUTTONDBLCLK )
   {
      HMENU hMenu = LoadMenu( g_hInstance, MAKEINTRESOURCE( IDR_TRAYMENU ) );

      if ( NULL == hMenu )
      {
         return 0;
      }

      SetForegroundWindow( g_hWnd );

      HMENU hPopupMenu = GetSubMenu( hMenu, 0 );

      if ( NULL != hPopupMenu )
      {
         UINT itemID = GetMenuItemID( hPopupMenu, 0 );

         SendMessage( g_hWnd, WM_COMMAND, itemID, 0 );
      }

      DestroyMenu( hMenu );
   }

   return 0;
}


HWND GetTrayClock()
{
   HWND hWndTrayClock = NULL;

   HWND hWndShellTray = FindWindow( SHELLTRAY_WNDCLASS, NULL );

   if ( NULL != hWndShellTray )
   {
      HWND hWndTrayNotify = FindWindowEx( hWndShellTray, NULL, TRAYNOTIFY_WNDCLASS, NULL );

      if ( NULL != hWndTrayNotify )
      {
         hWndTrayClock = FindWindowEx( hWndTrayNotify, NULL, TRAYCLOCK_WNDCLASS, NULL );
      }
   }

   return hWndTrayClock;
}