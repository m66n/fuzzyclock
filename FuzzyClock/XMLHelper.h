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
#include <vector>

#import <MSXML6.dll>


class XMLHelper
{
public:

   XMLHelper();
   ~XMLHelper();

   bool LoadFile( LPCWSTR szFileName );

   const std::wstring& GetApplicationName() const { return applicationName_; }
   const std::wstring& GetExitText() const { return exitText_; }
   const std::wstring& GetStartText() const { return startText_; }
   const std::wstring& GetFuzzinessText() const { return fuzzinessText_; }
   const std::wstring& GetHourText( int index ) const { return hoursText_[index]; }
   const std::wstring& GetTimeText( int index ) const { return timesText_[index]; }
   const std::wstring& GetMidTimeText( int index ) const { return midTimesText_[index]; }
   const std::wstring& GetHighTimeText( int index ) const { return highTimesText_[index]; }
   const std::wstring& GetFuzzinessLevelText( int index ) const { return fuzzinessLevelsText_[index]; }


   size_t GetHoursTextCount() const { return hoursText_.size(); }
   size_t GetTimesTextCount() const { return timesText_.size(); }
   size_t GetMidTimesTextCount() const { return midTimesText_.size(); }
   size_t GetHighTimesTextCount() const { return highTimesText_.size(); }
   size_t GetFuzzinessLevelsTextCount() const { return fuzzinessLevelsText_.size(); }

private:

   bool ParseApplicationName( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseExitText( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseStartText( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseFuzzinessText( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseHoursText( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseHourText( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseTimesText( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseTimeText( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseMidTimesText( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseMidTimeText( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseHighTimesText( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseHighTimeText( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseFuzzinessLevelsText( MSXML2::IXMLDOMNodePtr pNode );
   bool ParseFuzzinessLevelText( MSXML2::IXMLDOMNodePtr pNode );

   std::wstring applicationName_;
   std::wstring exitText_;
   std::wstring startText_;
   std::wstring fuzzinessText_;
   std::vector<std::wstring> hoursText_;
   std::vector<std::wstring> timesText_;
   std::vector<std::wstring> midTimesText_;
   std::vector<std::wstring> highTimesText_;
   std::vector<std::wstring> fuzzinessLevelsText_;
};
