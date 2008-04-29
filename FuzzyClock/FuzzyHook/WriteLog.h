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


#pragma once

#include <stdio.h>


static void WriteLog( LPCTSTR lpszDebugOut, ... )
{
   TCHAR buf[1024]; 
   va_list arg_list;

   va_start( arg_list, lpszDebugOut );
   _vstprintf( buf, lpszDebugOut, arg_list );
   va_end( arg_list );

#ifdef _UNICODE
   FILE* file = _wfopen( L"c:\\log.txt", L"a+" );
   fwprintf( file, L"%s", buf );
   fclose( file );
#else
   FILE* file = fopen( "c:\\log.txt", "a+" );
   fprintf( file, "%s", buf );
   fclose( file );
#endif
}