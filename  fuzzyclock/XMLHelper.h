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

#include <string>
#include <vector>

#import <msxml.dll>


class XMLHelper
{
public:

   XMLHelper();
   ~XMLHelper();

   bool LoadFile( LPCWSTR szFileName );

   const std::wstring& GetApplicationName() const { return applicationName_; }
   const std::wstring& GetExitText() const { return exitText_; }
   const std::wstring& GetHourText( int index ) { return hoursText_[index]; }
   const std::wstring& GetTimeText( int index ) { return timesText_[index]; }

   size_t GetHoursTextCount() { return hoursText_.size(); }
   size_t GetTimesTextCount() { return timesText_.size(); }

private:

   bool ParseApplicationName( MSXML::IXMLDOMNodePtr pNode );
   bool ParseExitText( MSXML::IXMLDOMNodePtr pNode );
   bool ParseHoursText( MSXML::IXMLDOMNodePtr pNode );
   bool ParseHourText( MSXML::IXMLDOMNodePtr pNode );
   bool ParseTimesText( MSXML::IXMLDOMNodePtr pNode );
   bool ParseTimeText( MSXML::IXMLDOMNodePtr pNode );

   std::wstring applicationName_;
   std::wstring exitText_;
   std::vector<std::wstring> hoursText_;
   std::vector<std::wstring> timesText_;
};
