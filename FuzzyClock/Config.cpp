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
#include "Config.h"
#include <Shlobj.h>

#include <exception>
#include <iostream>

#include <boost/filesystem.hpp>
#include <boost/filesystem/fstream.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

namespace fs = boost::filesystem;
namespace pt = boost::property_tree;


const char* KEY_FUZZINESS = "config.fuzziness";
const char* KEY_PRECISE = "config.precise";


class SafePWSTR
{
public:
   SafePWSTR() : _p(NULL) {}
   ~SafePWSTR() { if (_p) CoTaskMemFree(_p); }

   operator PWSTR*() { return &_p; }
   operator PWSTR() { return _p; }

private:
   PWSTR _p;
};


Config::Config(const wchar_t* appName, const wchar_t* fileName) : 
      path_(GetConfigPath(appName, fileName)), dirty_(false), fuzziness_(LOWEST), precise_(FUZZY)
{
   Load();
}


Config::~Config()
{
   Save();
}


std::wstring Config::GetConfigPath(const wchar_t* appName,
         const wchar_t* fileName)
{
   SafePWSTR rootPath;
   HRESULT hr = SHGetKnownFolderPath(FOLDERID_RoamingAppData, 0, NULL, rootPath);

   if (FAILED(hr)) 
   {
      return L"";
   }

   fs::path configPath(rootPath);
   configPath /= appName;

   if (!fs::exists(configPath))
   {
      try { fs::create_directory(configPath); }
      catch (fs::filesystem_error& e)
      {
         std::cerr << e.what() << std::endl;
         return L"";
      }
   }

   configPath /= fileName;
   
   return configPath.c_str();
}


void Config::SetFuzziness(int fuzziness)
{
   if (fuzziness_ != fuzziness)
   {
      fuzziness_ = fuzziness;
      dirty_ = true;
   }
}


void Config::SetPrecise(int precise)
{
   if (precise_ != precise)
   {
      precise_ = precise;
      dirty_ = true;
   }
}


void Config::Load()
{
   if (fs::exists(path_))
   {
      try
      {
         fs::ifstream file(path_);
         pt::ptree pt;
         pt::read_xml(file, pt);
         fuzziness_ = pt.get<int>(KEY_FUZZINESS);
         precise_ = pt.get<int>(KEY_PRECISE);
      }
      catch (std::exception& e)
      {
         std::cerr << e.what() << std::endl;
      }
   }
}


void Config::Save()
{
   if (dirty_)
   {
      try
      {
         fs::ofstream file(path_);
         pt::ptree pt;
         pt.put(KEY_FUZZINESS, fuzziness_);
         pt.put(KEY_PRECISE, precise_);
         pt::write_xml(file, pt);
         dirty_ = false;
      }
      catch (std::exception& e)
      {
         std::cerr << e.what() << std::endl;
      }
   }
}
