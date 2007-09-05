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
#include "XMLHelper.h"

#import <msxml.dll>


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
      MSXML::IXMLDOMDocumentPtr pDOMDoc( __uuidof( MSXML::DOMDocument ) );

      pDOMDoc->put_async( VARIANT_FALSE );

      _bstr_t bstrFileName = szFileName;

      VARIANT_BOOL varResult = pDOMDoc->load( _variant_t( bstrFileName ) );

      if ( VARIANT_TRUE == varResult )
      {
         MSXML::IXMLDOMNodePtr pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//application_name" );

         bool success = ParseApplicationName( pNode );

         if ( !success )
         {
            return false;
         }

         pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//exit_text" );

         success = ParseExitText( pNode );

         if ( !success )
         {
            return false;
         }
         pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//hours_text" );

         success = ParseHoursText( pNode );

         if ( !success )
         {
           return false;
         }

         pNode = pDOMDoc->selectSingleNode( "//fuzzy_clock//times_text" );

         success = ParseTimesText( pNode );

         return success;
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


bool XMLHelper::ParseApplicationName( MSXML::IXMLDOMNodePtr pNode )
{
   if ( pNode )
   {
      MSXML::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

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


bool XMLHelper::ParseExitText( MSXML::IXMLDOMNodePtr pNode )
{
   if ( pNode )
   {
      MSXML::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

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


bool XMLHelper::ParseHourText( MSXML::IXMLDOMNodePtr pNode )
{
   int index = -1;

   if ( pNode )
   {
      MSXML::IXMLDOMNamedNodeMapPtr pAttrMap = pNode->Getattributes();

      if ( NULL != pAttrMap )
      {
         MSXML::IXMLDOMAttributePtr pAttribute;
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

      MSXML::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         pNode = pChildren->nextNode();

         if ( pNode )
         {
            _variant_t varData = pNode->GetnodeValue();

            if ( varData.vt != VT_NULL )
            {
               if ( index >= 0 )
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


bool XMLHelper::ParseHoursText( MSXML::IXMLDOMNodePtr pNode )
{
   hoursText_.clear();

   hoursText_.resize( 12 );

   if ( pNode )
   {
      MSXML::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

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


bool XMLHelper::ParseTimeText( MSXML::IXMLDOMNodePtr pNode )
{
   int index = -1;

   if ( pNode )
   {
      MSXML::IXMLDOMNamedNodeMapPtr pAttrMap = pNode->Getattributes();

      if ( NULL != pAttrMap )
      {
         MSXML::IXMLDOMAttributePtr pAttribute;
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

      MSXML::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

      if ( pChildren )
      {
         pNode = pChildren->nextNode();

         if ( pNode )
         {
            _variant_t varData;
            pNode->get_nodeValue( varData.GetAddress() );

            if ( varData.vt != VT_NULL )
            {
               if ( index >= 0 )
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


bool XMLHelper::ParseTimesText( MSXML::IXMLDOMNodePtr pNode )
{
   timesText_.clear();

   timesText_.resize( 13 );

   if ( pNode )
   {
      MSXML::IXMLDOMNodeListPtr pChildren = pNode->GetchildNodes();

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