// xmlpgtest_doccopy.cpp

// Google test stuff for XML Parser.
// This tests copy of files by first reading the files into an xml_document or xml_document_var and then writing them out again.
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
#include "xmlpgtest.h"

std::string g_strProgramName;

// When this is true we don't write anything.
static const bool vkfDontTestXmlWrite = false;

namespace ns_XMLGTest
{
__XMLP_USING_NAMESPACE

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
  typedef typename _TyXmlTraits::_TyChar _TyChar;
  typedef xml_parser< _TyXmlTraits > _TyXmlParser;
  typedef typename _TyXmlParser::_TyTransport _TyTransport;
  typedef typename XmlpTestEnvironment::_TyKeyEncodingBOM _TyKeyEncodingBOM;
  typedef typename XmlpTestEnvironment::_TyMapTestFiles _TyMapTestFiles;
protected:
  // SetUp() is run immediately before a test starts.
  void SetUp() override 
  {
    VerifyThrowSz( !!vpxteXmlpTestEnvironment, "No test environment - are you debugging without passing in a filename?" );
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
    typedef typename _TyXmlParser::_TyReadCursor _TyReadCursor;
    typedef xml_document< _TyXmlTraits > _TyXmlDoc;
    auto prXmlDoc = _TyXmlDoc::PrCreateXmlDocument();
    _TyXmlDoc * pXmlDoc = prXmlDoc.first; // We store this separately for now - need to come up with a class infrastructure to store this paradigm.
    {//B
      _TyXmlParser xmlParser;
      _TyReadCursor xmlReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
      pXmlDoc->FromXmlStream( xmlReadCursor, prXmlDoc.second );
      pXmlDoc->AssertValid();
    }//EB
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
    if ( vkfDontTestXmlWrite )
      return;
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
          _WriteXmlDocToFile< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    }
  }
  template < class t_TyXmlWriteTransport >
  void _WriteXmlDocToFile( xml_document< _TyXmlTraits > const & _rxd, filesystem::path _pathOutputPath, const _TyMapTestFiles::value_type * _pvtGoldenFile )
  {
    typedef xml_writer< t_TyXmlWriteTransport > _TyXmlWriter;
    _rxd.AssertValid();
    {//B
      _TyXmlWriter xwWriter;
      xwWriter.SetWriteBOM( _pvtGoldenFile->first.second );
      xwWriter.SetWriteXMLDecl( !_rxd.FPseudoXMLDecl() ); // Only write the XMLDecl if we got one in the source.
      // For this test we won't rewrite the encoding with any new encoding value - we'll just leave it the same.
      // The default operation (and we will test this elsewhere) will be to write the current encoding into the encoding="" statement.
      typedef typename _TyXmlWriter::_TyXMLDeclProperties _TyXMLDeclProperties;
      _TyXMLDeclProperties xdpTranslated( _rxd.GetXMLDeclProperties() );
      
      // Without an output format (and no whitespace token filter, etc) we expect the output to exactly match the input except
      //  for encoding of course. This is the nature of the current unit test.
      xwWriter.OpenFile( _pathOutputPath.string().c_str(), xdpTranslated, nullptr, true ); // true indicates "keep encoding present in XMLDecl".
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
    typedef typename _TyXmlParser::_TyReadCursor _TyReadCursor;
    _TyXmlParser xmlParser;
    _TyReadCursor xmlReadCursor = xmlParser.template OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
    typedef xml_document< _TyXmlTraits > _TyXmlDoc;
    auto prXmlDoc = _TyXmlDoc::PrCreateXmlDocument();
    _TyXmlDoc * pXmlDoc = prXmlDoc.first; // We store this separately for now - need to come up with a class infrastructure to store this paradigm.
    pXmlDoc->FromXmlStream( xmlReadCursor, prXmlDoc.second );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
    if ( vkfDontTestXmlWrite )
      return;
    // Since we succeeded we test writing and then we compare the results.
    size_t grfOutputFiles = ( 1 << ( 2 * efceFileCharacterEncodingCount ) ) - 1;
    while( grfOutputFiles )
    {
      const _TyMapTestFiles::value_type * pvtGoldenFile;
      filesystem::path pathOutput = vpxteXmlpTestEnvironment->GetNextFileNameOutput( m_pathOutputDir, grfOutputFiles, pvtGoldenFile );
      typedef t_tempTransport< _TyChar, false_type > _TyTransport; // Define this just to get the types we care about.
      switch( pvtGoldenFile->first.first )
      {
        case efceUTF8:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char8_t, false_type >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    }
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
    typedef typename _TyXmlParser::_TyReadCursor _TyReadCursor;
    typedef xml_document< _TyXmlTraits > _TyXmlDoc;
    auto prXmlDoc = _TyXmlDoc::PrCreateXmlDocument();
    _TyXmlDoc * pXmlDoc = prXmlDoc.first; // We store this separately for now - need to come up with a class infrastructure to store this paradigm.
    _TyXmlParser xmlParser;
    _TyReadCursor xmlReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
    pXmlDoc->FromXmlStream( xmlReadCursor, prXmlDoc.second );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
    if ( vkfDontTestXmlWrite )
      return;
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
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    }
  }
  template < class t_TyXmlWriteTransport >
  void _WriteXmlDocToMemory( xml_document< _TyXmlTraits > const & _rxd, filesystem::path _pathOutputPath, const _TyMapTestFiles::value_type * _pvtGoldenFile )
  {
    // We'll write the memstream to the file so that we test that part of the MemStream impl as well and then we can have a look
    //  at whatever might be wrong also.
    typedef xml_writer< t_TyXmlWriteTransport > _TyXmlWriter;
    _rxd.AssertValid();
    {//B
      _TyXmlWriter xwWriter;
      xwWriter.SetWriteBOM( _pvtGoldenFile->first.second );
      xwWriter.SetWriteXMLDecl( !_rxd.FPseudoXMLDecl() ); // Only write the XMLDecl if we got one in the source.
      // For this test we won't rewrite the encoding with any new encoding value - we'll just leave it the same.
      // The default operation (and we will test this elsewhere) will be to write the current encoding into the encoding="" statement.
      typedef typename _TyXmlWriter::_TyXMLDeclProperties _TyXMLDeclProperties;
      _TyXMLDeclProperties xdpTranslated( _rxd.GetXMLDeclProperties() );
      
      // Without an output format (and no whitespace token filter, etc) we expect the output to exactly match the input except
      //  for encoding of course. This is the nature of the current unit test.
      xwWriter.OpenMemFile( xdpTranslated, nullptr, true ); // true indicates "keep encoding present in XMLDecl".
      _rxd.ToXmlStream( xwWriter );

      // Now open the file and write the memstream to the file:
      FileObj fo( CreateWriteOnlyFile( _pathOutputPath.string().c_str() ) );
      VerifyThrowSz( fo.FIsOpen(), "Couldn't open output file [%s].", _pathOutputPath.string().c_str() );
      xwWriter.GetTransportOut().GetMemStream().WriteToFile( fo.HFileGet(), 0, xwWriter.GetTransportOut().GetMemStream().GetEndPos() );
    }//EB
    // Now compare the two files:
    vpxteXmlpTestEnvironment->CompareFiles( _pathOutputPath, _pvtGoldenFile );
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
    typedef typename _TyXmlParser::_TyReadCursor _TyReadCursor;
    typedef xml_document< _TyXmlTraits > _TyXmlDoc;
    auto prXmlDoc = _TyXmlDoc::PrCreateXmlDocument();
    _TyXmlDoc * pXmlDoc = prXmlDoc.first; // We store this separately for now - need to come up with a class infrastructure to store this paradigm.
    _TyXmlParser xmlParser;
    _TyReadCursor xmlReadCursor = xmlParser.template OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
    pXmlDoc->FromXmlStream( xmlReadCursor, prXmlDoc.second );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
    if ( vkfDontTestXmlWrite )
      return;
    // Since we succeeded we test writing and then we compare the results.
    size_t grfOutputFiles = ( 1 << ( 2 * efceFileCharacterEncodingCount ) ) - 1;
    while( grfOutputFiles )
    {
      const _TyMapTestFiles::value_type * pvtGoldenFile;
      filesystem::path pathOutput = vpxteXmlpTestEnvironment->GetNextFileNameOutput( m_pathOutputDir, grfOutputFiles, pvtGoldenFile );
      typedef t_tempTransport< _TyChar, false_type > _TyTransport; // Define this just to get the types we care about.
      switch( pvtGoldenFile->first.first )
      {
        case efceUTF8:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char8_t, false_type >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( *pXmlDoc, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    }
  }
public:
  _TyMapTestFiles::const_iterator m_citTestFile;
  filesystem::path m_pathOutputDir;
  bool m_fExpectFailure{false}; // We should only succeed on two types of the ten for this test. On the others we expect failure. Also the test itself may be a failure type test.
};

// Test single transport type parser types:
// For each of these we test the corresponding xml_writer type.
// NSE: NoSwitchEndian: ends up being UTF16LE on a little endian machine.
// SE: SwitchEndian
 // 1) Files.
typedef XmlpTestParser< xml_traits< _l_transport_file< char8_t, false_type >, false, false > > vTyTestXmlDocFileTransportUTF8;
TEST_P( vTyTestXmlDocFileTransportUTF8, TestXmlDocFileTransportUTF8 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char16_t, false_type >, false, false > > vTyTestXmlDocFileTransportUTF16_NSE;
TEST_P( vTyTestXmlDocFileTransportUTF16_NSE, TestXmlDocFileTransportUTF16 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char16_t, false_type >, false, false > > vTyTestXmlDocFileTransportUTF16_SE;
TEST_P( vTyTestXmlDocFileTransportUTF16_SE, TestXmlDocFileTransportUTF16 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char32_t, false_type >, false, false > > vTyTestXmlDocFileTransportUTF32_NSE;
TEST_P( vTyTestXmlDocFileTransportUTF32_NSE, TestXmlDocFileTransportUTF32 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char32_t, false_type >, false, false > > vTyTestXmlDocFileTransportUTF32_SE;
TEST_P( vTyTestXmlDocFileTransportUTF32_SE, TestXmlDocFileTransportUTF32 ) { TestParserFile(); }
// Give us a set of 10 tests for each scenario above. 8 of those tests will fail appropriately (or not if there is a bug).
INSTANTIATE_TEST_SUITE_P( TestXmlDocFileTransport, vTyTestXmlDocFileTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlDocFileTransport, vTyTestXmlDocFileTransportUTF16_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlDocFileTransport, vTyTestXmlDocFileTransportUTF16_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlDocFileTransport, vTyTestXmlDocFileTransportUTF32_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlDocFileTransport, vTyTestXmlDocFileTransportUTF32_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
                          //Combine( Values(true), Values( int(efceUTF32LE) ) ) );
// 2) Mapped files.
typedef XmlpTestParser< xml_traits< _l_transport_mapped< char8_t, false_type >, false, false > > vTyTestXmlDocMappedTransportUTF8;
TEST_P( vTyTestXmlDocMappedTransportUTF8, TestXmlDocMappedTransportUTF8 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_mapped< char16_t, false_type >, false, false > > vTyTestXmlDocMappedTransportUTF16_NSE;
TEST_P( vTyTestXmlDocMappedTransportUTF16_NSE, TestXmlDocMappedTransportUTF16 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_mapped< char16_t, false_type >, false, false > > vTyTestXmlDocMappedTransportUTF16_SE;
TEST_P( vTyTestXmlDocMappedTransportUTF16_SE, TestXmlDocMappedTransportUTF16 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_mapped< char32_t, false_type >, false, false > > vTyTestXmlDocMappedTransportUTF32_NSE;
TEST_P( vTyTestXmlDocMappedTransportUTF32_NSE, TestXmlDocMappedTransportUTF32 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_mapped< char32_t, false_type >, false, false > > vTyTestXmlDocMappedTransportUTF32_SE;
TEST_P( vTyTestXmlDocMappedTransportUTF32_SE, TestXmlDocMappedTransportUTF32 ) { TestParserFile(); }
// Give us a set of 10 tests for each scenario above. 8 of those tests will fail appropriately (or not if there is a bug).
INSTANTIATE_TEST_SUITE_P( TestXmlDocMappedTransport, vTyTestXmlDocMappedTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlDocMappedTransport, vTyTestXmlDocMappedTransportUTF16_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlDocMappedTransport, vTyTestXmlDocMappedTransportUTF16_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlDocMappedTransport, vTyTestXmlDocMappedTransportUTF32_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlDocMappedTransport, vTyTestXmlDocMappedTransportUTF32_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
                          //Combine( Values(true), Values( int(efceUTF32LE) ) ) );
// 3) Memory. We use fixed memory input and memstream output for these.
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char8_t, false_type >, false, false > > vTyTestXmlDocMemoryTransportUTF8;
TEST_P( vTyTestXmlDocMemoryTransportUTF8, TestXmlDocMemoryTransportUTF8 ) { TestParserMemory(); }
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char16_t, false_type >, false, false > > vTyTestXmlDocMemoryTransportUTF16_NSE;
TEST_P( vTyTestXmlDocMemoryTransportUTF16_NSE, TestXmlDocMemoryTransportUTF16 ) { TestParserMemory(); }
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char16_t, false_type >, false, false > > vTyTestXmlDocMemoryTransportUTF16_SE;
TEST_P( vTyTestXmlDocMemoryTransportUTF16_SE, TestXmlDocMemoryTransportUTF16 ) { TestParserMemory(); }
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char32_t, false_type >, false, false > > vTyTestXmlDocMemoryTransportUTF32_NSE;
TEST_P( vTyTestXmlDocMemoryTransportUTF32_NSE, TestXmlDocMemoryTransportUTF32 ) { TestParserMemory(); }
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char32_t, false_type >, false, false > > vTyTestXmlDocMemoryTransportUTF32_SE;
TEST_P( vTyTestXmlDocMemoryTransportUTF32_SE, TestXmlDocMemoryTransportUTF32 ) { TestParserMemory(); }
// Give us a set of 10 tests for each scenario above. 8 of those tests will fail appropriately (or not if there is a bug).
INSTANTIATE_TEST_SUITE_P( TestXmlDocMemoryTransport, vTyTestXmlDocMemoryTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlDocMemoryTransport, vTyTestXmlDocMemoryTransportUTF16_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlDocMemoryTransport, vTyTestXmlDocMemoryTransportUTF16_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlDocMemoryTransport, vTyTestXmlDocMemoryTransportUTF32_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlDocMemoryTransport, vTyTestXmlDocMemoryTransportUTF32_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

typedef _l_transport_var< _l_transport_file< char8_t, false_type >, _l_transport_mapped< char8_t, false_type >, _l_transport_fixedmem< char8_t, false_type > > vTyTransportVarChar8;
typedef XmlpTestParser< xml_traits< vTyTransportVarChar8, false, false > > vTyTestXmlDocVarTransportUTF8;
TEST_P( vTyTestXmlDocVarTransportUTF8, TestXmlDocVarTransportUTF8 ) 
{ 
  // _l_transport_mapped is mostly the same as _l_transport_fixedmem so we don't really need to test _l_transport_fixedmem.
  TestParserFileTransportVar< _l_transport_mapped >(); 
  TestParserFileTransportVar< _l_transport_file >(); 
  TestParserMemoryTransportVar< _l_transport_fixedmem >();
}
INSTANTIATE_TEST_SUITE_P( TestXmlDocVarTransport, vTyTestXmlDocVarTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

typedef _l_transport_var< _l_transport_file< char16_t, false_type >, _l_transport_file< char16_t, true_type >, _l_transport_mapped< char16_t, false_type >, _l_transport_mapped< char16_t, true_type >, 
                          _l_transport_fixedmem< char16_t, false_type >, _l_transport_fixedmem< char16_t, true_type > > vTyTransportVarChar16;
typedef XmlpTestParser< xml_traits< vTyTransportVarChar16, false, false > > vTyTestXmlDocVarTransportUTF16;
TEST_P( vTyTestXmlDocVarTransportUTF16, TestXmlDocVarTransportUTF16 ) 
{ 
  TestParserFileTransportVar< _l_transport_mapped >(); 
  TestParserFileTransportVar< _l_transport_file >(); 
  TestParserMemoryTransportVar< _l_transport_fixedmem >();
}
INSTANTIATE_TEST_SUITE_P( TestXmlDocVarTransport, vTyTestXmlDocVarTransportUTF16,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

typedef _l_transport_var< _l_transport_file< char32_t, false_type >, _l_transport_file< char32_t, true_type >, _l_transport_mapped< char32_t, false_type >, _l_transport_mapped< char32_t, true_type >, 
                          _l_transport_fixedmem< char32_t, false_type >, _l_transport_fixedmem< char32_t, true_type > > vTyTransportVarChar32;
typedef XmlpTestParser< xml_traits< vTyTransportVarChar32, false, false > > vTyTestXmlDocVarTransportUTF32;
TEST_P( vTyTestXmlDocVarTransportUTF32, TestXmlDocVarTransportUTF32 ) 
{ 
  TestParserFileTransportVar< _l_transport_mapped >(); 
  TestParserFileTransportVar< _l_transport_file >();
  TestParserMemoryTransportVar< _l_transport_fixedmem >();
}
INSTANTIATE_TEST_SUITE_P( TestXmlDocVarTransport, vTyTestXmlDocVarTransportUTF32,
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
    VerifyThrowSz( !!vpxteXmlpTestEnvironment, "No test environment - are you debugging without passing in a filename?" );
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
    typedef typename _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
    typedef typename _TyXmlParser::_TyTpTransports _TyTpTransports;
    typedef xml_document_var< _TyTpTransports > _TyXmlDocVar;
    _TyReadCursorVar xmlReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
    _TyXmlDocVar xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
    if ( vkfDontTestXmlWrite )
      return;
    // Since we succeeded we test writing and then we compare the results.
    size_t grfOutputFiles = ( 1 << ( 2 * efceFileCharacterEncodingCount ) ) - 1;
    while( grfOutputFiles )
    {
      const _TyMapTestFiles::value_type * pvtGoldenFile;
      filesystem::path pathOutput = vpxteXmlpTestEnvironment->GetNextFileNameOutput( m_pathOutputDir, grfOutputFiles, pvtGoldenFile );
      typedef t_tempTyTransport< char8_t, false_type > _TyTransport; // Define this just to get the types we care about.
      switch( pvtGoldenFile->first.first )
      {
        case efceUTF8:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char8_t, false_type >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    }
  }
  template < class t_TyXmlWriteTransport >
  void _WriteXmlDocToFile( xml_document_var< typename _TyXmlParser::_TyTpTransports > const & _rxd, filesystem::path _pathOutputPath, const _TyMapTestFiles::value_type * _pvtGoldenFile )
  {
    typedef xml_writer< t_TyXmlWriteTransport > _TyXmlWriter;
    _rxd.AssertValid();
    {//B
      _TyXmlWriter xwWriter;
      xwWriter.SetWriteBOM( _pvtGoldenFile->first.second );
      xwWriter.SetWriteXMLDecl( !_rxd.FPseudoXMLDecl() ); // Only write the XMLDecl if we got one in the source.
      // For this test we won't rewrite the encoding with any new encoding value - we'll just leave it the same.
      // The default operation (and we will test this elsewhere) will be to write the current encoding into the encoding="" statement.
      typedef typename _TyXmlWriter::_TyXMLDeclProperties _TyXMLDeclProperties;
      _TyXMLDeclProperties xdpTranslated;
      _rxd.GetXMLDeclProperties( xdpTranslated );
      
      // Without an output format (and no whitespace token filter, etc) we expect the output to exactly match the input except
      //  for encoding of course. This is the nature of the current unit test.
      xwWriter.OpenFile( _pathOutputPath.string().c_str(), xdpTranslated, nullptr, true ); // true indicates "keep encoding present in XMLDecl".
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
    _TyXmlParser xmlParser;
    typedef typename _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
    typedef typename _TyXmlParser::_TyTpTransports _TyTpTransports;
    typedef xml_document_var< _TyTpTransports > _TyXmlDocVar;
    _TyReadCursorVar xmlReadCursor = xmlParser.template OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
    _TyXmlDocVar xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
    if ( vkfDontTestXmlWrite )
      return;
    // Since we succeeded we test writing and then we compare the results.
    size_t grfOutputFiles = ( 1 << ( 2 * efceFileCharacterEncodingCount ) ) - 1;
    while( grfOutputFiles )
    {
      const _TyMapTestFiles::value_type * pvtGoldenFile;
      filesystem::path pathOutput = vpxteXmlpTestEnvironment->GetNextFileNameOutput( m_pathOutputDir, grfOutputFiles, pvtGoldenFile );
      typedef t_tempTransport< char8_t, false_type > _TyTransport; // Define this just to get the types we care about.
      switch( pvtGoldenFile->first.first )
      {
        case efceUTF8:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char8_t, false_type >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToFile< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    }
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
    typedef typename _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
    typedef typename _TyXmlParser::_TyTpTransports _TyTpTransports;
    typedef xml_document_var< _TyTpTransports > _TyXmlDocVar;
    _TyReadCursorVar xmlReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
    _TyXmlDocVar xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
    if ( vkfDontTestXmlWrite )
      return;
    // Since we succeeded we test writing and then we compare the results.
    size_t grfOutputFiles = ( 1 << ( 2 * efceFileCharacterEncodingCount ) ) - 1;
    while( grfOutputFiles )
    {
      const _TyMapTestFiles::value_type * pvtGoldenFile;
      filesystem::path pathOutput = vpxteXmlpTestEnvironment->GetNextFileNameOutput( m_pathOutputDir, grfOutputFiles, pvtGoldenFile );
      typedef t_tempTyTransport< char8_t, false_type > _TyTransport; // Define this just to get the types we care about.
      switch( pvtGoldenFile->first.first )
      {
        case efceUTF8:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char8_t, false_type >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    }
  }
  template < class t_TyXmlWriteTransport >
  void _WriteXmlDocToMemory( xml_document_var< typename _TyXmlParser::_TyTpTransports > const & _rxd, filesystem::path _pathOutputPath, const _TyMapTestFiles::value_type * _pvtGoldenFile )
  {
    // We'll write the memstream to the file so that we test that part of the MemStream impl as well and then we can have a look
    //  at whatever might be wrong also.
    typedef xml_writer< t_TyXmlWriteTransport > _TyXmlWriter;
    _rxd.AssertValid();
    {//B
      _TyXmlWriter xwWriter;
      xwWriter.SetWriteBOM( _pvtGoldenFile->first.second );
      xwWriter.SetWriteXMLDecl( !_rxd.FPseudoXMLDecl() ); // Only write the XMLDecl if we got one in the source.
      // For this test we won't rewrite the encoding with any new encoding value - we'll just leave it the same.
      // The default operation (and we will test this elsewhere) will be to write the current encoding into the encoding="" statement.
      typedef typename _TyXmlWriter::_TyXMLDeclProperties _TyXMLDeclProperties;
      _TyXMLDeclProperties xdpTranslated;
      _rxd.GetXMLDeclProperties( xdpTranslated );
      
      // Without an output format (and no whitespace token filter, etc) we expect the output to exactly match the input except
      //  for encoding of course. This is the nature of the current unit test.
      xwWriter.OpenMemFile( xdpTranslated, nullptr, true ); // true indicates "keep encoding present in XMLDecl".
      _rxd.ToXmlStream( xwWriter );

      // Now open the file and write the memstream to the file:
      FileObj fo( CreateWriteOnlyFile( _pathOutputPath.string().c_str() ) );
      VerifyThrowSz( fo.FIsOpen(), "Couldn't open output file [%s].", _pathOutputPath.string().c_str() );
      xwWriter.GetTransportOut().GetMemStream().WriteToFile( fo.HFileGet(), 0, xwWriter.GetTransportOut().GetMemStream().GetEndPos() );
    }//EB
    // Now compare the two files:
    vpxteXmlpTestEnvironment->CompareFiles( _pathOutputPath, _pvtGoldenFile );
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
    typedef typename _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
    typedef typename _TyXmlParser::_TyTpTransports _TyTpTransports;
    typedef xml_document_var< _TyTpTransports > _TyXmlDocVar;
    _TyReadCursorVar xmlReadCursor = xmlParser.template OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
    _TyXmlDocVar xmlDocVar;
    xmlDocVar.FromXmlStream( xmlReadCursor );
    VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
    if ( vkfDontTestXmlWrite )
      return;
    // Since we succeeded we test writing and then we compare the results.
    size_t grfOutputFiles = ( 1 << ( 2 * efceFileCharacterEncodingCount ) ) - 1;
    while( grfOutputFiles )
    {
      const _TyMapTestFiles::value_type * pvtGoldenFile;
      filesystem::path pathOutput = vpxteXmlpTestEnvironment->GetNextFileNameOutput( m_pathOutputDir, grfOutputFiles, pvtGoldenFile );
      typedef t_tempTransport< char8_t, false_type > _TyTransport; // Define this just to get the types we care about.
      switch( pvtGoldenFile->first.first )
      {
        case efceUTF8:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char8_t, false_type >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _WriteXmlDocToMemory< _TyXmlWriteTransport >( xmlDocVar, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    }
  }
public:
  _TyMapTestFiles::const_iterator m_citTestFile;
  filesystem::path m_pathOutputDir;
  bool m_fExpectFailure{false};
};

typedef XmlpTestVariantParser<_l_transport_mapped> vTyTestVarXmlDocMappedTransport;
TEST_P( vTyTestVarXmlDocMappedTransport, TestVarXmlDocMappedTransport )
{
  TestParserFile();
}
typedef XmlpTestVariantParser<_l_transport_file> vTyTestVarXmlDocFileTransport;
TEST_P( vTyTestVarXmlDocFileTransport, TestVarXmlDocFileTransport )
{
  TestParserFile();
}
typedef XmlpTestVariantParser<_l_transport_fixedmem> vTyTestVarXmlDocMemoryTransport;
TEST_P( vTyTestVarXmlDocMemoryTransport, TestVarXmlDocMemoryTransport )
{
  TestParserMemory();
}
// Give us some tests.
INSTANTIATE_TEST_SUITE_P( TestVarXmlDoc, vTyTestVarXmlDocMappedTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
                          //Combine( Values(true), Values( int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestVarXmlDoc, vTyTestVarXmlDocFileTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestVarXmlDoc, vTyTestVarXmlDocMemoryTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

// Test variant transport with the variant parser.
typedef XmlpTestVariantParser< xml_var_get_var_transport_t, tuple< tuple< char8_t >, tuple< char16_t >, tuple< char32_t > > > vTyTestVarXmlDocVarTransport;
TEST_P( vTyTestVarXmlDocVarTransport, TestVarXmlDocVarTransport )
{
  TestParserFileTransportVar<_l_transport_mapped>();
  TestParserFileTransportVar<_l_transport_file>();
  TestParserMemoryTransportVar<_l_transport_fixedmem>();
}
INSTANTIATE_TEST_SUITE_P( TestVarXmlDoc, vTyTestVarXmlDocVarTransport,
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
#ifdef WIN32
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif //WIN32
  __BIENUTIL_USING_NAMESPACE

  g_strProgramName = argv[0];
#ifdef WIN32
  {//B
    // Sometimes in windows we get called from cmake and the path is using Linux path separators:
    std::filesystem::path pathProgramName( g_strProgramName );
    g_strProgramName = pathProgramName.lexically_normal().string().c_str();
  }//EB
#endif //WIN32
	n_SysLog::InitSysLog( g_strProgramName.c_str(),  
		LOG_PERROR, LOG_USER
	);

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
