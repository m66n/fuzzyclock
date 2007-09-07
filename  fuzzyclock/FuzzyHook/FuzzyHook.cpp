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
#include "FuzzyHook.h"
//#include "WriteLog.h"  // for debugging
#include <string>
#include <vector>
#include <cctype>
#include <algorithm>


#pragma data_seg(".fuzzy")
BOOL g_subclassed = FALSE;
UINT RWM_FUZZYHOOK = 0;
UINT RWM_RESET = 0;
HWND g_hWnd	= NULL;
HHOOK g_hHook = NULL;
wchar_t g_szHoursText[HT_STRING_SIZE * HT_COUNT] = L"";
wchar_t g_szTimesText[TT_STRING_SIZE * TT_COUNT] = L"";
#pragma data_seg()

#pragma comment(linker,"/SECTION:.fuzzy,RWS")


#define FUZZYHOOK_HOOK 1
#define FUZZYHOOK_UNHOOK 2

#define TIMER_FORCE_UPDATE 1000
#define FORCE_UPDATE_ELAPSE 60000


HINSTANCE g_hInst;
WNDPROC g_wndProcOld;

COLORREF g_clrForeground = COLOR_BTNTEXT;
HFONT g_hFont = NULL;
int g_widthClock = -1;

HDC g_hdcClock = NULL;
HBITMAP g_hbmpClock = NULL;
HDC g_hdcClockBackground = NULL;
HBITMAP g_hbmpClockBackground = NULL;


LRESULT CalculateWindowSize( HWND );
void Cleanup();
void ClearClockDC();
void CopyParentSurface( HWND, HDC, int, int, int, int, int, int );
bool CreateClockHDC( HWND );
bool CreateMemoryDC( HDC, HDC&, HBITMAP&, int, int );
void DrawFuzzyClock( HWND, HDC );
void GetTextSize( HDC, LPCWSTR, TEXTMETRIC&, SIZE& );
COLORREF GetThemeForeColor();
LRESULT _stdcall HookProc( int, WPARAM, LPARAM );
void Initialize();
LRESULT CALLBACK NewWndProc( HWND, UINT, WPARAM, LPARAM );
void RefreshTaskbar( HWND );
std::wstring TimeString();


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
         RWM_RESET = RegisterWindowMessage( _T("RWM_RESET__AD6337A5_5544_48cd_9C29_B9768DC467D3") );
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


