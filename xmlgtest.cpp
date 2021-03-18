
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
#include <filesystem>
#include "syslogmgr.inl"
#include "_compat.inl"
#include "xml_inc.h"
#include "xml_tag.h"
#include "xml_tag_var.h"
#include "xml_transport.h"
#include "xml_writer.h"
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
  typedef std::pair< EFileCharacterEncoding, bool > _TyKeyEncodingBOM;
  typedef map< _TyKeyEncodingBOM, string > _TyMapTestFiles;
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
    using namespace filesystem;
    path pathFileOrig( m_strFileNameOrig );
    VerifyThrowSz( exists( pathFileOrig ), "Original test file[%s] doesn't exist.", m_strFileNameOrig.c_str() );
    VerifyThrowSz( pathFileOrig.has_parent_path(), "Need full path to input file[%s] - couldn't find parent path.", m_strFileNameOrig.c_str() );
    VerifyThrowSz( pathFileOrig.has_extension() && ( pathFileOrig.extension() == ".xml" ), "File doesn't have an '.xml' extention [%s].", m_strFileNameOrig.c_str() );
    VerifyThrowSz( pathFileOrig.has_stem(), "No base filename in[%s].", m_strFileNameOrig.c_str() );
    path pathBaseFile( "unittests" );
    m_pathStemOrig = pathFileOrig.stem();
    pathBaseFile /= m_pathStemOrig;
    FileObj fo( OpenReadOnlyFile( m_strFileNameOrig.c_str() ) );
    VerifyThrowSz( fo.FIsOpen(), "Couldn't open file [%s]", m_strFileNameOrig.c_str() );
    uint8_t rgbyBOM[vknBytesBOM];
    size_t nbyLenghtBOM;
    int iResult = FileRead( fo.HFileGet(), rgbyBOM, vknBytesBOM, &nbyLenghtBOM );
    Assert( !iResult );
    Assert( nbyLenghtBOM == vknBytesBOM ); // The smallest valid xml file is 4 bytes... "<t/>".
    VerifyThrowSz( nbyLenghtBOM == vknBytesBOM, "Unable to read [%lu] bytes from the file[%s].", vknBytesBOM, m_strFileNameOrig.c_str() );
    m_fExpectFailure = ( string::npos != m_strFileNameOrig.find("FAIL") ); // simple method for detecting expected failures.
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
    m_grfFileOrig = ( ( 1 << efceEncoding ) << ( size_t(fEncodingFromBOM) * efceFileCharacterEncodingCount ) );
    grfNeedFiles &= ~m_grfFileOrig;
    while( grfNeedFiles )
    {
      // This isn't as efficient as it could be but it feng shuis...
      size_t stGenerate = _bv_get_clear_first_set( grfNeedFiles );
      bool fWithBOM = stGenerate >= efceFileCharacterEncodingCount;
      EFileCharacterEncoding efceGenerate = EFileCharacterEncoding( stGenerate % efceFileCharacterEncodingCount );
      // Name the new file with the base of the original XML file:
      path pathConvertedFile( pathBaseFile );
      pathConvertedFile += "_";
      pathConvertedFile += PszCharacterEncodingShort( efceGenerate );
      if ( fWithBOM )
        pathConvertedFile += "BOM";
      pathConvertedFile += ".xml";
      (void)NFileSeekAndThrow( fo.HFileGet(), nbyLenghtBOM, vkSeekBegin );
      ConvertFileMapped( fo.HFileGet(), efceEncoding, pathConvertedFile.string().c_str(), efceGenerate, fWithBOM );
      VerifyThrow( m_mapFileNamesTestDir.insert( _TyMapTestFiles::value_type( _TyKeyEncodingBOM( efceGenerate, fWithBOM ), pathConvertedFile.string() ) ).second );
    }
    // Now copy the original to the output directory - don't modify the name so we know it's the original.
    path pathConvertedFile( pathBaseFile );
    pathConvertedFile += ".xml";
    ConvertFileMapped( fo.HFileGet(), efceEncoding, pathConvertedFile.string().c_str(), efceEncoding, fEncodingFromBOM );
    VerifyThrow( m_mapFileNamesTestDir.insert( _TyMapTestFiles::value_type( _TyKeyEncodingBOM( efceEncoding, fEncodingFromBOM ), pathConvertedFile.string() ) ).second );

    // Create a directory for the base file - sans extension - this is where the output files for the given unit test will go.
    VerifyThrowSz( create_directory( pathBaseFile ), "Unable to create unittest output directory [%s].", pathBaseFile.string().c_str() );
    m_pathBaseFile = std::move( pathBaseFile );
  }

  // TearDown() is invoked immediately after a test finishes.
  void TearDown() override 
  {
    // Nothing to do in TearDown() - we want to leave the generated unit test files so that they can be analyzed if there are any issues.
  }
