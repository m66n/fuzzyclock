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
#include "XMLHelper.h"
#include ".\FuzzyHook\FuzzyHook.h"
#import <msxml6.dll>


class CoInitializeHelper
{
public:
   CoInitializeHelper() { CoInitialize( NULL ); }
   ~CoInitializeHelper() { CoUninitialize(); }
};


XMLHelper::XMLHelper()
{
}


XMLHelper::~XMLHelper()
{
}


bool XMLHelper::LoadFile( LPCWSTR szFileName )
{
   CoInitializeHelper helper;

   try
   {
      MSXML2::IXMLDOMDocumentPtr pDOMDoc( __uuidof( MSXML2::DOMDocument ) );

      pDOMDoc->put_async( VARIANT_FALSE );

      _bstr_t bstrFileName = szFileName;

      VARIANT_BOOL varResult = pDOMDoc->load( _variant_t( bstrFileName ) );

      if ( VARIANT_TRUE == varResult )
      {
         MSXML2::IXMLDOMNodePtr pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//application_name" );

         if ( !ParseApplicationName( pNode ) )
         {
            return false;
         }

         pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//exit_text" );

         if ( !ParseExitText( pNode ) )
         {
            return false;
         }

         pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//start_text" );

         if ( !ParseStartText( pNode ) )
         {
            return false;
         }

         pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//fuzziness_text" );

         if ( !ParseFuzzinessText( pNode ) )
         {
            return false;
         }

         pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//fuzziness_levels" );

         if ( !ParseFuzzinessLevelsText( pNode ) )
         {
            return false;
         }

         pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//hours_text" );

         if ( !ParseHoursText( pNode ) )
         {
           return false;
         }

         pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//times_text" );

         if ( !ParseTimesText( pNode ) )
         {
            return false;
         }

         pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//mid_times_text" );

         if ( !ParseMidTimesText( pNode ) )
         {
            return false;
         }

         pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//high_times_text" );

         return ParseHighTimesText( pNode );
      }
      else
      {
         // put an error somewhere?  the event log?
      }
   }
   catch ( _com_error& e )
   {
      // put an error somewhere?
      UNREFERENCED_PARAMETER( e );
   }

   return false;
}


bool XMLHelper::ParseApplicationName( MSXML2::IXMLDOMNodePtr pNode )
{
   if ( pNode )
   {
      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         pNode = pChildren->nextNode();

         if ( pNode )
         {
            _variant_t varData = pNode->GetnodeValue();

            if ( varData.vt != VT_NULL )
            {
               applicationName_ = (LPCWSTR)(varData.bstrVal);
               return true;
            }            
         }
      }
   }

   return false;
}


bool XMLHelper::ParseExitText( MSXML2::IXMLDOMNodePtr pNode )
{
   if ( pNode )
   {
      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         pNode = pChildren->nextNode();

         if ( pNode )
         {
            _variant_t varData = pNode->GetnodeValue();

            if ( varData.vt != VT_NULL )
            {
               exitText_ = (LPCWSTR)(varData.bstrVal);
               return true;
            }            
         }
      }
   }

   return false;
}


bool XMLHelper::ParseStartText( MSXML2::IXMLDOMNodePtr pNode )
{
   if ( pNode )
   {
      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         pNode = pChildren->nextNode();

         if ( pNode )
         {
            _variant_t varData = pNode->GetnodeValue();

            if ( varData.vt != VT_NULL )
            {
               startText_ = (LPCWSTR)(varData.bstrVal);
               return true;
            }            
         }
      }
   }

   return false;
}


bool XMLHelper::ParseFuzzinessText( MSXML2::IXMLDOMNodePtr pNode )
{
   if ( pNode )
   {
      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         pNode = pChildren->nextNode();

         if ( pNode )
         {
            _variant_t varData = pNode->GetnodeValue();

            if ( varData.vt != VT_NULL )
            {
               fuzzinessText_ = (LPCWSTR)(varData.bstrVal);
               return true;
            }            
         }
      }
   }

   return false;
}