FUZZYHOOK_API void Invalidate()
{
   PostMessage( g_hWnd, WM_SIZE, SIZE_RESTORED, 0 );
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
               g_wndProcOld = (WNDPROC)(intptr_t)SetWindowLong( g_hWnd, GWL_WNDPROC, (long)(intptr_t)NewWndProc );

               if ( NULL == g_wndProcOld )
               {
                  FreeLibrary( g_hInst );
               }
               else
               {
                  g_subclassed = TRUE;
                  Initialize();
                  InvalidateRect( g_hWnd, NULL, TRUE );
               }
            }
         }
      }
      else
      {
         UnhookWindowsHookEx( g_hHook );

         if ( SetWindowLong( g_hWnd, GWL_WNDPROC, (long)(intptr_t)g_wndProcOld ) )
         {
            KillTimer( g_hWnd, TIMER_FORCE_UPDATE );
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
   if ( message == RWM_FUZZYHOOK )
   {
      return 0;
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

   case WM_SETTINGCHANGE:
   case WM_SIZE:

      CreateClockHDC( hWnd );
      InvalidateRect( hWnd, NULL, TRUE );
      return 0;

   case WM_USER+100:

      return CalculateWindowSize( hWnd );

   case WM_SYSCOLORCHANGE:

      g_clrForeground = GetThemeForeColor();
      CreateClockHDC( hWnd );
      InvalidateRect( hWnd, NULL, TRUE );
      return 0;

   case WM_TIMECHANGE:
   case (WM_USER+101):
   case WM_SETFOCUS:
   case WM_KILLFOCUS:
   case WM_TIMER:

      InvalidateRect( hWnd, NULL, FALSE );
      return 0;

   case WM_NCHITTEST:

      return HTTRANSPARENT;
   }

   return CallWindowProc( g_wndProcOld, hWnd, message, wParam, lParam );
}


std::wstring TimeString()
{
   SYSTEMTIME systemTime;

   GetLocalTime( &systemTime );

   int minute = systemTime.wMinute;
   int sector = 0;
   int realHour = 0;

   if ( minute > 2 )
   {
      sector = ( minute - 3 ) / 5 + 1;
   }

   std::wstring timeString( &g_szTimesText[sector * TT_STRING_SIZE] );

   size_t startIndex = timeString.find( L"%" );

   if ( startIndex != std::wstring::npos )
   {
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

      transform( timeString.begin(), timeString.begin() + 1, timeString.begin(), toupper );
   }

   return timeString;
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

   std::wstring timeString = TimeString();

   SIZE sizeText;

   GetTextSize( hdc, timeString.c_str(), tm, sizeText );

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

   SetTimer( hWnd, TIMER_FORCE_UPDATE, FORCE_UPDATE_ELAPSE, NULL );

   RECT rc;
   GetClientRect( hWnd, &rc );

   BitBlt( g_hdcClock, 0, 0, rc.right, rc.bottom, g_hdcClockBackground, 0, 0, SRCCOPY );

   std::wstring timeString = TimeString();

   TEXTMETRIC textMetric;
   GetTextMetrics( g_hdcClock, &textMetric );

   SIZE sizeText;

   GetTextSize( g_hdcClock, timeString.c_str(), textMetric, sizeText );

   int x = rc.right / 2;;
   int y = ( rc.bottom - sizeText.cy ) / 2 - textMetric.tmInternalLeading / 2;

   TextOutW( g_hdcClock, x, y, timeString.c_str(), (int)timeString.size() );

   BitBlt( hDC, 0, 0, rc.right, rc.bottom, g_hdcClock, 0, 0, SRCCOPY );

   if ( sizeText.cx + textMetric.tmAveCharWidth * 2 != g_widthClock )
   {
      g_widthClock = sizeText.cx + textMetric.tmAveCharWidth * 2;
      PostMessage( GetParent( GetParent( hWnd ) ), WM_SIZE, SIZE_RESTORED, 0 );
      InvalidateRect( GetParent( GetParent( hWnd ) ), NULL, TRUE );
   }
}


void Initialize()
{
   g_clrForeground = GetThemeForeColor();

   HFONT hFont = (HFONT)GetStockObject( DEFAULT_GUI_FONT );

   LOGFONT lf;
   GetObject( hFont, sizeof(lf), (LPVOID)(&lf) );

   HDC hDC = GetDC( NULL );

   POINT pt;
   pt.x = 0;
   pt.y = GetDeviceCaps( hDC, LOGPIXELSY ) * 9 / 72;
   DPtoLP( hDC, &pt, 1 );

   lf.lfHeight = -pt.y;

   ReleaseDC( NULL, hDC );

   lf.lfWidth = 0;
   lf.lfEscapement = 0;
   lf.lfOrientation = 0;
   lf.lfWeight = 0;
   lf.lfItalic = FALSE;
   lf.lfUnderline = FALSE;
   lf.lfStrikeOut = FALSE;
   lf.lfOutPrecision = OUT_DEFAULT_PRECIS;
   lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
   lf.lfQuality = DEFAULT_QUALITY;
   lf.lfPitchAndFamily = DEFAULT_PITCH | FF_DONTCARE;

   g_hFont = CreateFontIndirect( &lf );
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


COLORREF GetThemeForeColor()
{
   COLORREF clr = COLOR_BTNTEXT;

   HMODULE hMod = LoadLibrary( _T("uxtheme.dll") );

   if ( NULL != hMod )
   {
      typedef HANDLE WINAPI OPENTHEMEDATA( HWND, LPCWSTR );
      typedef HRESULT WINAPI GETTHEMECOLOR( HANDLE, int, int, int, COLORREF* );
      typedef HRESULT WINAPI CLOSETHEMEDATA( HANDLE );

      OPENTHEMEDATA* pOPENTHEMEDATA = reinterpret_cast<OPENTHEMEDATA*>(
         GetProcAddress( hMod, ( "OpenThemeData" ) ) );
      GETTHEMECOLOR* pGETTHEMECOLOR = reinterpret_cast<GETTHEMECOLOR*>(
         GetProcAddress( hMod, ( "GetThemeColor" ) ) );
      CLOSETHEMEDATA* pCLOSETHEMEDATA = reinterpret_cast<CLOSETHEMEDATA*>(
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

            if ( NULL != pCLOSETHEMEDATA )
            {
               pCLOSETHEMEDATA( hTheme );
            }
         }
      }

      FreeLibrary( hMod );
   }

   return clr;
}

