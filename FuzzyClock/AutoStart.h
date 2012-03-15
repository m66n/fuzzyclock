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

#pragma once

#include <string>
#include <boost/utility.hpp>


class AutoStart : boost::noncopyable
{
public:
   AutoStart(const wchar_t* shortcutName, const wchar_t* filePath,
         const wchar_t* workingDir, const wchar_t* description);
   ~AutoStart();

   void Enable();
   void Disable();
   bool IsEnabled() const;

   static std::wstring GetAppPath();
   static std::wstring GetWorkingDir();
   static std::wstring GetShortcutPath(const wchar_t* shortcutName);

private:
   std::wstring filePath_;
   std::wstring workingDir_;
   std::wstring description_;
   std::wstring shortcutPath_;

   static void CheckHR(HRESULT hr, bool throwOnFailure);
};

