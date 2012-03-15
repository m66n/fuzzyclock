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

#include <boost/utility.hpp>
#include <string>


typedef enum { LOWEST, LOWER, HIGHER, HIGHEST } FUZZINESS;

typedef enum { FUZZY, PRECISE } PRECISION;


class Config : boost::noncopyable
{
public:
   Config(const wchar_t* appName, const wchar_t* fileName);
   ~Config();

   static std::wstring GetConfigPath(const wchar_t* appName,
         const wchar_t* fileName);

   int GetFuzziness() const { return fuzziness_; }
   void SetFuzziness(int fuzziness);

   int GetPrecise() const { return precise_; }
   void SetPrecise(int precise);

   void Load();
   void Save();

private:
   std::wstring path_;
   bool dirty_;
   
   int fuzziness_;
   int precise_;
};