public:
  filesystem::path PathCreateUnitTestOutputDirectory() const
  {
    using namespace filesystem;
    const ::testing::TestInfo* const test_info =
    ::testing::UnitTest::GetInstance()->current_test_info();
    path pathSuite = m_pathBaseFile;
    pathSuite /= test_info->test_suite_name();
    path pathTest = test_info->name();
    path::iterator itTestNum = --pathTest.end();
    pathSuite += "_";
    pathSuite += itTestNum->c_str();
    VerifyThrowSz( create_directories( pathSuite ), "Unable to create unittest output directory[%s].", pathSuite.string().c_str() );
    return pathSuite;
  }
  // Return the filename for the output file for the given bit.
  filesystem::path GetNextFileNameOutput( filesystem::path const & _rpathOutputDir, size_t & _rgrfBitOutput, const _TyMapTestFiles::value_type *& _rpvtGoldenFile ) const
  {
    filesystem::path pathOutputFile( _rpathOutputDir );
    size_t stGenerate = _bv_get_clear_first_set( _rgrfBitOutput );
    bool fWithBOM = stGenerate >= efceFileCharacterEncodingCount;
    EFileCharacterEncoding efceEncoding = EFileCharacterEncoding( stGenerate % efceFileCharacterEncodingCount );
    _TyMapTestFiles::const_iterator citFile = m_mapFileNamesTestDir.find( _TyKeyEncodingBOM(efceEncoding, fWithBOM ) );
    _rpvtGoldenFile = &*citFile;
    VerifyThrow( m_mapFileNamesTestDir.end() != citFile ); // unexpected.
    pathOutputFile /= m_pathStemOrig;
    if ( ( 1ull << stGenerate ) != m_grfFileOrig )
    {
      pathOutputFile += "_";
      pathOutputFile += PszCharacterEncodingShort( efceEncoding );
      if ( fWithBOM )
        pathOutputFile += "BOM";
    }
    pathOutputFile += ".xml";
    return pathOutputFile;
  }
  void CompareFiles( filesystem::path const & _rpathOutputFile, const _TyMapTestFiles::value_type * _pvtGoldenFile )
  {
  }
   
  string m_strFileNameOrig;
  filesystem::path m_pathStemOrig;
  size_t m_grfFileOrig{0}; // The bit for the original file so we can match the name in the output.
  filesystem::path m_pathBaseFile;
  bool m_fExpectFailure{false}; // If this is true then we expect the test to fail.
  _TyMapTestFiles m_mapFileNamesTestDir; // The resultant files that were written to the test directory in the output.
};

XmlpTestEnvironment * vpxteXmlpTestEnvironment{nullptr};

using ::testing::TestWithParam;
using ::testing::Bool;
using ::testing::Values;
using ::testing::Combine;

