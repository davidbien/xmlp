
// xmlgtest.cpp
// Google test stuff for XML Parser.
// dbien
// 28JAN2021

#ifdef WIN32
#define _CRTDBG_MAP_ALLOC
#include <stdlib.h>
#include <crtdbg.h>
#ifndef NDEBUG
    #define DBG_NEW new ( _NORMAL_BLOCK , __FILE__ , __LINE__ )
#else
    #define DBG_NEW new
#endif
#else //!WIN32
#define DBG_NEW new
#endif //!WIN32

#include <memory>
#include <exception>
typedef std::allocator< char >	_TyDefaultAllocator;

#include "_compat.h"

// Include lexical analyzer:
#include <string>
#include <iostream>
#include <fstream>
#include "syslogmgr.inl"
#include "_compat.inl"
#include "xml_inc.h"
#include "xml_tag.h"
#include "gtest/gtest.h"

namespace ns_XMLGTest
{
  __XMLP_USING_NAMESPACE

  template < class t_TyXmlTraits >
  class XmlpTest : public testing::Test
  {
    typedef XmlpTest _TyThis;
    typedef testing::Test _TyBase;
  public:
    typedef t_TyXmlTraits _TyXmlTraits;
    typedef typename _TyXmlTraits::_TyStdStr _TyStdStr;
  protected:
    // SetUp() is run immediately before a test starts.
    void SetUp() override 
    {
    }

    // TearDown() is invoked immediately after a test finishes.
    void TearDown() override 
    {
    }
  };

  template < class t_TyXmlTraits >
  class XmlpTestEnvironment : public testing::Environment
  {
    typedef XmlpTestEnvironment _TyThis;
    typedef testing::Environment _TyBase;
  protected:
    typedef t_TyXmlTraits _TyXmlTraits;
    typedef typename _TyXmlTraits::_TyStdStr _TyStdStr;
    explicit XmlpTestEnvironment( const char * _pszFileName )
      : m_strFileName( _pszFileName )
    {
    }
    void SetUp() override 
    {
      try
      {
        _TrySetup();
      }
      catch( exception const & rexc )
      {
        // 
      }
    }
    void _TrySetUp() override 
    {
      // 1) Open the file.
      // 2) Figure out the encoding of the file.
      //    a) Potentially convert that encoding to an encoding we can read.
      // 3) Instantiate the correct transport for that encoding.
      VerifyThrowSz( FFileExists( m_strFileName ), "File[%s] doesn't exist.", m_strFileName.c_str() );
      FileObj fo( OpenReadOnlyFile( m_strFileName.c_str() ) );
      VerifyThrowSz( fo.FIsOpen(), "Couldn't open file [%s]", m_strFileName.c_str() );
      uint8_t rgbyBOM[vknBytesBOM];
      size_t nbyRead;
      int iResult = FileRead( fo.HFileGet(), rgbyBOM, vknBytesBOM, &nbyRead );
      Assert( !iResult );
      Assert( nbyRead == vknBytesBOM );
      EFileCharacterEncoding efceEncoding = efceFileCharacterEncodingCount;
      if ( !iResult && ( nbyRead == vknBytesBOM ) )
        efceEncoding = GetCharacterEncodingFromBOM( rgbyBOM, vknBytesBOM );
      if ( efceFileCharacterEncodingCount == efceEncoding )
      {
        // Since we know we are opening XML we can use an easy heuristic to determine the encoding:
        efceEncoding = DetectEncodingXmlFile( rgbyBOM, vknBytesBOM );
        Assert( efceFileCharacterEncodingCount != efceEncoding ); // Unless the source isn't XML the above should succeed.
      }


       
    }

    // TearDown() is invoked immediately after a test finishes.
    void TearDown() override 
    {
    }

    string m_strFileName;
    bool m_fExpectFailure{false}; // If this is true then we expect the test to fail.
    _TyStdStr m_strBackingBuffer; // If the file's character's type doesn't match the character type of the parser then we will convert.
  };
}

int main( int argc, char **argv ) 
{
  if ( 2 == argc )
    fprintf(stderr, "Got argument [%s].\n", argv[1]);
//  testing::InitGoogleTest(&argc, argv);
//  testing::AddGlobalTestEnvironment(new MyTestEnvironment(command_line_arg));
  return 0;
}