bool XMLHelper::ParseHourText( MSXML2::IXMLDOMNodePtr pNode )
{
   int index = -1;

   if ( pNode )
   {
      MSXML2::IXMLDOMNamedNodeMapPtr pAttrMap = pNode->Getattributes();

      if ( NULL != pAttrMap )
      {
         MSXML2::IXMLDOMAttributePtr pAttribute;
         while ( pAttribute = pAttrMap->nextNode() )
         {
            if ( pAttribute->Getname() == _bstr_t( "index" ) )
            {
               _variant_t varIndex = pAttribute->Getvalue();

               if ( varIndex.vt != VT_NULL )
               {
                  index = _wtoi( (LPCWSTR)(varIndex.bstrVal) );
               }
            }
         }
      }

      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         pNode = pChildren->nextNode();

         if ( pNode )
         {
            _variant_t varData = pNode->GetnodeValue();

            if ( varData.vt != VT_NULL )
            {
               if ( index >= 0 && index < HT_COUNT )
               {
                  hoursText_[index] = (LPCWSTR)(varData.bstrVal);
                  return true;
               }
            }            
         }
      }
   }

   return false;
}


bool XMLHelper::ParseHoursText( MSXML2::IXMLDOMNodePtr pNode )
{
   hoursText_.clear();

   hoursText_.resize( HT_COUNT );

   if ( pNode )
   {
      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         while ( pNode = pChildren->nextNode() )
         {
            if ( !ParseHourText( pNode ) )
            {
               return false;
            }
         }

         return true;
      }
   }

   return false;
}


bool XMLHelper::ParseTimeText( MSXML2::IXMLDOMNodePtr pNode )
{
   int index = -1;

   if ( pNode )
   {
      MSXML2::IXMLDOMNamedNodeMapPtr pAttrMap = pNode->Getattributes();

      if ( NULL != pAttrMap )
      {
         MSXML2::IXMLDOMAttributePtr pAttribute;
         while ( pAttribute = pAttrMap->nextNode() )
         {
            if ( pAttribute->Getname() == _bstr_t( "index" ) )
            {
               _variant_t varIndex = pAttribute->Getvalue();

               if ( varIndex.vt != VT_NULL )
               {
                  index = _wtoi( (LPCWSTR)(varIndex.bstrVal) );
               }
            }
         }
      }

      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         pNode = pChildren->nextNode();

         if ( pNode )
         {
            _variant_t varData;
            pNode->get_nodeValue( varData.GetAddress() );

            if ( varData.vt != VT_NULL )
            {
               if ( index >= 0 && index < TT_COUNT )
               {
                  timesText_[index] = (LPCWSTR)(varData.bstrVal);
                  return true;
               }
            }            
         }
      }
   }

   return false;
}


bool XMLHelper::ParseTimesText( MSXML2::IXMLDOMNodePtr pNode )
{
   timesText_.clear();

   timesText_.resize( TT_COUNT );

   if ( pNode )
   {
      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         while ( pNode = pChildren->nextNode() )
         {
            if ( !ParseTimeText( pNode ) )
            {
               return false;
            }
         }
      }
   }

   return true;
}


bool XMLHelper::ParseMidTimeText( MSXML2::IXMLDOMNodePtr pNode )
{
   int index = -1;

   if ( pNode )
   {
      MSXML2::IXMLDOMNamedNodeMapPtr pAttrMap = pNode->Getattributes();

      if ( NULL != pAttrMap )
      {
         MSXML2::IXMLDOMAttributePtr pAttribute;
         while ( pAttribute = pAttrMap->nextNode() )
         {
            if ( pAttribute->Getname() == _bstr_t( "index" ) )
            {
               _variant_t varIndex = pAttribute->Getvalue();

               if ( varIndex.vt != VT_NULL )
               {
                  index = _wtoi( (LPCWSTR)(varIndex.bstrVal) );
               }
            }
         }
      }

      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         pNode = pChildren->nextNode();

         if ( pNode )
         {
            _variant_t varData;
            pNode->get_nodeValue( varData.GetAddress() );

            if ( varData.vt != VT_NULL )
            {
               if ( index >= 0 && index < MID_COUNT )
               {
                  midTimesText_[index] = (LPCWSTR)(varData.bstrVal);
                  return true;
               }
            }            
         }
      }
   }

   return false;
}