void MapFileForMemoryTest( const char * _pszFileName, FileMappingObj & _rfmo, size_t & _nbyBytesFile )
{
  FileObj fo( OpenReadOnlyFile( _pszFileName ) );
  VerifyThrowSz( fo.FIsOpen(), "Couldn't open file [%s]", _pszFileName );
  FileMappingObj fmo( MapReadOnlyHandle( fo.HFileGet(), &_nbyBytesFile ) );
  VerifyThrowSz( fmo.FIsOpen(), "Couldn't map file [%s]", _pszFileName );
  _rfmo = std::move( fmo );
}

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
    m_pathOutputDir = vpxteXmlpTestEnvironment->PathCreateUnitTestOutputDirectory();
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
  void TestParserFile()
  {
    // If we expect to fail for this type of file then we will handle the exception locally, otherwise throw it on to the infrastructure because it will then record the failure appropriately.
    try
    {
      _TryTestParserFile();
    }
    catch( ... )
    {
      if ( !m_fExpectFailure )
        throw;
    }
  }
  void _TryTestParserFile()
  {
    typedef _TyXmlParser::_TyReadCursor _TyReadCursor;
    typedef xml_document< _TyXmlTraits > _TyXmlDoc;
    _TyXmlDoc xmlDoc;
    {
      _TyXmlParser xmlParser;
      _TyReadCursor xmlReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
      xmlDoc.FromXmlStream( xmlReadCursor );
    }
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
    // Since we succeeded we test writing and then we compare the results.
    size_t grfOutputFiles = ( 1 << ( 2 * efceFileCharacterEncodingCount ) ) - 1;
    while( grfOutputFiles )
    {
      const _TyMapTestFiles::value_type * pvtGoldenFile;
      filesystem::path pathOutput = vpxteXmlpTestEnvironment->GetNextFileNameOutput( m_pathOutputDir, grfOutputFiles, pvtGoldenFile );
      switch( pvtGoldenFile->first.first )
      {
        case efceUTF8:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char8_t, false_type >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDoc< _TyXmlWriteTransport >( xmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDoc< _TyXmlWriteTransport >( xmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDoc< _TyXmlWriteTransport >( xmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDoc< _TyXmlWriteTransport >( xmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDoc< _TyXmlWriteTransport >( xmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    }
  }
  template < class t_TyXmlWriteTransport >
  void _WriteXmlDoc( xml_document< _TyXmlTraits > const & _rxd, filesystem::path _pathOutputPath, const _TyMapTestFiles::value_type * _pvtGoldenFile )
  {
    typedef xml_writer< t_TyXmlWriteTransport > _TyXmlWriter;
    {//B
      _TyXmlWriter xwWriter;
      xwWriter.SetWriteBOM( _pvtGoldenFile->first.second );
      // Without an output format (and no whitespace token filter, etc) we expect the output to exactly match the input except
      //  for encoding of course. This is the nature of the current unit test.
      xwWriter.OpenFile( _pathOutputPath.string().c_str() );
      _rxd.ToXmlStream( xwWriter );
    }//EB
    // Now compare the two files:
    vpxteXmlpTestEnvironment->CompareFiles( _pathOutputPath, _pvtGoldenFile );
  }

  template < template < class ... > class t_tempTransport >
  void TestParserFileTransportVar()
  {
    // If we expect to fail for this type of file then we will handle the exception locally, otherwise throw it on to the infrastructure because it will then record the failure appropriately.
    try
    {
      _TryTestParserFileTransportVar<t_tempTransport>();
    }
    catch( ... )
    {
      if ( !m_fExpectFailure )
        throw;
    }
  }
  template < template < class ... > class t_tempTransport >
  void _TryTestParserFileTransportVar()
  {
    typedef _TyXmlParser::_TyReadCursor _TyReadCursor;
    typedef xml_document< _TyXmlTraits > _TyXmlDoc;
    _TyXmlParser xmlParser;
    _TyReadCursor xmlReadCursor = xmlParser.OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
    _TyXmlDoc xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
  }
  void TestParserMemory()
  {
    // If we expect to fail for this type of file then we will handle the exception locally, otherwise throw it on to the infrastructure because it will then record the failure appropriately.
    try
    {
      _TryTestParserMemory();
    }
    catch( ... )
    {
      if ( !m_fExpectFailure )
        throw;
    }
  }
  void _TryTestParserMemory()
  {
    FileMappingObj fmo;
    size_t nbySizeBytes;
    MapFileForMemoryTest( m_citTestFile->second.c_str(), fmo, nbySizeBytes );
    typedef _TyXmlParser::_TyReadCursor _TyReadCursor;
    typedef xml_document< _TyXmlTraits > _TyXmlDoc;
    _TyXmlParser xmlParser;
    _TyReadCursor xmlReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
    _TyXmlDoc xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
  }
  template < template < class ... > class t_tempTransport >
  void TestParserMemoryTransportVar()
  {
    // If we expect to fail for this type of file then we will handle the exception locally, otherwise throw it on to the infrastructure because it will then record the failure appropriately.
    try
    {
      _TryTestParserMemoryTransportVar<t_tempTransport>();
    }
    catch( ... )
    {
      if ( !m_fExpectFailure )
        throw;
    }
  }
  template < template < class ... > class t_tempTransport >
  void _TryTestParserMemoryTransportVar()
  {
    FileMappingObj fmo;
    size_t nbySizeBytes;
    MapFileForMemoryTest( m_citTestFile->second.c_str(), fmo, nbySizeBytes );
    typedef _TyXmlParser::_TyReadCursor _TyReadCursor;
    typedef xml_document< _TyXmlTraits > _TyXmlDoc;
    _TyXmlParser xmlParser;
    _TyReadCursor xmlReadCursor = xmlParser.OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
    _TyXmlDoc xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
  }
public:
  _TyMapTestFiles::const_iterator m_citTestFile;
  filesystem::path m_pathOutputDir;
  bool m_fExpectFailure{false}; // We should only succeed on two types of the ten for this test. On the others we expect failure. Also the test itself may be a failure type test.
};

// Test single transport type parser types: 1) Files.
// NSE: NoSwitchEndian: ends up being UTF16LE on a little endian machine.
// SE: SwitchEndian
typedef XmlpTestParser< xml_traits< _l_transport_file< char8_t, false_type >, false, false > > vTyTestFileTransportUTF8;
TEST_P( vTyTestFileTransportUTF8, TestFileTransportUTF8 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char16_t, false_type >, false, false > > vTyTestFileTransportUTF16_NSE;
TEST_P( vTyTestFileTransportUTF16_NSE, TestFileTransportUTF16 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char16_t, false_type >, false, false > > vTyTestFileTransportUTF16_SE;
TEST_P( vTyTestFileTransportUTF16_SE, TestFileTransportUTF16 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char32_t, false_type >, false, false > > vTyTestFileTransportUTF32_NSE;
TEST_P( vTyTestFileTransportUTF32_NSE, TestFileTransportUTF32 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char32_t, false_type >, false, false > > vTyTestFileTransportUTF32_SE;
TEST_P( vTyTestFileTransportUTF32_SE, TestFileTransportUTF32 ) { TestParserFile(); }

// Give us a set of 10 tests for each scenario above. 8 of those tests will fail appropriately (or not if there is a bug).
INSTANTIATE_TEST_SUITE_P( TestXmlParserFileTransport, vTyTestFileTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserFileTransport, vTyTestFileTransportUTF16_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserFileTransport, vTyTestFileTransportUTF16_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserFileTransport, vTyTestFileTransportUTF32_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserFileTransport, vTyTestFileTransportUTF32_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
                          //Combine( Values(true), Values( int(efceUTF32LE) ) ) );
// 2) Memory.
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char8_t, false_type >, false, false > > vTyTestMemoryTransportUTF8;
TEST_P( vTyTestMemoryTransportUTF8, TestMemoryTransportUTF8 ) { TestParserMemory(); }
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char16_t, false_type >, false, false > > vTyTestMemoryTransportUTF16_NSE;
TEST_P( vTyTestMemoryTransportUTF16_NSE, TestMemoryTransportUTF16 ) { TestParserMemory(); }
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char16_t, false_type >, false, false > > vTyTestMemoryTransportUTF16_SE;
TEST_P( vTyTestMemoryTransportUTF16_SE, TestMemoryTransportUTF16 ) { TestParserMemory(); }
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char32_t, false_type >, false, false > > vTyTestMemoryTransportUTF32_NSE;
TEST_P( vTyTestMemoryTransportUTF32_NSE, TestMemoryTransportUTF32 ) { TestParserMemory(); }
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char32_t, false_type >, false, false > > vTyTestMemoryTransportUTF32_SE;
TEST_P( vTyTestMemoryTransportUTF32_SE, TestMemoryTransportUTF32 ) { TestParserMemory(); }

// Give us a set of 10 tests for each scenario above. 8 of those tests will fail appropriately (or not if there is a bug).
INSTANTIATE_TEST_SUITE_P( TestXmlParserMemoryTransport, vTyTestMemoryTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserMemoryTransport, vTyTestMemoryTransportUTF16_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserMemoryTransport, vTyTestMemoryTransportUTF16_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserMemoryTransport, vTyTestMemoryTransportUTF32_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlParserMemoryTransport, vTyTestMemoryTransportUTF32_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );


typedef _l_transport_var< _l_transport_file< char8_t, false_type >, _l_transport_mapped< char8_t, false_type >, _l_transport_fixedmem< char8_t, false_type > > vTyTransportVarChar8;
typedef XmlpTestParser< xml_traits< vTyTransportVarChar8, false, false > > vTyTestVarTransportUTF8;
TEST_P( vTyTestVarTransportUTF8, TestVarTransportUTF8 ) 
{ 
  // _l_transport_mapped is mostly the same as _l_transport_fixedmem so we don't really need to test _l_transport_fixedmem.
  TestParserFileTransportVar< _l_transport_mapped >(); 
  TestParserFileTransportVar< _l_transport_file >(); 
  TestParserMemoryTransportVar< _l_transport_fixedmem >();
}
INSTANTIATE_TEST_SUITE_P( TestXmlParserVarTransport, vTyTestVarTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

typedef _l_transport_var< _l_transport_file< char16_t, false_type >, _l_transport_file< char16_t, true_type >, _l_transport_mapped< char16_t, false_type >, _l_transport_mapped< char16_t, true_type >, 
                          _l_transport_fixedmem< char16_t, false_type >, _l_transport_fixedmem< char16_t, true_type > > vTyTransportVarChar16;
typedef XmlpTestParser< xml_traits< vTyTransportVarChar16, false, false > > vTyTestVarTransportUTF16;
TEST_P( vTyTestVarTransportUTF16, TestVarTransportUTF16 ) 
{ 
  TestParserFileTransportVar< _l_transport_mapped >(); 
  TestParserFileTransportVar< _l_transport_file >(); 
  TestParserMemoryTransportVar< _l_transport_fixedmem >();
}
INSTANTIATE_TEST_SUITE_P( TestXmlParserVarTransport, vTyTestVarTransportUTF16,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

typedef _l_transport_var< _l_transport_file< char32_t, false_type >, _l_transport_file< char32_t, true_type >, _l_transport_mapped< char32_t, false_type >, _l_transport_mapped< char32_t, true_type >, 
                          _l_transport_fixedmem< char32_t, false_type >, _l_transport_fixedmem< char32_t, true_type > > vTyTransportVarChar32;
typedef XmlpTestParser< xml_traits< vTyTransportVarChar32, false, false > > vTyTestVarTransportUTF32;
TEST_P( vTyTestVarTransportUTF32, TestVarTransportUTF32 ) 
{ 
  TestParserFileTransportVar< _l_transport_mapped >(); 
  TestParserFileTransportVar< _l_transport_file >();
  TestParserMemoryTransportVar< _l_transport_fixedmem >();
}
INSTANTIATE_TEST_SUITE_P( TestXmlParserVarTransport, vTyTestVarTransportUTF32,
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
    m_pathOutputDir = vpxteXmlpTestEnvironment->PathCreateUnitTestOutputDirectory();
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
  void TestParserFile()
  {
    // If we expect to fail for this type of file then we will handle the exception locally, otherwise throw it on to the infrastructure because it will then record the failure appropriately.
    try
    {
      _TryTestParserFile();
    }
    catch( ... )
    {
      if ( !m_fExpectFailure )
        throw;
    }
  }
  void _TryTestParserFile()
  {
    _TyXmlParser xmlParser;
    typedef _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
    typedef _TyXmlParser::_TyTpTransports _TyTpTransports;
    typedef xml_document_var< _TyTpTransports > _TyXmlDocVar;
    _TyReadCursorVar xmlReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
    _TyXmlDocVar xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
  }
  template < template < class ... > class t_tempTransport >
  void TestParserFileTransportVar()
  {
    // If we expect to fail for this type of file then we will handle the exception locally, otherwise throw it on to the infrastructure because it will then record the failure appropriately.
    try
    {
      _TryTestParserFileTransportVar<t_tempTransport>();
    }
    catch( ... )
    {
      if ( !m_fExpectFailure )
        throw;
    }
  }
  template < template < class ... > class t_tempTransport >
  void _TryTestParserFileTransportVar()
  {
    _TyXmlParser xmlParser;
    typedef _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
    typedef _TyXmlParser::_TyTpTransports _TyTpTransports;
    typedef xml_document_var< _TyTpTransports > _TyXmlDocVar;
    _TyReadCursorVar xmlReadCursor = xmlParser.OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
    _TyXmlDocVar xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
  }
  void TestParserMemory()
  {
    // If we expect to fail for this type of file then we will handle the exception locally, otherwise throw it on to the infrastructure because it will then record the failure appropriately.
    try
    {
      _TryTestParserMemory();
    }
    catch( ... )
    {
      if ( !m_fExpectFailure )
        throw;
    }
  }
  void _TryTestParserMemory()
  {
    FileMappingObj fmo;
    size_t nbySizeBytes;
    MapFileForMemoryTest( m_citTestFile->second.c_str(), fmo, nbySizeBytes );
    _TyXmlParser xmlParser;
    typedef _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
    typedef _TyXmlParser::_TyTpTransports _TyTpTransports;
    typedef xml_document_var< _TyTpTransports > _TyXmlDocVar;
    _TyReadCursorVar xmlReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
    _TyXmlDocVar xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
  }
  template < template < class ... > class t_tempTransport >
  void TestParserMemoryTransportVar()
  {
    // If we expect to fail for this type of file then we will handle the exception locally, otherwise throw it on to the infrastructure because it will then record the failure appropriately.
    try
    {
      _TryTestParserMemoryTransportVar<t_tempTransport>();
    }
    catch( ... )
    {
      if ( !m_fExpectFailure )
        throw;
    }
  }
  template < template < class ... > class t_tempTransport >
  void _TryTestParserMemoryTransportVar()
  {
    FileMappingObj fmo;
    size_t nbySizeBytes;
    MapFileForMemoryTest( m_citTestFile->second.c_str(), fmo, nbySizeBytes );
    _TyXmlParser xmlParser;
    typedef _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
    typedef _TyXmlParser::_TyTpTransports _TyTpTransports;
    typedef xml_document_var< _TyTpTransports > _TyXmlDocVar;
    _TyReadCursorVar xmlReadCursor = xmlParser.OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
    _TyXmlDocVar xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
  }
public:
  _TyMapTestFiles::const_iterator m_citTestFile;
  filesystem::path m_pathOutputDir;
  bool m_fExpectFailure{false};
};

typedef XmlpTestVariantParser<_l_transport_mapped> vTyTestVarParserMappedTransport;
TEST_P( vTyTestVarParserMappedTransport, TestVarParserMappedTransport )
{
  TestParserFile();
}
typedef XmlpTestVariantParser<_l_transport_file> vTyTestVarParserFileTransport;
TEST_P( vTyTestVarParserFileTransport, TestVarParserFileTransport )
{
  TestParserFile();
}
typedef XmlpTestVariantParser<_l_transport_fixedmem> vTyTestVarParserMemoryTransport;
TEST_P( vTyTestVarParserMemoryTransport, TestVarParserMemoryTransport )
{
  TestParserMemory();
}
// Give us some tests.
INSTANTIATE_TEST_SUITE_P( TestXmlVarParser, vTyTestVarParserMappedTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
                          //Combine( Values(true), Values( int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlVarParser, vTyTestVarParserFileTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlVarParser, vTyTestVarParserMemoryTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

// Test variant transport with the variant parser.
typedef XmlpTestVariantParser< xml_var_get_var_transport_t, tuple< tuple< char8_t >, tuple< char16_t >, tuple< char32_t > > > vTyTestVarParserVarTransport;
TEST_P( vTyTestVarParserVarTransport, TestVarParserVarTransport )
{
  TestParserFileTransportVar<_l_transport_mapped>();
  TestParserFileTransportVar<_l_transport_file>();
  TestParserMemoryTransportVar<_l_transport_fixedmem>();
}
INSTANTIATE_TEST_SUITE_P( TestXmlVarParser, vTyTestVarParserVarTransport,
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
