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
#include "FuzzyHook.h"
//#include "WriteLog.h"  // for debugging
#include <string>
#include <vector>


#pragma data_seg(".fuzzy")
BOOL g_subclassed = FALSE;
UINT RWM_FUZZYHOOK = 0;
HWND g_hWnd	= NULL;
HHOOK g_hHook = NULL;
wchar_t g_szHoursText[HT_STRING_SIZE * HT_COUNT] = L"";
wchar_t g_szTimesText[TT_STRING_SIZE * TT_COUNT] = L"";
wchar_t g_szMidTimesText[MID_STRING_SIZE * MID_COUNT] = L"";
wchar_t g_szHighTimesText[HIGH_STRING_SIZE * HIGH_COUNT] = L"";
#pragma data_seg()

#pragma comment(linker,"/SECTION:.fuzzy,RWS")


#define FUZZYHOOK_HOOK 1
#define FUZZYHOOK_UNHOOK 2

#define TIMER_ID_PRECISE 1
#define TIMER_ELAPSE_TIME 200

#define TIMER_ID_FUZZY 2
#define TIMER_ELAPSE_FUZZY 5000


HINSTANCE g_hInst;
WNDPROC g_wndProcOld;

COLORREF g_clrForeground = COLOR_BTNTEXT;
HFONT g_hFont = NULL;
int g_widthClock = -1;

HDC g_hdcClock = NULL;
HBITMAP g_hbmpClock = NULL;
HDC g_hdcClockBackground = NULL;
HBITMAP g_hbmpClockBackground = NULL;

bool g_bShowFuzzy = true;
std::wstring g_strTime;
int g_sector = 0;

FuzzinessLevel g_level = Lowest;


LRESULT CalculateWindowSize( HWND );
void Cleanup();
void ClearClockDC();
void CopyParentSurface( HWND, HDC, int, int, int, int, int, int );
bool CreateClockHDC( HWND );
bool CreateMemoryDC( HDC, HDC&, HBITMAP&, int, int );
void DrawFuzzyClock( HWND, HDC );
int GetFuzzyTime( std::wstring& );
int GetFuzzyTimeSector( const SYSTEMTIME& );
void GetTextSize( HDC, LPCWSTR, TEXTMETRIC&, SIZE& );
void GetThemeSettings( COLORREF&, HFONT& );
void GetPreciseTime( std::wstring& );
LRESULT _stdcall HookProc( int, WPARAM, LPARAM );
LRESULT CALLBACK NewWndProc( HWND, UINT, WPARAM, LPARAM );
LRESULT OnMouseDown( HWND, UINT, WPARAM, LPARAM );
LRESULT OnMouseUp( HWND, UINT, WPARAM, LPARAM );
void RefreshTaskbar( HWND );
void SetTime();

int GetLowerFuzzyTimeSector( const SYSTEMTIME& );
int GetHighFuzzyTime( const SYSTEMTIME&, std::wstring& );
int GetMidFuzzyTime( const SYSTEMTIME&, std::wstring& );
int GetMidFuzzyTimeSector( const SYSTEMTIME& );
int GetHighFuzzyTimeSector( const SYSTEMTIME& );


BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD   ul_reason_for_call,
                       LPVOID  lpReserved )
{
   if ( DLL_PROCESS_ATTACH == ul_reason_for_call )
   {
      g_hInst = (HINSTANCE)hModule;
      DisableThreadLibraryCalls( g_hInst );

      if ( 0 == RWM_FUZZYHOOK )
      {
         RWM_FUZZYHOOK = RegisterWindowMessage( _T("RWM_FUZZYHOOK__B78C168A_5AE4_405c_A8B6_B30FB917567A") );
      }
   }

   return TRUE;
}


FUZZYHOOK_API BOOL Hook( HWND hWnd )
{
   g_hWnd = hWnd;

   g_hHook = SetWindowsHookEx( WH_CALLWNDPROC,(HOOKPROC)HookProc, g_hInst, GetWindowThreadProcessId( hWnd, NULL ) );

   if ( NULL != g_hHook )
   {
      SendMessage( hWnd, RWM_FUZZYHOOK, 0, FUZZYHOOK_HOOK );
   }

   PostMessage( hWnd, WM_SIZE, SIZE_RESTORED, 0 );

   return g_subclassed;
}


