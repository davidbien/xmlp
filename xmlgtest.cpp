
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
#include "_heapchk.h"

// Include lexical analyzer:
#include <string>
#include <iostream>
#include <fstream>
#include "syslogmgr.inl"
#include "_compat.inl"
#include "xml_inc.h"
#include "xml_tag.h"
#include "xml_tag_var.h"
#include "gtest/gtest.h"

std::string g_strProgramName;

namespace ns_XMLGTest
{
__XMLP_USING_NAMESPACE

class XmlpTestEnvironment : public testing::Environment
{
  typedef XmlpTestEnvironment _TyThis;
  typedef testing::Environment _TyBase;
public:
  explicit XmlpTestEnvironment( const char * _pszFileName )
    : m_strFileNameOrig( _pszFileName )
  {
  }
protected:
  void SetUp() override 
  {
    // 0) Check if the caller used a slash that doesn't match the OS slash and substitute.
    // 1) Open the file.
    // 2) Figure out the encoding of the file. Also check if it is a failure scenario.
    // 3) Create all potential encodings of the file - i.e. UTF32, UTF16, and UTF8 big and little endian.
    //  a) Write each encoding with and without a BOM.
    // 4) Write all encodings in a unittest directory in the output.
    // 5) List all file names in this environment object in m_rgFileNamesTestDir.
    const char kcSlash = ( string::npos == m_strFileNameOrig.find( TChGetFileSeparator<char>() ) ) ? TChGetOtherFileSeparator<char>() : TChGetFileSeparator<char>();
    VerifyThrowSz( FFileExists( m_strFileNameOrig.c_str() ), "Original test file[%s] doesn't exist.", m_strFileNameOrig.c_str() );
    size_t nPosXmlExt = m_strFileNameOrig.rfind( ".xml" );
    VerifyThrowSz( string::npos != nPosXmlExt, "File doesn't have an '.xml' extention [%s].", m_strFileNameOrig.c_str() );
    size_t nPosPrevSlash = m_strFileNameOrig.rfind( kcSlash );
    VerifyThrowSz( string::npos != nPosPrevSlash, "Need full path to input file[%s] - couldn't find preceding slash.", m_strFileNameOrig.c_str() );
    VerifyThrowSz( nPosXmlExt > nPosPrevSlash+1, "uh that's not an acceptable file name [%s].", m_strFileNameOrig.c_str() );
    string strBaseFile("unittests");
    strBaseFile += TChGetFileSeparator<char>();
    strBaseFile += m_strFileNameOrig.substr( nPosPrevSlash+1, nPosXmlExt - ( nPosPrevSlash+1 ) );
    FileObj fo( OpenReadOnlyFile( m_strFileNameOrig.c_str() ) );
    VerifyThrowSz( fo.FIsOpen(), "Couldn't open file [%s]", m_strFileNameOrig.c_str() );
    uint8_t rgbyBOM[vknBytesBOM];
    size_t nbyLenghtBOM;
    int iResult = FileRead( fo.HFileGet(), rgbyBOM, vknBytesBOM, &nbyLenghtBOM );
    Assert( !iResult );
    Assert( nbyLenghtBOM == vknBytesBOM ); // The smallest valid xml file is 4 bytes... "<t/>".
    VerifyThrowSz( nbyLenghtBOM == vknBytesBOM, "Unable to read [%lu] bytes from the file[%s].", vknBytesBOM, m_strFileNameOrig.c_str() );
    m_fExpectFailure = m_strFileNameOrig.find("FAIL"); // simple method for detecting expected failures.
    EFileCharacterEncoding efceEncoding = efceFileCharacterEncodingCount;
    if ( !iResult && ( nbyLenghtBOM == vknBytesBOM ) )
      efceEncoding = GetCharacterEncodingFromBOM( rgbyBOM, nbyLenghtBOM );
    bool fEncodingFromBOM = ( efceFileCharacterEncodingCount != efceEncoding );
    if ( !fEncodingFromBOM )
    {
      // Since we know we are opening XML we can use an easy heuristic to determine the encoding:
      efceEncoding = DetectEncodingXmlFile( rgbyBOM, vknBytesBOM );
      Assert( efceFileCharacterEncodingCount != efceEncoding ); // Unless the source isn't XML the above should succeed.
      VerifyThrowSz( efceFileCharacterEncodingCount != efceEncoding, "Unable to sus encoding for the file[%s].", m_strFileNameOrig.c_str() );
      nbyLenghtBOM = 0;
    }
    
    size_t grfNeedFiles = ( 1 << ( 2 * efceFileCharacterEncodingCount ) ) - 1;
    grfNeedFiles &= ~( ( 1 << efceEncoding ) << ( size_t(fEncodingFromBOM) * efceFileCharacterEncodingCount ) );
    while( grfNeedFiles )
    {
      // This isn't as efficient as it could be but it feng shuis...
      size_t stGenerate = _bv_get_clear_first_set( grfNeedFiles );
      bool fWithBOM = stGenerate >= efceFileCharacterEncodingCount;
      EFileCharacterEncoding efceGenerate = EFileCharacterEncoding( stGenerate - ( fWithBOM ? efceFileCharacterEncodingCount : 0 ) );
      // Name the new file with the base of the original XML file:
      string strConvertedFile = strBaseFile;
      strConvertedFile += "_";
      strConvertedFile += PszCharacterEncodingShort( efceGenerate );
      if ( fWithBOM )
        strConvertedFile += "BOM";
      strConvertedFile += ".xml";
      (void)NFileSeekAndThrow( fo.HFileGet(), nbyLenghtBOM, vkSeekBegin );
      ConvertFileMapped( fo.HFileGet(), efceEncoding, strConvertedFile.c_str(), efceGenerate, fWithBOM );
      VerifyThrow( m_mapFileNamesTestDir.insert( _TyMapTestFiles::value_type( _TyKeyEncodingBOM( efceGenerate, fWithBOM ), strConvertedFile ) ).second );
    }
    // Now copy the original to the output directory - don't modify the name so we know it's the original.
    string strConvertedFile = strBaseFile;
    strConvertedFile += ".xml";
    ConvertFileMapped( fo.HFileGet(), efceEncoding, strConvertedFile.c_str(), efceEncoding, fEncodingFromBOM );
    VerifyThrow( m_mapFileNamesTestDir.insert( _TyMapTestFiles::value_type( _TyKeyEncodingBOM( efceEncoding, fEncodingFromBOM ), strConvertedFile ) ).second );
  }