bool XMLHelper::ParseMidTimesText( MSXML2::IXMLDOMNodePtr pNode )
{
   midTimesText_.clear();

   midTimesText_.resize( MID_COUNT );

   if ( pNode )
   {
      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         while ( pNode = pChildren->nextNode() )
         {
            if ( !ParseMidTimeText( pNode ) )
            {
               return false;
            }
         }
      }
   }

   return true;
}


bool XMLHelper::ParseHighTimeText( MSXML2::IXMLDOMNodePtr pNode )
{
   int index = -1;

   if ( pNode )
   {
      MSXML2::IXMLDOMNamedNodeMapPtr pAttrMap = pNode->Getattributes();

      if ( NULL != pAttrMap )
      {
         MSXML2::IXMLDOMAttributePtr pAttribute;
         while ( pAttribute = pAttrMap->nextNode() )
         {
            if ( pAttribute->Getname() == _bstr_t( "index" ) )
            {
               _variant_t varIndex = pAttribute->Getvalue();

               if ( varIndex.vt != VT_NULL )
               {
                  index = _wtoi( (LPCWSTR)(varIndex.bstrVal) );
               }
            }
         }
      }

      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         pNode = pChildren->nextNode();

         if ( pNode )
         {
            _variant_t varData;
            pNode->get_nodeValue( varData.GetAddress() );

            if ( varData.vt != VT_NULL )
            {
               if ( index >= 0 && index < HIGH_COUNT )
               {
                  highTimesText_[index] = (LPCWSTR)(varData.bstrVal);
                  return true;
               }
            }            
         }
      }
   }

   return false;
}


bool XMLHelper::ParseHighTimesText( MSXML2::IXMLDOMNodePtr pNode )
{
   highTimesText_.clear();

   highTimesText_.resize( HIGH_COUNT );

   if ( pNode )
   {
      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         while ( pNode = pChildren->nextNode() )
         {
            if ( !ParseHighTimeText( pNode ) )
            {
               return false;
            }
         }
      }
   }

   return true;
}


bool XMLHelper::ParseFuzzinessLevelText( MSXML2::IXMLDOMNodePtr pNode )
{
   int index = -1;

   if ( pNode )
   {
      MSXML2::IXMLDOMNamedNodeMapPtr pAttrMap = pNode->Getattributes();

      if ( NULL != pAttrMap )
      {
         MSXML2::IXMLDOMAttributePtr pAttribute;
         while ( pAttribute = pAttrMap->nextNode() )
         {
            if ( pAttribute->Getname() == _bstr_t( "index" ) )
            {
               _variant_t varIndex = pAttribute->Getvalue();

               if ( varIndex.vt != VT_NULL )
               {
                  index = _wtoi( (LPCWSTR)(varIndex.bstrVal) );
               }
            }
         }
      }

      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         pNode = pChildren->nextNode();

         if ( pNode )
         {
            _variant_t varData;
            pNode->get_nodeValue( varData.GetAddress() );

            if ( varData.vt != VT_NULL )
            {
               if ( index >= 0 && index < (int)fuzzinessLevelsText_.size() )
               {
                  fuzzinessLevelsText_[index] = (LPCWSTR)(varData.bstrVal);
                  return true;
               }
            }            
         }
      }
   }

   return false;
}


bool XMLHelper::ParseFuzzinessLevelsText( MSXML2::IXMLDOMNodePtr pNode )
{
   fuzzinessLevelsText_.clear();

   fuzzinessLevelsText_.resize( 4 );

   if ( pNode )
   {
      MSXML2::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         while ( pNode = pChildren->nextNode() )
         {
            if ( !ParseFuzzinessLevelText( pNode ) )
            {
               return false;
            }
         }
      }
   }

   return true;
}