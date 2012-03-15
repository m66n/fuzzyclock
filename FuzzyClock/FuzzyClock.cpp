// Copyright (c) 2007-2012  Michael Chapman
// http://fuzzyclock.googlecode.com
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
#include "AutoStart.h"
#include "Config.h"
#include <ShellAPI.h>
#include <vector>
#include <string>
#include <boost/scoped_ptr.hpp>

#ifdef _UNICODE
typedef std::wstring tstring;
#else
typedef std::string tstring;
#endif

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

boost::scoped_ptr<Config> config_;
boost::scoped_ptr<AutoStart> autoStart_;

const UINT RWM_IDENTITY = RegisterWindowMessage( _T("RWM_IDENTITY__F4252D21_27F7_4d84_AE3B_48156BC571BC") );
const UINT RWM_TRAYICON = RegisterWindowMessage( _T("RWM_TRAYICON__C363ED38_3BEA_477b_B407_2A235F89F4E7") );

const UINT RWM_TASKBARCREATED = RegisterWindowMessage( _T("TaskbarCreated") );


const wchar_t* APP_NAME = L"FuzzyClock";
const wchar_t* CONFIG_FILE = L"config";
const wchar_t* SHORTCUT_NAME = L"FuzzyClock";
const wchar_t* SHORTCUT_DESC = L"A fuzzy clock (à la KDE) for the Windows tray";


BOOL AddTrayIcon( HWND, LPCWSTR, HICON, UINT );
void ApplyConfig();
void CALLBACK DelayedSingleClick( HWND, UINT, UINT_PTR, DWORD );
BOOL CALLBACK EnumWindowsProc( HWND, LPARAM );
tstring GetDefaultXMLFile( LPCTSTR );
HWND GetTrayClock();
tstring GetXMLFile();
BOOL InitInstance( HINSTANCE, int );
LRESULT OnIdentity( WPARAM, LPARAM );
LRESULT OnTaskbarCreated( WPARAM, LPARAM );
LRESULT OnTrayIcon( WPARAM, LPARAM );
BOOL ProcessXMLFile( LPCWSTR );
ATOM RegisterWindow( HINSTANCE );
BOOL RemoveTrayIcon();
void SetTrayIconText( LPCWSTR );
void ToggleStart();
LRESULT CALLBACK WndProc( HWND, UINT, WPARAM, LPARAM );


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
      HWND hRunning = NULL;

      EnumWindows( EnumWindowsProc, (LPARAM)&hRunning );

      if ( ( NULL != hRunning ) && ( _tcslen( lpCmdLine ) > 0 ) )
      {
         int argsCount = 0;

         LPWSTR* szArgList = CommandLineToArgvW( GetCommandLineW(), &argsCount );

         if ( argsCount > 1 )
         {
            COPYDATASTRUCT cds;

            cds.dwData = LOAD_XML;
            cds.cbData = ( (int)wcslen( szArgList[1] ) + 1 ) * sizeof( wchar_t );
            cds.lpData = szArgList[1];

            DWORD_PTR result = 0;
            SendMessageTimeout( hRunning, WM_COPYDATA, 0, (LPARAM)&cds,
               SMTO_BLOCK | SMTO_ABORTIFHUNG, 100, &result );
         }

         LocalFree( szArgList );
      }

      return FALSE;
   }

   LoadString( hInstance, IDS_APP_TITLE, g_szTitle, MAX_LOADSTRING );
   LoadString( hInstance, IDC_FUZZYCLOCK, g_szWindowClass, MAX_LOADSTRING );

   RegisterWindow( hInstance );

   if ( !InitInstance( hInstance, nCmdShow ) )
   {
      return FALSE;
   }

   config_.reset(new Config(APP_NAME, CONFIG_FILE));

   tstring xmlFile = GetXMLFile();

   if ( xmlFile.empty() )
   {
      return FALSE;
   }

   if ( !ProcessXMLFile( xmlFile.c_str() ) )
   {
      return FALSE;
   }

   Hook( GetTrayClock() );

   ApplyConfig();

   autoStart_.reset(new AutoStart(SHORTCUT_NAME, AutoStart::GetAppPath().c_str(),
         AutoStart::GetWorkingDir().c_str(), SHORTCUT_DESC));

   MSG msg;

   g_hTrayIcon = (HICON)LoadImage( hInstance, MAKEINTRESOURCE( IDI_SMALL ),
      IMAGE_ICON, GetSystemMetrics( SM_CXSMICON ), GetSystemMetrics( SM_CYSMICON ), 0 );

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
   DWORD_PTR result = 0;

   BOOL success = (BOOL)SendMessageTimeout( hWnd, RWM_IDENTITY, 0, 0,
      SMTO_BLOCK | SMTO_ABORTIFHUNG, 100, &result );

   if ( success && RWM_IDENTITY == result )
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


void ApplyConfig()
{
   switch (config_->GetFuzziness())
   {
   case LOWEST:
      PostMessage( GetTrayClock(), RWM_SETFUZZINESS, Lowest, 0 );
      break;
   case LOWER:
      PostMessage( GetTrayClock(), RWM_SETFUZZINESS, Lower, 0 );
      break;
   case HIGHER:
      PostMessage( GetTrayClock(), RWM_SETFUZZINESS, Higher, 0 );
      break;
   case HIGHEST:
      PostMessage( GetTrayClock(), RWM_SETFUZZINESS, Highest, 0 );
      break;
   }

   if ( config_->GetPrecise() == PRECISE &&
         SendMessage( GetTrayClock(), RWM_GETPRECISE, 0, 0 ) == FALSE )
   {
      PostMessage( GetTrayClock(), RWM_TOGGLE, 0, 0 );
   }

}