FUZZYHOOK_API BOOL Unhook()
{
   g_hHook = SetWindowsHookEx( WH_CALLWNDPROC, (HOOKPROC)HookProc, g_hInst, GetWindowThreadProcessId( g_hWnd, NULL ) );

   if ( NULL != g_hHook )
   {
      SendMessage( g_hWnd, RWM_FUZZYHOOK, 0, FUZZYHOOK_UNHOOK );
   }

   return !g_subclassed;
}


FUZZYHOOK_API void SetHourText( int index, LPCWSTR szHourText )
{
   if ( index >= 0 && index < HT_COUNT )
   {
      wcscpy_s( &g_szHoursText[index * HT_STRING_SIZE], HT_STRING_SIZE, szHourText );
   }
}


FUZZYHOOK_API void SetTimeText( int index, LPCWSTR szTimeText )
{
   if ( index >= 0 && index < TT_COUNT )
   {
      wcscpy_s( &g_szTimesText[index * TT_STRING_SIZE], TT_STRING_SIZE, szTimeText );
   }
}


FUZZYHOOK_API void SetMidTimeText( int index, LPCWSTR szMidTimeText )
{
   if ( index >= 0 && index < MID_COUNT )
   {
      wcscpy_s( &g_szMidTimesText[index * MID_STRING_SIZE], MID_STRING_SIZE, szMidTimeText );
   }
}


FUZZYHOOK_API void SetHighTimeText( int index, LPCWSTR szHighTimeText )
{
   if ( index >= 0 && index < HIGH_COUNT )
   {
      wcscpy_s( &g_szHighTimesText[index * HIGH_STRING_SIZE], HIGH_STRING_SIZE, szHighTimeText );
   }
}


FUZZYHOOK_API void Invalidate()
{
   PostMessage( g_hWnd, WM_TIMECHANGE, 0, 0 );
}


LRESULT _stdcall HookProc( int code, WPARAM wParam, LPARAM lParam )
{
   LPCWPSTRUCT pCWP = reinterpret_cast<LPCWPSTRUCT>(lParam);

   if ( NULL != pCWP && RWM_FUZZYHOOK == pCWP->message )
   {
      if ( FUZZYHOOK_HOOK == pCWP->lParam )
      {
         UnhookWindowsHookEx( g_hHook );

         if ( !g_subclassed )
         {
            TCHAR dllPath[MAX_PATH+1];
            GetModuleFileName( g_hInst, dllPath, MAX_PATH+1 );

            if ( LoadLibrary( dllPath ) )
            {
               g_wndProcOld = (WNDPROC)SetWindowLongPtr( g_hWnd, GWLP_WNDPROC, (LONG_PTR)NewWndProc );

               if ( NULL == g_wndProcOld )
               {
                  FreeLibrary( g_hInst );
               }
               else
               {
                  g_subclassed = TRUE;
                  GetThemeSettings( g_clrForeground, g_hFont );
                  InvalidateRect( g_hWnd, NULL, TRUE );
               }
            }
         }
      }
      else
      {
         KillTimer( g_hWnd, TIMER_ID_FUZZY );
         KillTimer( g_hWnd, TIMER_ID_PRECISE );

         g_bShowFuzzy = true;
         SetTime();

         UnhookWindowsHookEx( g_hHook );

         if ( SetWindowLongPtr( g_hWnd, GWLP_WNDPROC, (LONG_PTR)g_wndProcOld ) )
         {
            Cleanup();
            FreeLibrary( g_hInst );
            g_subclassed = FALSE;
            RefreshTaskbar( g_hWnd );
         }   
      }
   }

   return ::CallNextHookEx( g_hHook, code, wParam, lParam );
}


