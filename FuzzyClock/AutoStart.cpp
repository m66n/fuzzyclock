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
#include "AutoStart.h"
#include <boost/filesystem.hpp>
#include <comdef.h>
#include <Shlobj.h>


namespace fs = boost::filesystem;


template <typename T>
class SmartRefPointer
{
private:

   T* interface_;

   void Reset()
   {
      if (interface_)
      {
         interface_->Release();
      }

      interface_ = NULL;
   }

   void AddReference()
   {
      if (interface_)
      {
          interface_->AddRef();
      }
   }

public:

   SmartRefPointer()
   {
      interface_ = NULL;
   }

   SmartRefPointer(T* rhs)
   {
      interface_ = rhs;
      AddReference();
   }

   SmartRefPointer(const SmartRefPointer& rhs)
   {
      interface_ = rhs.interface_;
      AddReference();
   }

   ~SmartRefPointer()
   {
      Reset();
   }

   T* Get() const
   {
      return interface_;
   }

   T* Release()
   {
      T* retval = interface_;
      interface_ = NULL;
      return retval;
   }

   void Clear()
   {
      Reset();
   }

   T** GetInterfacePointer()
   {
      Reset();
      return &interface_;
   }

   operator LPVOID*()
   {
      return reinterpret_cast<LPVOID*>(GetInterfacePointer());
   }

   SmartRefPointer& operator =(const SmartRefPointer& rhs)
   {
      Reset();
      interface_ = rhs.interface_;
      AddReference();
      return *this;
   }

   T* operator ->() const
   {
      return interface_;
   }

   operator const T*() const
   {
      return interface_;
   }
};


typedef SmartRefPointer<IShellLinkW> ShellLinkPtr;
typedef SmartRefPointer<IPersistFile> PersistFilePtr;


AutoStart::AutoStart(const wchar_t* shortcutName, const wchar_t* filePath,
      const wchar_t* workingDir, const wchar_t* description) :
         filePath_(filePath), workingDir_(workingDir),
         description_(description), shortcutPath_(GetShortcutPath(shortcutName))
{
}


AutoStart::~AutoStart()
{
}


std::wstring AutoStart::GetAppPath()
{
   WCHAR szPath[MAX_PATH];
   GetModuleFileNameW(NULL, szPath, MAX_PATH);
   return szPath;
}


std::wstring AutoStart::GetShortcutPath(const wchar_t* shortcutName)
{
   wchar_t szShortcutPath[MAX_PATH];

   CheckHR(SHGetFolderPathW(NULL, CSIDL_STARTUP, NULL,
      SHGFP_TYPE_CURRENT, szShortcutPath), true);

   fs::path path(szShortcutPath);

   path /= shortcutName;
   path.replace_extension(L".lnk");

   return path.wstring().c_str();
}


void AutoStart::CheckHR(HRESULT hr, bool throwOnFailure)
{
   if (SUCCEEDED(hr))
   {
      return;
   }

   if (throwOnFailure)
   {
      throw _com_error(hr);
   }
}


void AutoStart::Enable()
{
   ShellLinkPtr shellPtr;

   CheckHR(CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER,
         IID_IShellLink, shellPtr), true);

   PersistFilePtr filePtr;

   CheckHR(shellPtr->QueryInterface(IID_IPersistFile, filePtr), true);
   CheckHR(shellPtr->SetPath(filePath_.c_str()), true);
   CheckHR(shellPtr->SetWorkingDirectory(workingDir_.c_str()), true);
   CheckHR(shellPtr->SetDescription(description_.c_str()), true);

   CheckHR(filePtr->Save(shortcutPath_.c_str(), TRUE), true);
}


void AutoStart::Disable()
{
   fs::remove(shortcutPath_.c_str());
}


bool AutoStart::IsEnabled() const
{
   return fs::is_regular_file(shortcutPath_.c_str());
}


std::wstring AutoStart::GetWorkingDir()
{
   return fs::current_path().wstring().c_str();
}