BOOL ProcessXMLFile( LPCWSTR szFilePath )
{
   if ( !g_xmlHelper.LoadFile( szFilePath ) )
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

   for ( UINT index = 0; index < g_xmlHelper.GetMidTimesTextCount(); ++index )
   {
      SetMidTimeText( index, g_xmlHelper.GetMidTimeText( index ).c_str() );
   }

   for ( UINT index = 0; index < g_xmlHelper.GetHighTimesTextCount(); ++index )
   {
      SetHighTimeText( index, g_xmlHelper.GetHighTimeText( index ).c_str() );
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

         case IDM_START:
            ToggleStart();
            break;

         case ID_FUZZINESS_LOWEST:

            PostMessage( GetTrayClock(), RWM_SETFUZZINESS, Lowest, 0 );
            config_->SetFuzziness(LOWEST);
            break;

         case ID_FUZZINESS_LOWER:

            PostMessage( GetTrayClock(), RWM_SETFUZZINESS, Lower, 0 );
            config_->SetFuzziness(LOWER);
            break;

         case ID_FUZZINESS_HIGHER:

            PostMessage( GetTrayClock(), RWM_SETFUZZINESS, Higher, 0 );
            config_->SetFuzziness(HIGHER);
            break;

         case ID_FUZZINESS_HIGHEST:

            PostMessage( GetTrayClock(), RWM_SETFUZZINESS, Highest, 0 );
            config_->SetFuzziness(HIGHEST);
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
         if ( ProcessXMLFile( (LPCWSTR)(pCDS->lpData) ) )
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


void SetMenuText( HMENU hMenu, UINT id, LPCWSTR szText, BOOL indexed = FALSE )
{
   wchar_t buffer[64];

   MENUITEMINFOW mii;
   mii.cbSize = sizeof( MENUITEMINFOW );
   mii.dwTypeData = buffer;
   mii.cch = 64;

   GetMenuItemInfoW( hMenu, id, indexed, &mii );

   mii.fMask = MIIM_TYPE;
   mii.fType = MFT_STRING;

   wcscpy_s( buffer, 64, szText );

   buffer[63] = L'0';

   SetMenuItemInfoW( hMenu, id, indexed, &mii );
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

         SetMenuDefaultItem( hPopupMenu, IDM_EXIT, FALSE );

         SetMenuText( hPopupMenu, IDM_EXIT, g_xmlHelper.GetExitText().c_str() );

         SetMenuText( hPopupMenu, IDM_START, g_xmlHelper.GetStartText().c_str() );

         if ( autoStart_->IsEnabled() )
         {
            CheckMenuItem( hPopupMenu, IDM_START, MF_CHECKED );
         }

         SetMenuText( hPopupMenu, 0, g_xmlHelper.GetFuzzinessText().c_str(), TRUE );

         HMENU hFuzzinessMenu = GetSubMenu( hPopupMenu, 0 );

         if ( NULL != hFuzzinessMenu )
         {
            SetMenuText( hFuzzinessMenu, ID_FUZZINESS_LOWEST, g_xmlHelper.GetFuzzinessLevelText( 0 ).c_str() );
            SetMenuText( hFuzzinessMenu, ID_FUZZINESS_LOWER, g_xmlHelper.GetFuzzinessLevelText( 1 ).c_str() );
            SetMenuText( hFuzzinessMenu, ID_FUZZINESS_HIGHER, g_xmlHelper.GetFuzzinessLevelText( 2 ).c_str() );
            SetMenuText( hFuzzinessMenu, ID_FUZZINESS_HIGHEST, g_xmlHelper.GetFuzzinessLevelText( 3 ).c_str() );

            UINT level = (UINT)SendMessage( GetTrayClock(), RWM_GETFUZZINESS, 0, 0 );

            CheckMenuRadioItem( hFuzzinessMenu, 0, 3, level, MF_BYPOSITION | MF_CHECKED );
         }

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
      SendMessage( g_hWnd, WM_COMMAND, IDM_EXIT, 0 );
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

   std::wstring strFilePath = Config::GetConfigPath(APP_NAME, L"FuzzyClock.xml");

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
   SendMessage( g_hWnd, WM_COMMAND, MAKEWORD( IDM_EXIT, 0 ), 0 );
   return 0;
}


void CALLBACK DelayedSingleClick( HWND hwnd, UINT, UINT_PTR id, DWORD )
{
   KillTimer( hwnd, id );
   SendMessage( GetTrayClock(), RWM_TOGGLE, 0, 0 );
   if ( SendMessage( GetTrayClock(), RWM_GETPRECISE, 0, 0 ) == TRUE )
   {
      config_->SetPrecise(PRECISE);
   }
   else
   {
      config_->SetPrecise(FUZZY);
   }
}


void ToggleStart()
{
   if ( autoStart_->IsEnabled() )
   {
      autoStart_->Disable();
   }
   else
   {
      autoStart_->Enable();
   }
}