LRESULT CALLBACK NewWndProc( HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam )
{
   if ( RWM_FUZZYHOOK == message )
   {
      if ( FUZZYHOOK_HOOK == lParam )
      {  
         g_bShowFuzzy = true;
         SetTime();
         SetTimer( hWnd, TIMER_ID_FUZZY, TIMER_ELAPSE_FUZZY, NULL );
      }

      return 0;
   }
   else if ( RWM_TOGGLE == message )
   {
      g_bShowFuzzy = !g_bShowFuzzy;

      if ( g_bShowFuzzy )
      {
         KillTimer( hWnd, TIMER_ID_PRECISE );
         SetTimer( hWnd, TIMER_ID_FUZZY, TIMER_ELAPSE_FUZZY, NULL );
      }
      else
      {
         KillTimer( hWnd, TIMER_ID_FUZZY );
         SetTimer( hWnd, TIMER_ID_PRECISE, TIMER_ELAPSE_TIME, NULL );
      }

      SetTime();
      InvalidateRect( hWnd, NULL, TRUE );

      return 0;
   }
   else if ( RWM_SETFUZZINESS == message )
   {
      if ( wParam != g_level )
      {
         g_level = (FuzzinessLevel)wParam;

         if ( g_bShowFuzzy )
         {
            SetTime();
            InvalidateRect( hWnd, NULL, TRUE );
         }
      }

      return 0;
   }
   else if ( RWM_GETFUZZINESS == message )
   {
      return g_level;
   }
   else if ( RWM_GETPRECISE == message )
   {
      return g_bShowFuzzy ? FALSE : TRUE;
   }

   switch ( message )
   {
   case WM_PAINT:

      PAINTSTRUCT ps;
      HDC hDC;

      hDC = BeginPaint( hWnd, &ps );
      DrawFuzzyClock( hWnd, hDC );
      EndPaint( hWnd, &ps );

      return 0;

   case WM_SIZE:

      CreateClockHDC( hWnd );
      InvalidateRect( hWnd, NULL, TRUE );
      return 0;

   case WM_USER+100:

      return CalculateWindowSize( hWnd );

   case WM_SETTINGCHANGE:
   case WM_SYSCOLORCHANGE:

      GetThemeSettings( g_clrForeground, g_hFont );
      CreateClockHDC( hWnd );
      InvalidateRect( hWnd, NULL, TRUE );
      return 0;

   case WM_TIMECHANGE:
   case WM_USER + 101:
   case WM_SETFOCUS:
   case WM_KILLFOCUS:

      SetTime();
      InvalidateRect( hWnd, NULL, FALSE );
      return 0;

   case WM_TIMER:

      switch ( wParam )
      {
      case 0:
         break;

      case TIMER_ID_FUZZY:
         
         {
            SYSTEMTIME systemTime;
            GetLocalTime( &systemTime );

            if ( GetFuzzyTimeSector( systemTime ) == g_sector )
            {
               return 0;
            }

            g_sector = GetFuzzyTime( g_strTime );
         }

         break;

      case TIMER_ID_PRECISE:

         std::wstring temp( g_strTime );
         GetPreciseTime( g_strTime );

         if ( temp == g_strTime )
         {
            return 0;
         }

         break;
      }

      InvalidateRect( hWnd, NULL, FALSE );
      return 0;

   case WM_NCHITTEST:

      return HTTRANSPARENT;
   }

   return CallWindowProc( g_wndProcOld, hWnd, message, wParam, lParam );
}


LRESULT CalculateWindowSize( HWND hwnd )
{
   TEXTMETRIC tm;
   HDC hdc;
   HFONT hOldFont;
   int wclock, hclock;

   if ( !( GetWindowLong( hwnd, GWL_STYLE ) & WS_VISIBLE ) )
   {
      return 0;
   }

   hdc = GetDC( hwnd );

   if ( g_hFont )
   {
      hOldFont = (HFONT)SelectObject( hdc, g_hFont );
   }

   GetTextMetrics( hdc, &tm );

   SIZE sizeText;

   GetTextSize( hdc, g_strTime.c_str(), tm, sizeText );

   wclock = sizeText.cx;
   hclock = sizeText.cy;

   wclock += tm.tmAveCharWidth * 2;
   hclock += ( tm.tmHeight - tm.tmInternalLeading ) / 2;

   if ( hclock < 4 )
   {
      hclock = 4;
   }

   if ( wclock > g_widthClock )
   {
      g_widthClock = wclock;
   }

   if ( g_hFont )
   {
      SelectObject( hdc, hOldFont );
   }

   ReleaseDC( hwnd, hdc );

   return ( hclock << 16 ) + wclock;
}


