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
#include "XMLHelper.h"
#include <ShellAPI.h>
#include <vector>
#include <string>

#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

#define MAX_LOADSTRING 100

#define SHELLTRAY_WNDCLASS _T("Shell_TrayWnd")
#define TRAYNOTIFY_WNDCLASS _T("TrayNotifyWnd")
#define TRAYCLOCK_WNDCLASS _T("TrayClockWClass")


HINSTANCE g_hInstance;
HWND g_hWnd;
TCHAR g_szTitle[MAX_LOADSTRING];
TCHAR g_szWindowClass[MAX_LOADSTRING];
NOTIFYICONDATAW g_nid;

XMLHelper g_xmlHelper;

const UINT RWM_TRAYICON = RegisterWindowMessage( _T("RWM_TRAYICON__C363ED38_3BEA_477b_B407_2A235F89F4E7") );


ATOM RegisterWindow( HINSTANCE );
BOOL InitInstance( HINSTANCE, int );
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
BOOL AddTrayIcon( HWND, LPCWSTR, HICON, UINT );
BOOL RemoveTrayIcon();
LRESULT OnTrayIcon( WPARAM, LPARAM );
HWND GetTrayClock();
tstring GetXMLFile();
tstring GetDefaultXMLFile( LPCTSTR );


int APIENTRY _tWinMain( HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPTSTR    lpCmdLine,
                        int       nCmdShow )
{
   UNREFERENCED_PARAMETER( hPrevInstance );
   UNREFERENCED_PARAMETER( lpCmdLine );

   HANDLE hMutexSingleInstance = CreateMutex( NULL, FALSE,
      _T("FuzzyClock__8E5405E2_BD48_41cd_AAF4_C7183EA13CBB") );

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

   tstring xmlFile = GetXMLFile();

   if ( xmlFile.empty() )
   {
      return FALSE;
   }

   if ( !g_xmlHelper.LoadFile( xmlFile.c_str() ) )
   {
      return FALSE;
   }

   for ( UINT index = 0; index < g_xmlHelper.GetHoursTextCount(); ++index )
   {
      SetHourText( index, g_xmlHelper.GetHourText( index ).c_str() );
   }

   for ( UINT index = 0; index < g_xmlHelper.GetTimesTextCount(); ++index )
   {
      SetTimeText( index, g_xmlHelper.GetTimeText( index ).c_str() );
   }

   Hook( GetTrayClock() );

   MSG msg;

   HICON hTrayIcon = (HICON)LoadImage( hInstance, MAKEINTRESOURCE( IDI_SMALL ),
      IMAGE_ICON, 0, 0, 0 );

   AddTrayIcon( g_hWnd, g_xmlHelper.GetApplicationName().c_str(), hTrayIcon, IDR_TRAYMENU );

   SetProcessWorkingSetSize( GetCurrentProcess(), -1, -1 );

   while ( GetMessage( &msg, NULL, 0, 0 ) )
   {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
   }

   RemoveTrayIcon();
   DestroyIcon( hTrayIcon );

   Unhook();

   return (int)msg.wParam;
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
   wcex.hbrBackground      = (HBRUSH)( COLOR_WINDOW + 1 );
   wcex.lpszMenuName       = MAKEINTRESOURCE( IDC_FUZZYCLOCK );
   wcex.lpszClassName      = g_szWindowClass;
   wcex.hIconSm            = (HICON)LoadImage( hInstance, MAKEINTRESOURCE( IDI_SMALL ), IMAGE_ICON, 0, 0, 0 );

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
         wcscpy_s( buffer, 64, g_xmlHelper.GetExitText().c_str() );

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


tstring GetXMLFile()
{
   WIN32_FIND_DATAW findFileData;

   DWORD numChars = GetCurrentDirectory( 0, NULL );

   std::vector<TCHAR> szDirectory( numChars );

   if ( GetCurrentDirectory( numChars, &szDirectory[0] ) != ( numChars - 1 ) )
   {
      return _T("");
   }

   tstring strDirectory( &szDirectory[0] );

   tstring strFilePath = strDirectory + _T("\\") + _T("FuzzyClock.xml");

   HANDLE hFind = FindFirstFile( strFilePath.c_str(), &findFileData );

   if ( INVALID_HANDLE_VALUE == hFind )
   {
      return GetDefaultXMLFile( strFilePath.c_str() );
   }

   FindClose( hFind );

   return strFilePath;
}


tstring GetDefaultXMLFile( LPCTSTR szDefaultPath )
{
   TCHAR szResourceID[16];
   szResourceID[0] = _T('#');
   _itot_s( IDR_XMLDEFAULT, &szResourceID[1], 15, 10 );

   LPCTSTR szResourceType = _T("XMLFILE");

   HRSRC hres = FindResource( NULL, szResourceID, szResourceType );

   if ( NULL == hres )
   {
      return _T("");
   }

   DWORD sizeResource = SizeofResource( NULL, hres );

   HGLOBAL hbytes = LoadResource( NULL, hres );

   LPVOID pData = LockResource( hbytes );

   tstring strFilePath;

   HANDLE hfile = CreateFile( szDefaultPath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

   if ( INVALID_HANDLE_VALUE == hfile )
   {
      int numChars = GetTempPath( 0, NULL );

      std::vector<TCHAR> szTempDir( numChars );

      if ( GetTempPath( numChars, &szTempDir[0] ) != ( numChars - 1 ) )
      {
         return _T("");
      }

      TCHAR szTempFileName[MAX_PATH+1];

      GetTempFileName( &szTempDir[0], _T("FUZ"), 0, szTempFileName );

      hfile = CreateFile( szTempFileName, GENERIC_READ | GENERIC_WRITE, 0, NULL,
         CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

      if ( INVALID_HANDLE_VALUE == hfile )
      {
         return _T("");
      }

      strFilePath = szTempFileName;
   }
   else
   {
      strFilePath = szDefaultPath;
   }

   DWORD dwBytesWritten = 0;

   WriteFile( hfile, pData, sizeResource, &dwBytesWritten, NULL );

   CloseHandle( hfile );

   return strFilePath;
}