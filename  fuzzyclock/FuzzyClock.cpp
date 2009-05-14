// Copyright (c) 2009 Michael Chapman (http://fuzzyclock.googlecode.com)
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

#include <fstream>
#include <ios>
#include <string>
#include <vector>


#define MAX_LOADSTRING 100
#define LOAD_XML 32670

#define SHELLTRAY_WNDCLASS _T("Shell_TrayWnd")
#define TRAYNOTIFY_WNDCLASS _T("TrayNotifyWnd")
#define TRAYCLOCK_WNDCLASS _T("TrayClockWClass")

#define TIMER_ID_CLICK 1


HINSTANCE g_hInstance;
HWND g_hWnd;
TCHAR g_szTitle[MAX_LOADSTRING];
TCHAR g_szWindowClass[MAX_LOADSTRING];
NOTIFYICONDATAW g_nid;
HICON g_hTrayIcon = NULL;

XMLHelper g_xmlHelper;

const UINT RWM_IDENTITY = RegisterWindowMessage( _T("RWM_IDENTITY__F4252D21_27F7_4d84_AE3B_48156BC571BC") );
const UINT RWM_TRAYICON = RegisterWindowMessage( _T("RWM_TRAYICON__C363ED38_3BEA_477b_B407_2A235F89F4E7") );

const UINT RWM_TASKBARCREATED = RegisterWindowMessage( _T("TaskbarCreated") );


BOOL AddTrayIcon( HWND, LPCWSTR, HICON, UINT );
std::wstring CreateDefaultXMLFile( LPCWSTR );
void CALLBACK DelayedSingleClick( HWND, UINT, UINT_PTR, DWORD );
BOOL CALLBACK EnumWindowsProc( HWND, LPARAM );
bool FileExists( LPCWSTR );
std::wstring GetAppDataPath();
std::wstring GetArgPath();
HWND GetTrayClock();
std::wstring GetXMLFile();
std::wstring GetXMLPath();
BOOL InitInstance( HINSTANCE, int );
LRESULT OnIdentity( WPARAM, LPARAM );
LRESULT OnTaskbarCreated( WPARAM, LPARAM );
LRESULT OnTrayIcon( WPARAM, LPARAM );
BOOL ProcessXMLFile( LPCWSTR );
ATOM RegisterWindow( HINSTANCE );
BOOL RemoveTrayIcon();
bool ReplaceXMLFile( LPCWSTR );
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );
void SetTrayIconText( LPCWSTR );
void WriteLog( LPCWSTR output );


int APIENTRY _tWinMain( HINSTANCE hInstance,
                        HINSTANCE hPrevInstance,
                        LPTSTR    lpCmdLine,
                        int       nCmdShow )
{
   UNREFERENCED_PARAMETER( hPrevInstance );
   UNREFERENCED_PARAMETER( lpCmdLine );

   std::wstring argPath = GetArgPath();

   HANDLE hMutexSingleInstance = CreateMutex( NULL, FALSE,
      _T("FuzzyClock__8E5405E2_BD48_41cd_AAF4_C7183EA13CBB") );

   if ( GetLastError() == ERROR_ALREADY_EXISTS ||
        GetLastError() == ERROR_ACCESS_DENIED )
   {
      HWND hRunning = NULL;

      EnumWindows( EnumWindowsProc, (LPARAM)&hRunning );

      if ( ( NULL != hRunning ) && !argPath.empty() )
      {
         COPYDATASTRUCT cds;

         cds.dwData = LOAD_XML;
         cds.cbData = (DWORD)(( argPath.length() + 1 ) * sizeof( wchar_t ));
         cds.lpData = (void*)argPath.c_str();

         DWORD_PTR result = NULL;
         SendMessageTimeout( hRunning, WM_COPYDATA, 0, (LPARAM)&cds,
            SMTO_BLOCK | SMTO_ABORTIFHUNG, 100, &result );
      }

      return FALSE;
   }

   LoadString( hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING );
   LoadString( hInstance, IDC_FUZZYCLOCK, g_szWindowClass, MAX_LOADSTRING );

   RegisterWindow( hInstance );

   if ( !InitInstance( hInstance, nCmdShow ) )
   {
      WriteLog( L"InitInstance() failed." );
      return FALSE;
   }

   if ( !argPath.empty() )
   {
      ReplaceXMLFile( argPath.c_str() );
   }

   std::wstring xmlFile = GetXMLFile();

   if ( xmlFile.empty() )
   {
      WriteLog( L"IGetXMLFile() failed." );
      return FALSE;
   }

   if ( !ProcessXMLFile( xmlFile.c_str() ) )
   {
      WriteLog( L"ProcessXMLFile() failed." );
      return FALSE;
   }

   HWND hwndClock = GetTrayClock();

   if ( NULL == hwndClock )
   {
      WriteLog( L"GetTrayClock() failed." );
      return FALSE;
   }

   if ( !Hook( hwndClock ) )
   {
      WriteLog( L"Hook() failed." );
      return FALSE;
   }

   MSG msg;

   g_hTrayIcon = (HICON)LoadImage( hInstance, MAKEINTRESOURCE( IDI_SMALL ),
      IMAGE_ICON, 0, 0, 0 );

   AddTrayIcon( g_hWnd, g_xmlHelper.GetApplicationName().c_str(), g_hTrayIcon, IDR_TRAYMENU );

   SetProcessWorkingSetSize( GetCurrentProcess(), -1, -1 );

   while ( GetMessage( &msg, NULL, 0, 0 ) )
   {
      TranslateMessage( &msg );
      DispatchMessage( &msg );
   }

   RemoveTrayIcon();
   DestroyIcon( g_hTrayIcon );

   Unhook();

   return (int)msg.wParam;
}