void DrawFuzzyClock( HWND hWnd, HDC hDC )
{
   if ( NULL == g_hdcClock )
   {
      if ( !CreateClockHDC( hWnd ) )
      {
         return;
      }
   }

   RECT rc;
   GetClientRect( hWnd, &rc );

   BitBlt( g_hdcClock, 0, 0, rc.right, rc.bottom, g_hdcClockBackground, 0, 0, SRCCOPY );

   TEXTMETRIC textMetric;
   GetTextMetrics( g_hdcClock, &textMetric );

   SIZE sizeText;

   GetTextSize( g_hdcClock, g_strTime.c_str(), textMetric, sizeText );

   int x = rc.right / 2;
   int y = ( rc.bottom - sizeText.cy ) / 2 - textMetric.tmInternalLeading / 2;

   TextOutW( g_hdcClock, x, y, g_strTime.c_str(), (int)g_strTime.size() );

   BitBlt( hDC, 0, 0, rc.right, rc.bottom, g_hdcClock, 0, 0, SRCCOPY );

   if ( sizeText.cx + textMetric.tmAveCharWidth * 2 != g_widthClock )
   {
      g_widthClock = sizeText.cx + textMetric.tmAveCharWidth * 2;
      PostMessage( GetParent( GetParent( hWnd ) ), WM_SIZE, SIZE_RESTORED, 0 );
      InvalidateRect( GetParent( GetParent( hWnd ) ), NULL, TRUE );
   }
}


void Cleanup()
{
   if ( NULL != g_hFont )
   {
      DeleteObject( g_hFont );
      g_hFont = NULL;
   }

   ClearClockDC();
}


bool CreateMemoryDC( HDC hDC, HDC& hdcMem, HBITMAP& hBMP, int width, int height )
{
   hdcMem = CreateCompatibleDC( hDC );

   if ( NULL == hdcMem )
   {
      hBMP = NULL;
      return false;
   }

   hBMP = CreateCompatibleBitmap( hDC, width, height );

   if ( NULL == hBMP )
   {
      DeleteDC( hdcMem );
      hdcMem = NULL;
      return false;
   }

   SelectObject( hdcMem, hBMP );

   return true;
}


void CopyParentSurface( HWND hWnd, HDC hDC, int xdst, int ydst, int w, int h, int xsrc, int ysrc )
{
   HDC hdcTemp;
   HDC hdcMem;
   HBITMAP hBMP;
   RECT rcParent;

   GetWindowRect( GetParent( hWnd), &rcParent );

   hdcTemp = GetDC( NULL );

   if ( !CreateMemoryDC( hdcTemp, hdcMem, hBMP, rcParent.right - rcParent.left, rcParent.bottom - rcParent.top ) )
   {
      ReleaseDC( NULL, hdcTemp );
      return;
   }

   SendMessage( GetParent( hWnd ), WM_PRINTCLIENT, (WPARAM)hdcMem, (LPARAM)PRF_CLIENT );

   BitBlt( hDC, xdst, ydst, w, h, hdcMem, xsrc, ysrc, SRCCOPY );

   DeleteDC( hdcMem );
   DeleteObject( hBMP );

   ReleaseDC( NULL, hdcTemp );
}


void ClearClockDC()
{
   g_widthClock = -1;

   if ( NULL != g_hdcClock )
   {
      DeleteDC( g_hdcClock );
      g_hdcClock = NULL;
   }

   if ( NULL != g_hbmpClock)
   {
      DeleteObject( g_hbmpClock );
      g_hbmpClock = NULL;
   }

   if ( NULL != g_hdcClockBackground )
   { 
      DeleteDC( g_hdcClockBackground ); 
      g_hdcClockBackground = NULL;
   }

   if ( NULL != g_hbmpClockBackground )
   {
      DeleteObject( g_hbmpClockBackground );
      g_hbmpClockBackground = NULL;
   }
}


bool CreateClockHDC( HWND hWnd )
{
   ClearClockDC();

   RECT rc;
   GetClientRect( hWnd, &rc );

   HDC hDC = GetDC( NULL );

   if ( !CreateMemoryDC( hDC, g_hdcClock, g_hbmpClock, rc.right, rc.bottom ) )
   {
      ReleaseDC( NULL, hDC );
      return false;
   }

   SelectObject( g_hdcClock, g_hFont );

   SetBkMode( g_hdcClock, TRANSPARENT );

   SetTextAlign( g_hdcClock, TA_CENTER | TA_TOP );

   SetTextColor( g_hdcClock, g_clrForeground );

   if ( !CreateMemoryDC( hDC, g_hdcClockBackground, g_hbmpClockBackground, rc.right, rc.bottom ) )
   {
      ClearClockDC();
      ReleaseDC( NULL, hDC );
      return false;
   }

   RECT rcWnd;
   RECT rcTray;

   GetWindowRect( hWnd, &rcWnd );
   GetWindowRect( GetParent( hWnd ), &rcTray );

   POINT pt = { rcWnd.right, rcWnd.bottom };
   ScreenToClient( hWnd, &pt );

   CopyParentSurface( hWnd, g_hdcClockBackground, 0, 0, pt.x, pt.y, rcWnd.left - rcTray.left, rcWnd.top - rcTray.top );

   ReleaseDC( NULL, hDC );

   return true;
}