  // TearDown() is invoked immediately after a test finishes.
  void TearDown() override 
  {
    // Nothing to do in TearDown() - we want to leave the generated unit test files so that they can be analyzed if there are any issues.
  }
public:
  string m_strFileNameOrig;
  bool m_fExpectFailure{false}; // If this is true then we expect the test to fail.
  typedef std::pair< EFileCharacterEncoding, bool > _TyKeyEncodingBOM;
  typedef map< _TyKeyEncodingBOM, string > _TyMapTestFiles;
  _TyMapTestFiles m_mapFileNamesTestDir; // The resultant files that were written to the test directory in the output.
};

XmlpTestEnvironment * vpxteXmlpTestEnvironment{nullptr};

using ::testing::TestWithParam;
using ::testing::Bool;
using ::testing::Values;
using ::testing::Combine;

// XmlpTestParser:
// This just tests a single type of the non-variant parser. It will reject files that it knows aren't in the right config - appropriately.
template < class t_TyXmlTraits >
class XmlpTestParser : public testing::TestWithParam< std::tuple< bool, int > >
{
  typedef XmlpTestParser _TyThis;
  typedef testing::Test _TyBase;
public:
  typedef t_TyXmlTraits _TyXmlTraits;
  typedef xml_parser< _TyXmlTraits > _TyXmlParser;
  typedef typename _TyXmlParser::_TyTransport _TyTransport;
  typedef typename XmlpTestEnvironment::_TyKeyEncodingBOM _TyKeyEncodingBOM;
  typedef typename XmlpTestEnvironment::_TyMapTestFiles _TyMapTestFiles;
protected:
  // SetUp() is run immediately before a test starts.
  void SetUp() override 
  {
    _TyKeyEncodingBOM keyTestFile;
    std::tie(keyTestFile.second, (int&)keyTestFile.first) = GetParam();
    m_citTestFile = vpxteXmlpTestEnvironment->m_mapFileNamesTestDir.find( keyTestFile );
    VerifyThrow( vpxteXmlpTestEnvironment->m_mapFileNamesTestDir.end() != m_citTestFile );
    VerifyThrowSz( FFileExists( m_citTestFile->second.c_str() ), "Test file [%s] not found.", m_citTestFile->second.c_str() );
    m_fExpectFailure = vpxteXmlpTestEnvironment->m_fExpectFailure;
    if ( !m_fExpectFailure )
    {
      // Then we have to check if this file is not the type we can load:
      size_t grfSupportedTransports = _GrfSupportedTransports();
      m_fExpectFailure = !( grfSupportedTransports & ( 1ull << keyTestFile.first ) );
    }
  }
  size_t _GrfSupportedTransports() const
    requires( !TFIsTransportVar_v< _TyTransport > )
  {
    return ( 1ull << _TyTransport::GetSupportedCharacterEncoding() );
  }
  size_t _GrfSupportedTransports() const
    requires( TFIsTransportVar_v< _TyTransport > )
  {
    return _TyTransport::GetSupportedEncodingBitVector();
  }
  // TearDown() is invoked immediately after a test finishes.
  void TearDown() override 
  {
  }
  void TestParser()
  {
    // If we expect to fail for this type of file then we will handle the exception locally, otherwise throw it on to the infrastructure because it will then record the failure appropriately.
    try
    {
      _TryTestParser();
    }
    catch( ... )
    {
      if ( !m_fExpectFailure )
        throw;
    }
  }
  void _TryTestParser()
  {
    typedef _TyXmlParser::_TyReadCursor _TyReadCursor;
    typedef xml_document< _TyXmlTraits > _TyXmlDocVar;
    _TyXmlParser xmlParser;
    _TyReadCursor xmlReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
    _TyXmlDocVar xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
  }
  template < template < class ... > class t_tempTransport >
  void TestParserVarTransport()
  {
    // If we expect to fail for this type of file then we will handle the exception locally, otherwise throw it on to the infrastructure because it will then record the failure appropriately.
    try
    {
      _TryTestParserVarTransport<t_tempTransport>();
    }
    catch( ... )
    {
      if ( !m_fExpectFailure )
        throw;
    }
  }
  template < template < class ... > class t_tempTransport >
  void _TryTestParserVarTransport()
  {
    typedef _TyXmlParser::_TyReadCursor _TyReadCursor;
    typedef xml_document< _TyXmlTraits > _TyXmlDocVar;
    _TyXmlParser xmlParser;
    _TyReadCursor xmlReadCursor = xmlParser.OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
    _TyXmlDocVar xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
  }
public:
  _TyMapTestFiles::const_iterator m_citTestFile;
  bool m_fExpectFailure{false}; // We should only succeed on two types of the ten for this test. On the others we expect failure. Also the test itself may be a failure type test.
};

// We must, unfortunately, enumerate here:
typedef XmlpTestParser< xml_traits< _l_transport_file< char8_t, false_type >, false, false > > vTyTestFileTransportUTF8;
TEST_P( vTyTestFileTransportUTF8, TestFileTransportUTF8 ) { TestParser(); }
// NSE: NoSwitchEndian: ends up being UTF16LE on a little endian machine.
// SE: SwitchEndian
typedef XmlpTestParser< xml_traits< _l_transport_file< char16_t, false_type >, false, false > > vTyTestFileTransportUTF16_NSE;
TEST_P( vTyTestFileTransportUTF16_NSE, TestFileTransportUTF16 ) { TestParser(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char16_t, false_type >, false, false > > vTyTestFileTransportUTF16_SE;
TEST_P( vTyTestFileTransportUTF16_SE, TestFileTransportUTF16 ) { TestParser(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char32_t, false_type >, false, false > > vTyTestFileTransportUTF32_NSE;
TEST_P( vTyTestFileTransportUTF32_NSE, TestFileTransportUTF32 ) { TestParser(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char32_t, false_type >, false, false > > vTyTestFileTransportUTF32_SE;
TEST_P( vTyTestFileTransportUTF32_SE, TestFileTransportUTF32 ) { TestParser(); }

// Give us a set of 10 tests for each scenario above. 8 of those tests will fail appropriately (or not if there is a bug).
INSTANTIATE_TEST_SUITE_P( TestXmlParserFileTransportUTF8, vTyTestFileTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserFileTransportUTF16_NSE, vTyTestFileTransportUTF16_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserFileTransportUTF16_SE, vTyTestFileTransportUTF16_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserFileTransportUTF32_NSE, vTyTestFileTransportUTF32_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserFileTransportUTF32_SE, vTyTestFileTransportUTF32_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
                          //Combine( Values(true), Values( int(efceUTF32LE) ) ) );

typedef _l_transport_var< _l_transport_file< char8_t, false_type >, _l_transport_mapped< char8_t, false_type >, _l_transport_fixedmem< char8_t, false_type > > vTyTransportVarChar8;
typedef XmlpTestParser< xml_traits< vTyTransportVarChar8, false, false > > vTyTestVarTransportUTF8;
TEST_P( vTyTestVarTransportUTF8, TestVarTransportUTF8 ) 
{ 
  // _l_transport_mapped is mostly the same as _l_transport_fixedmem so we don't really need to test _l_transport_fixedmem.
  TestParserVarTransport< _l_transport_file >(); 
  TestParserVarTransport< _l_transport_mapped >(); 
}
INSTANTIATE_TEST_SUITE_P( TestXmlParserVarTransportUTF8, vTyTestVarTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

typedef _l_transport_var< _l_transport_file< char16_t, false_type >, _l_transport_file< char16_t, true_type >, _l_transport_mapped< char16_t, false_type >, _l_transport_mapped< char16_t, true_type >, 
                          _l_transport_fixedmem< char16_t, false_type >, _l_transport_fixedmem< char16_t, true_type > > vTyTransportVarChar16;
typedef XmlpTestParser< xml_traits< vTyTransportVarChar16, false, false > > vTyTestVarTransportUTF16;
TEST_P( vTyTestVarTransportUTF16, TestVarTransportUTF16 ) 
{ 
  TestParserVarTransport< _l_transport_file >(); 
  TestParserVarTransport< _l_transport_mapped >(); 
}
INSTANTIATE_TEST_SUITE_P( TestXmlParserVarTransportUTF16, vTyTestVarTransportUTF16,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

typedef _l_transport_var< _l_transport_file< char32_t, false_type >, _l_transport_file< char32_t, true_type >, _l_transport_mapped< char32_t, false_type >, _l_transport_mapped< char32_t, true_type >, 
                          _l_transport_fixedmem< char32_t, false_type >, _l_transport_fixedmem< char32_t, true_type > > vTyTransportVarChar32;
typedef XmlpTestParser< xml_traits< vTyTransportVarChar32, false, false > > vTyTestVarTransportUTF32;
TEST_P( vTyTestVarTransportUTF32, TestVarTransportUTF32 ) 
{ 
  TestParserVarTransport< _l_transport_file >(); 
  TestParserVarTransport< _l_transport_mapped >(); 
}
INSTANTIATE_TEST_SUITE_P( TestXmlParserVarTransportUTF32, vTyTestVarTransportUTF32,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

// XmlpTestVariantParser: This unit tests a single file in all possible formats - with and without BOM in the file.
// The file is multiplexed by the XmlpTestEnvironment object and then is available as a "global" during
//  the execution of the unittest.
template <  template < class ... > class t_tempTyTransport, 
            class t_TyTp2DCharPack = tuple< tuple< char32_t, true_type >, tuple< char32_t, false_type >, tuple< char16_t, true_type >, tuple< char16_t, false_type >, tuple< char8_t, false_type > > >
            //class t_TyTp2DCharPack = tuple< tuple< char8_t, false_type > > >
class XmlpTestVariantParser : public testing::TestWithParam< std::tuple< bool, int > >
{
  typedef XmlpTestVariantParser _TyThis;
  typedef testing::Test _TyBase;
public:
  typedef t_TyTp2DCharPack _TyTp2DCharPack;
  typedef xml_parser_var< t_tempTyTransport, _TyTp2DCharPack > _TyXmlParser;
  typedef typename XmlpTestEnvironment::_TyKeyEncodingBOM _TyKeyEncodingBOM;
  typedef typename XmlpTestEnvironment::_TyMapTestFiles _TyMapTestFiles;
protected:
  // SetUp() is run immediately before a test starts.
  void SetUp() override 
  {
    _TyKeyEncodingBOM keyTestFile;
    std::tie(keyTestFile.second, (int&)keyTestFile.first) = GetParam();
    m_citTestFile = vpxteXmlpTestEnvironment->m_mapFileNamesTestDir.find( keyTestFile );
    VerifyThrow( vpxteXmlpTestEnvironment->m_mapFileNamesTestDir.end() != m_citTestFile );
    VerifyThrowSz( FFileExists( m_citTestFile->second.c_str() ), "Test file [%s] not found.", m_citTestFile->second.c_str() );
    m_fExpectFailure = vpxteXmlpTestEnvironment->m_fExpectFailure;
  }
  // TearDown() is invoked immediately after a test finishes.
  void TearDown() override 
  {
  }
  void TestParser()
  {
    // If we expect to fail for this type of file then we will handle the exception locally, otherwise throw it on to the infrastructure because it will then record the failure appropriately.
    try
    {
      _TryTestParser();
    }
    catch( ... )
    {
      if ( !m_fExpectFailure )
        throw;
    }
  }
  void _TryTestParser()
  {
    _TyXmlParser xmlParser;
    typedef _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
    typedef _TyXmlParser::_TyTpTransports _TyTpTransports;
    typedef xml_document_var< _TyTpTransports > _TyXmlDocVar;
    _TyReadCursorVar xmlReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
    _TyXmlDocVar xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
  }
public:
  _TyMapTestFiles::const_iterator m_citTestFile;
  bool m_fExpectFailure{false};
};

typedef XmlpTestVariantParser<_l_transport_mapped> vTyTestMappedTransport;
TEST_P( vTyTestMappedTransport, TestMappedTransport )
{
  TestParser();
}

typedef XmlpTestVariantParser<_l_transport_file> vTyTestFileTransport;
TEST_P( vTyTestFileTransport, TestFileTransport )
{
  TestParser();
}

// Give us a set of 10 tests.
INSTANTIATE_TEST_SUITE_P( TestXmlParserMappedTransport, vTyTestMappedTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
                          //Combine( Values(true), Values( int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserFileTransport, vTyTestFileTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

// Test variant transport with the variant parser since that hits all scenarios.


int _TryMain( int argc, char **argv )
{
  if ( argc > 1 ) // We may get called by cmake infrastructure gtest_discover_tests_impl - in that case we should just return RUN_ALL_TESTS().
  {
    //VerifyThrowSz( argc > 1, "Missing file name for unit test file." );
    // We should be running in the executable output directory and we should have a subdirectory created by the build already called "unittests".
    VerifyThrowSz( FDirectoryExists( "unittests" ), "We expect to find a directory called 'unittests' in the build directory." );
    // We expect a fully qualified path name for our file - but the test environment ends up verifying that.
    (void)testing::AddGlobalTestEnvironment( vpxteXmlpTestEnvironment = new XmlpTestEnvironment( argv[1] ) );
  }
  return RUN_ALL_TESTS();
}

} // namespace ns_XMLGTest

int main( int argc, char **argv )
{
  __BIENUTIL_USING_NAMESPACE

  g_strProgramName = argv[0];
  testing::InitGoogleTest(&argc, argv);
  try
  {
    return ns_XMLGTest::_TryMain(argc, argv);
  }
  catch( const std::exception & rexc )
  {
    n_SysLog::Log( eslmtError, "%s: *** Exception: [%s]", g_strProgramName.c_str(), rexc.what() );
    fprintf( stderr, "%s: *** Exception: [%s]\n", g_strProgramName.c_str(), rexc.what() );      
    return -1;
  }
}
