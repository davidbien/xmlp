
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

// XmlpTest: This unit tests a single file in all possible formats - with and without BOM in the file.
// The file is multiplexed by the XmlpTestEnvironment object and then is available as a "global" during
//  the execution of the unittest.
template <  template < class ... > class t_tempTyTransport, 
            class t_TyTp2DCharPack = tuple< tuple< char32_t, true_type >, tuple< char32_t, false_type >, tuple< char16_t, true_type >, tuple< char16_t, false_type >, tuple< char8_t, false_type > > >
            //class t_TyTp2DCharPack = tuple< tuple< char8_t, false_type > > >
class XmlpTest : public testing::TestWithParam< std::tuple< bool, int > >
{
  typedef XmlpTest _TyThis;
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
  }
  // TearDown() is invoked immediately after a test finishes.
  void TearDown() override 
  {
  }
public:
  _TyMapTestFiles::const_iterator m_citTestFile;
};

typedef XmlpTest<_l_transport_mapped> vTyTestMappedTransport;
TEST_P( vTyTestMappedTransport, TestMappedTransport )
{
  _TyXmlParser xmlParser;
  typedef _TyXmlParser::_TyXmlCursorVar _TyXmlCursorVar;
  typedef _TyXmlParser::_TyTpTransports _TyTpTransports;
  typedef xml_document_var< _TyTpTransports > _TyXmlDocVar;
  _TyXmlCursorVar xmlReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
  _TyXmlDocVar xmlDocVar;
  xmlDocVar.FromXmlStream( xmlReadCursor );
}

typedef XmlpTest<_l_transport_file> vTyTestFileTransport;
TEST_P( vTyTestFileTransport, TestFileTransport )
{
  _TyXmlParser xmlParser;
  typedef _TyXmlParser::_TyXmlCursorVar _TyXmlCursorVar;
  typedef _TyXmlParser::_TyTpTransports _TyTpTransports;
  typedef xml_document_var< _TyTpTransports > _TyXmlDocVar;
  _TyXmlCursorVar xmlReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
  _TyXmlDocVar xmlDocVar;
  xmlDocVar.FromXmlStream( xmlReadCursor );
}

// Give us a set of 10 tests.
INSTANTIATE_TEST_SUITE_P( TestXmlParserMappedTransport, vTyTestMappedTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
                          //Combine( Values(true), Values( int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserFileTransport, vTyTestFileTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

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