void GetTextSize( HDC hDC, LPCWSTR szText, TEXTMETRIC& textMetric, SIZE& size )
{
   size.cx = 0;
   size.cy = 0;

   int heightFont = textMetric.tmHeight - textMetric.tmInternalLeading;

   if ( GetTextExtentPoint32W( hDC, szText, (int)wcslen( szText ), &size ) == 0 )
   {
      size.cx = (int)wcslen( szText ) * textMetric.tmAveCharWidth;
   }

   size.cy = heightFont;
}


void RefreshTaskbar( HWND hwndClock )
{
   HWND hwndTaskbar, hwndTray;

   hwndTray = GetParent( hwndClock );
   hwndTaskbar = GetParent( hwndTray );

   InvalidateRect( hwndTray, NULL, TRUE );
   PostMessage( hwndTray, WM_SIZE, SIZE_RESTORED, 0 );

   InvalidateRect( hwndTaskbar, NULL, TRUE );
   PostMessage( hwndTaskbar, WM_SIZE, SIZE_RESTORED, 0 );

   PostMessage( hwndClock, WM_TIMECHANGE, 0, 0 );
}


void GetThemeSettings( COLORREF& clr, HFONT& hFont )
{
   clr = COLOR_BTNTEXT;

   if ( NULL != hFont )
   {
      DeleteObject( hFont );
      hFont = NULL;
   }

   HMODULE hMod = LoadLibrary( _T("uxtheme.dll") );

   if ( NULL != hMod )
   {
      typedef HANDLE WINAPI OPENTHEMEDATA( HWND, LPCWSTR );
      typedef HRESULT WINAPI GETTHEMECOLOR( HANDLE, int, int, int, COLORREF* );
      typedef HRESULT WINAPI GETTHEMEFONT( HANDLE, HDC, int, int, int, LOGFONT* );
      typedef HRESULT WINAPI CLOSETHEMEDATA( HANDLE );

      OPENTHEMEDATA* pOPENTHEMEDATA = reinterpret_cast< OPENTHEMEDATA* >(
         GetProcAddress( hMod, ( "OpenThemeData" ) ) );
      GETTHEMECOLOR* pGETTHEMECOLOR = reinterpret_cast< GETTHEMECOLOR* >(
         GetProcAddress( hMod, ( "GetThemeColor" ) ) );
      GETTHEMEFONT* pGETTHEMEFONT = reinterpret_cast< GETTHEMEFONT* >(
         GetProcAddress( hMod, ( "GetThemeFont" ) ) );
      CLOSETHEMEDATA* pCLOSETHEMEDATA = reinterpret_cast< CLOSETHEMEDATA* >(
         GetProcAddress( hMod, ( "CloseThemeData" ) ) );

      //TMT_TEXTCOLOR 3803
      //CLP_TIME 1
      //CLS_NORMAL 1

      if ( NULL != pOPENTHEMEDATA )
      {
         HANDLE hTheme = pOPENTHEMEDATA( g_hWnd, L"CLOCK" );

         if ( hTheme )
         {
            if ( NULL != pGETTHEMECOLOR )
            {
               COLORREF clrTheme;

               if ( SUCCEEDED( pGETTHEMECOLOR( hTheme, 1, 1, 3803, &clrTheme ) ) )
               {
                  clr = clrTheme;
               }
            }

            if ( NULL != pGETTHEMEFONT )
            {
               LOGFONT logFont;

               if ( SUCCEEDED( pGETTHEMEFONT( hTheme, NULL, 1, 1, 210, &logFont ) ) )
               {
                  hFont = CreateFontIndirect( &logFont );
               }
            }

            if ( NULL != pCLOSETHEMEDATA )
            {
               pCLOSETHEMEDATA( hTheme );
            }
         }
      }

      FreeLibrary( hMod );
   }

   if ( NULL == hFont )
   {
      NONCLIENTMETRICS ncm;
      ncm.cbSize = sizeof( ncm );
      SystemParametersInfo( SPI_GETNONCLIENTMETRICS, ncm.cbSize, &ncm, 0 );
      ncm.lfCaptionFont.lfWeight = FW_NORMAL;
      hFont = CreateFontIndirect( &(ncm.lfCaptionFont) );
   }
}