BOOL CALLBACK EnumWindowsProc( HWND hWnd, LPARAM lParam )
{
   DWORD_PTR result = NULL;

   BOOL success = (BOOL)SendMessageTimeout( hWnd, RWM_IDENTITY, 0, 0,
      SMTO_BLOCK | SMTO_ABORTIFHUNG, 100, &result );

   if ( success && ( RWM_IDENTITY == result ) )
   {
      HWND* pTarget = (HWND*)lParam;
      *pTarget = hWnd;
      return FALSE;  // window found.  stop search.
   }

   return TRUE;  // ignore this window and continue
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


BOOL ProcessXMLFile( LPCWSTR szFilePath )
{
   std::wstring error;

   if ( !g_xmlHelper.LoadFile( szFilePath, error ) )
   {
      WriteLog( error.c_str() );
      CreateDefaultXMLFile( szFilePath );

      if ( !g_xmlHelper.LoadFile( szFilePath, error ) )
      {
         WriteLog( error.c_str() );
         return FALSE;
      }
   }

   for ( UINT index = 0; index < g_xmlHelper.GetHoursTextCount(); ++index )
   {
      SetHourText( index, g_xmlHelper.GetHourText( index ).c_str() );
   }

   for ( UINT index = 0; index < g_xmlHelper.GetTimesTextCount(); ++index )
   {
      SetTimeText( index, g_xmlHelper.GetTimeText( index ).c_str() );
   }

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

   case WM_COPYDATA:

      PCOPYDATASTRUCT pCDS = (PCOPYDATASTRUCT)lParam;

      if ( pCDS->dwData == LOAD_XML )
      {
         ReplaceXMLFile( (LPCWSTR)(pCDS->lpData) );

         if ( ProcessXMLFile( GetXMLFile().c_str() ) )
         {
            SetTrayIconText( g_xmlHelper.GetApplicationName().c_str() );
            Invalidate();
         }

         return TRUE;
      }

      break;
   }

   if ( RWM_TRAYICON == message )
   {
      return OnTrayIcon( wParam, lParam );
   }
   else if ( RWM_IDENTITY == message )
   {
      return OnIdentity( wParam, lParam );
   }
   else if ( RWM_TASKBARCREATED == message )
   {
      return OnTaskbarCreated( wParam, lParam );
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

         MENUITEMINFOW mii;
         mii.cbSize = sizeof( MENUITEMINFOW );
         mii.dwTypeData = buffer;
         mii.cch = 64;

         GetMenuItemInfoW( hPopupMenu, 0, TRUE, &mii );

         mii.fMask = MIIM_TYPE;
         mii.fType = MFT_STRING;

         wcscpy_s( buffer, 64, g_xmlHelper.GetExitText().c_str() );

         SetMenuItemInfoW( hPopupMenu, 0, TRUE, &mii );

         TrackPopupMenu( hPopupMenu, TPM_LEFTALIGN, cursorPos.x, cursorPos.y, 0, g_hWnd, NULL );

         SendMessage( g_hWnd, WM_NULL, 0, 0 );
      }

      DestroyMenu( hMenu );
   }
   else if ( LOWORD( lParam ) == WM_LBUTTONDOWN )
   {
      // thanks to http://blogs.msdn.com/oldnewthing/archive/2004/10/15/242761.aspx
      //
      SetTimer( g_hWnd, TIMER_ID_CLICK, GetDoubleClickTime(), DelayedSingleClick );
   }
   else if ( LOWORD( lParam ) == WM_LBUTTONDBLCLK )
   {
      KillTimer( g_hWnd, TIMER_ID_CLICK );

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


std::wstring GetXMLFile()
{
   WIN32_FIND_DATAW findFileData;

   std::wstring strFilePath = GetXMLPath();

   HANDLE hFind = FindFirstFileW( strFilePath.c_str(), &findFileData );

   if ( INVALID_HANDLE_VALUE == hFind )
   {
      return CreateDefaultXMLFile( strFilePath.c_str() );
   }

   FindClose( hFind );

   return strFilePath;
}


std::wstring CreateDefaultXMLFile( LPCWSTR szDefaultPath )
{
   WCHAR szResourceID[16];
   szResourceID[0] = L'#';
   _itow_s( IDR_XMLDEFAULT, &szResourceID[1], 15, 10 );

   LPCWSTR szResourceType = L"XMLFILE";

   HRSRC hres = FindResourceW( NULL, szResourceID, szResourceType );

   if ( NULL == hres )
   {
      return L"";
   }

   DWORD sizeResource = SizeofResource( NULL, hres );

   HGLOBAL hbytes = LoadResource( NULL, hres );

   LPVOID pData = LockResource( hbytes );

   // In case an invalid existing file is read-only
   //
   SetFileAttributesW( szDefaultPath, FILE_ATTRIBUTE_NORMAL );

   HANDLE hfile = CreateFileW( szDefaultPath, GENERIC_READ | GENERIC_WRITE, 0, NULL,
      CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL );

   if ( INVALID_HANDLE_VALUE == hfile )
   {
      return L"";
   }

   DWORD dwBytesWritten = 0;

   WriteFile( hfile, pData, sizeResource, &dwBytesWritten, NULL );

   CloseHandle( hfile );

   return szDefaultPath;
}


LRESULT OnIdentity( WPARAM wParam, LPARAM lParam )
{
   return RWM_IDENTITY;
}


void SetTrayIconText( LPCWSTR szText )
{
   g_nid.uFlags = NIF_TIP;

   wcscpy_s( g_nid.szTip, sizeof( g_nid.szTip ), szText );

   Shell_NotifyIconW( NIM_MODIFY, &g_nid );
}


LRESULT OnTaskbarCreated( WPARAM, LPARAM )
{
   // This is a kludge for when explorer.exe crashes.  Calling Hook() again did not
   // appear to work.  Program just exits instead.
   //
   SendMessage( g_hWnd, WM_COMMAND, MAKEWORD( IDM_EXIT, 0 ), 0 );
   return 0;
}


void CALLBACK DelayedSingleClick( HWND hwnd, UINT, UINT_PTR id, DWORD )
{
   KillTimer( hwnd, id );
   PostMessage( GetTrayClock(), RWM_TOGGLE, 0, 0 );
}


std::wstring GetAppDataPath()
{
   std::wstring path;

   WCHAR szPath[MAX_PATH];

   HRESULT hr = SHGetFolderPathW( NULL, CSIDL_APPDATA, NULL,
      SHGFP_TYPE_CURRENT, szPath );

   if ( SUCCEEDED( hr ) )
   {
      PathAppendW( szPath, L"FuzzyClock" );
      SHCreateDirectoryExW( NULL, szPath, NULL );
      path = szPath;
   }

   return path;
}


std::wstring GetArgPath()
{
   std::wstring path;

   int argsCount = 0;

   LPWSTR* szArgList = CommandLineToArgvW( GetCommandLineW(), &argsCount );

   if ( argsCount > 1 )
   {
      if ( FileExists( szArgList[1] ) )
      {
         path = szArgList[1];
      }
   }

   LocalFree( szArgList );

   return path;
}


std::wstring GetXMLPath()
{
   WCHAR szPath[MAX_PATH];
   
   size_t numChars = GetAppDataPath().copy( szPath, MAX_PATH );

   szPath[numChars] = L'\0';

   PathAppendW( szPath, L"FuzzyClock.xml" );

   std::wstring path( szPath );

   return path;
}


bool ReplaceXMLFile( LPCWSTR newPath )
{
   std::wstring error;

   // Validate XML by loading it
   //
   if ( !g_xmlHelper.LoadFile( newPath, error ) )
   {
      WriteLog( error.c_str() );
      return false;
   }

   std::wstring existingPath = GetXMLPath();

   return ( CopyFileW( newPath, existingPath.c_str(), FALSE ) != 0 );
}


bool FileExists( LPCWSTR path )
{
   WIN32_FIND_DATAW findFileData;

   HANDLE hFind = FindFirstFileW( path, &findFileData );

   bool exists = false;
   
   if ( INVALID_HANDLE_VALUE != hFind )
   {
      exists = true;
      FindClose( hFind );
   }

   return exists;
}


void WriteLog( LPCWSTR output )
{
   WCHAR logpath[MAX_PATH];
   wcscpy_s( logpath, MAX_PATH, GetAppDataPath().c_str() );

   PathAppendW( logpath, L"FuzzyClock.log" );

   try
   {
      std::wofstream logstream( logpath, std::ios::app );
      logstream << output << std::endl;
      logstream << L"GetLastError() returns " << GetLastError() << L"." << std::endl;
   }
   catch (...)
   {

   }
}