void GetPreciseTime( std::wstring& strTime )
{
   int numChars = GetTimeFormatW( NULL, 0, NULL, NULL, NULL, 0 );

   std::vector<WCHAR> buffer( numChars );

   int result = GetTimeFormatW( NULL, 0, NULL, NULL, &(buffer[0]), numChars );

   strTime = &(buffer[0]);
}


int GetFuzzyTimeSector( const SYSTEMTIME& systemTime )
{
   switch ( g_level )
   {
   case Lower:
      return GetLowerFuzzyTimeSector( systemTime );

   case Higher:
      return GetMidFuzzyTimeSector( systemTime );

   case Highest:
      return GetHighFuzzyTimeSector( systemTime );
   }

   int sector = 0;

   int seconds = systemTime.wMinute * 60 + systemTime.wSecond;

   if ( seconds > 150 )
   {
      sector = ( seconds - 150 ) / 300 + 1;
   }

   return sector;
}


int GetFuzzyTime( std::wstring& strFuzzyTime )
{
   SYSTEMTIME systemTime;
   GetLocalTime( &systemTime );

   int sector = GetFuzzyTimeSector( systemTime );

   switch ( g_level )
   {
   case Lower:
      sector = GetLowerFuzzyTimeSector( systemTime );
      break;
   case Higher:
      return GetMidFuzzyTime( systemTime, strFuzzyTime );
   case Highest:
      return GetHighFuzzyTime( systemTime, strFuzzyTime );
   }

   std::wstring timeString( &g_szTimesText[sector * TT_STRING_SIZE] );

   size_t startIndex = timeString.find( L"%" );

   if ( std::wstring::npos != startIndex )
   {
      int realHour = 0;

      size_t endIndex = timeString.find_first_of( L"01", startIndex );

      if ( std::wstring::npos == endIndex )
      {
         endIndex = timeString.length();
      }

      size_t fieldLength = endIndex - startIndex + 1;

      int deltaHour = _wtoi( timeString.substr( startIndex + 1, fieldLength - 1  ).c_str() );

      if ( ( systemTime.wHour + deltaHour ) % 12 > 0 )
      {
         realHour = ( systemTime.wHour + deltaHour ) % 12 - 1;
      }
      else
      {
         realHour = 12 - ( ( systemTime.wHour + deltaHour ) % 12 + 1 );
      }

      timeString.replace( startIndex, fieldLength, &g_szHoursText[realHour * HT_STRING_SIZE] );

      wchar_t capitalizedFirstChar[2];

      LCMapStringW( LOCALE_SYSTEM_DEFAULT, LCMAP_UPPERCASE, timeString.c_str(), 1, capitalizedFirstChar, 2 );
      capitalizedFirstChar[1] = L'\0';

      timeString.replace( 0, 1, capitalizedFirstChar );

      strFuzzyTime = timeString.c_str();
   }

   return sector;
}


void SetTime()
{
   if ( g_bShowFuzzy )
   {
      g_sector = GetFuzzyTime( g_strTime );
   }
   else
   {
      GetPreciseTime( g_strTime );
   }
}


int GetLowerFuzzyTimeSector( const SYSTEMTIME& systemTime )
{
   int sector = 0;

   int seconds = systemTime.wMinute * 60 + systemTime.wSecond;

   if ( seconds > 450 )
   {
      sector = ( ( seconds - 450 ) / 900 + 1 ) * 3;
   }

   return sector;
}


int GetMidFuzzyTimeSector( const SYSTEMTIME& systemTime )
{
   return ( systemTime.wHour / 3 );
}


int GetHighFuzzyTimeSector( const SYSTEMTIME& systemTime )
{
   int day = systemTime.wDayOfWeek;

   if ( 1 == day )
   {
      return 0;
   }
   else if ( day >= 2 && day <= 4 )
   {
      return 1;
   }
   else if ( 5 == day )
   {
      return 2;
   }

   return 3;
}


int GetMidFuzzyTime( const SYSTEMTIME& systemTime, std::wstring& strMidFuzzyTime )
{
   int sector = GetMidFuzzyTimeSector( systemTime );

   strMidFuzzyTime = &g_szMidTimesText[sector * MID_STRING_SIZE];

   return sector;
}


int GetHighFuzzyTime( const SYSTEMTIME& systemTime, std::wstring& strHighFuzzyTime )
{
   int sector = GetHighFuzzyTimeSector( systemTime );

   strHighFuzzyTime = &g_szHighTimesText[sector * HIGH_STRING_SIZE];
      
   return sector;
}