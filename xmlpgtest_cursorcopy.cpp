
// xmlpgtest_cursorcopy.cpp
// Google test stuff for XML Parser.
// This tests copying files using cursors and non-recursive copying.
// dbien
// 06APR2021

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
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    } // while( grfOutputFiles )
  }
  template < class t_TyXmlWriteTransport >
  void _WriteCursorToFile( typename _TyXmlParser::_TyReadCursor & _rxrc, filesystem::path _pathOutputPath, const _TyMapTestFiles::value_type * _pvtGoldenFile )
  {
    typedef xml_writer< t_TyXmlWriteTransport > _TyXmlWriter;
    _rxrc.AssertValid();
    {//B
      _TyXmlWriter xwWriter;
      xwWriter.SetWriteBOM( _pvtGoldenFile->first.second );
      xwWriter.SetWriteXMLDecl( !_rxrc.FPseudoXMLDecl() ); // Only write the XMLDecl if we got one in the source.
      // For this test we won't rewrite the encoding with any new encoding value - we'll just leave it the same.
      // The default operation (and we will test this elsewhere) will be to write the current encoding into the encoding="" statement.
      typedef typename _TyXmlWriter::_TyXMLDeclProperties _TyXMLDeclProperties;
      _TyXMLDeclProperties xdpTranslated( _rxrc.GetXMLDeclProperties() );
      
      // Without an output format (and no whitespace token filter, etc) we expect the output to exactly match the input except
      //  for encoding of course. This is the nature of the current unit test.
      xwWriter.OpenFile( _pathOutputPath.string().c_str(), xdpTranslated, nullptr, true ); // true indicates "keep encoding present in XMLDecl".
      size_t nTagsWritten = xwWriter.NWriteFromReadCursor( _rxrc );
      Assert( 1 == nTagsWritten ); // since we started in the prologue.
      VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
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
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.template OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.template OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.template OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.template OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.template OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
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
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    }
  }
  template < class t_TyXmlWriteTransport >
  void _WriteCursorToMemory( typename _TyXmlParser::_TyReadCursor & _rxrc, filesystem::path _pathOutputPath, const _TyMapTestFiles::value_type * _pvtGoldenFile )
  {
    // We'll write the memstream to the file so that we test that part of the MemStream impl as well and then we can have a look
    //  at whatever might be wrong also.
    typedef xml_writer< t_TyXmlWriteTransport > _TyXmlWriter;
    _rxrc.AssertValid();
    {//B
      _TyXmlWriter xwWriter;
      xwWriter.SetWriteBOM( _pvtGoldenFile->first.second );
      xwWriter.SetWriteXMLDecl( !_rxrc.FPseudoXMLDecl() ); // Only write the XMLDecl if we got one in the source.
      // For this test we won't rewrite the encoding with any new encoding value - we'll just leave it the same.
      // The default operation (and we will test this elsewhere) will be to write the current encoding into the encoding="" statement.
      typedef typename _TyXmlWriter::_TyXMLDeclProperties _TyXMLDeclProperties;
      _TyXMLDeclProperties xdpTranslated( _rxrc.GetXMLDeclProperties() );
      
      // Without an output format (and no whitespace token filter, etc) we expect the output to exactly match the input except
      //  for encoding of course. This is the nature of the current unit test.
      xwWriter.OpenMemFile( xdpTranslated, nullptr, true ); // true indicates "keep encoding present in XMLDecl".
      size_t nTagsWritten = xwWriter.NWriteFromReadCursor( _rxrc );
      Assert( 1 == nTagsWritten ); // since we started in the prologue.
      VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );

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
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.template OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.template OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.template OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.template OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursor xrcReadCursor = xmlParser.template OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
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
typedef XmlpTestParser< xml_traits< _l_transport_file< char8_t, false_type >, false, false > > vTyTestXmlCursorFileTransportUTF8;
TEST_P( vTyTestXmlCursorFileTransportUTF8, TestXmlCursorFileTransportUTF8 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char16_t, false_type >, false, false > > vTyTestXmlCursorFileTransportUTF16_NSE;
TEST_P( vTyTestXmlCursorFileTransportUTF16_NSE, TestXmlCursorFileTransportUTF16 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char16_t, false_type >, false, false > > vTyTestXmlCursorFileTransportUTF16_SE;
TEST_P( vTyTestXmlCursorFileTransportUTF16_SE, TestXmlCursorFileTransportUTF16 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char32_t, false_type >, false, false > > vTyTestXmlCursorFileTransportUTF32_NSE;
TEST_P( vTyTestXmlCursorFileTransportUTF32_NSE, TestXmlCursorFileTransportUTF32 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_file< char32_t, false_type >, false, false > > vTyTestXmlCursorFileTransportUTF32_SE;
TEST_P( vTyTestXmlCursorFileTransportUTF32_SE, TestXmlCursorFileTransportUTF32 ) { TestParserFile(); }
// Give us a set of 10 tests for each scenario above. 8 of those tests will fail appropriately (or not if there is a bug).
INSTANTIATE_TEST_SUITE_P( TestXmlCursorFileTransport, vTyTestXmlCursorFileTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlCursorFileTransport, vTyTestXmlCursorFileTransportUTF16_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlCursorFileTransport, vTyTestXmlCursorFileTransportUTF16_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlCursorFileTransport, vTyTestXmlCursorFileTransportUTF32_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlCursorFileTransport, vTyTestXmlCursorFileTransportUTF32_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
                          //Combine( Values(true), Values( int(efceUTF32LE) ) ) );
// 2) Mapped files.
typedef XmlpTestParser< xml_traits< _l_transport_mapped< char8_t, false_type >, false, false > > vTyTestXmlCursorMappedTransportUTF8;
TEST_P( vTyTestXmlCursorMappedTransportUTF8, TestXmlCursorMappedTransportUTF8 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_mapped< char16_t, false_type >, false, false > > vTyTestXmlCursorMappedTransportUTF16_NSE;
TEST_P( vTyTestXmlCursorMappedTransportUTF16_NSE, TestXmlCursorMappedTransportUTF16 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_mapped< char16_t, false_type >, false, false > > vTyTestXmlCursorMappedTransportUTF16_SE;
TEST_P( vTyTestXmlCursorMappedTransportUTF16_SE, TestXmlCursorMappedTransportUTF16 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_mapped< char32_t, false_type >, false, false > > vTyTestXmlCursorMappedTransportUTF32_NSE;
TEST_P( vTyTestXmlCursorMappedTransportUTF32_NSE, TestXmlCursorMappedTransportUTF32 ) { TestParserFile(); }
typedef XmlpTestParser< xml_traits< _l_transport_mapped< char32_t, false_type >, false, false > > vTyTestXmlCursorMappedTransportUTF32_SE;
TEST_P( vTyTestXmlCursorMappedTransportUTF32_SE, TestXmlCursorMappedTransportUTF32 ) { TestParserFile(); }
// Give us a set of 10 tests for each scenario above. 8 of those tests will fail appropriately (or not if there is a bug).
INSTANTIATE_TEST_SUITE_P( TestXmlCursorMappedTransport, vTyTestXmlCursorMappedTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlCursorMappedTransport, vTyTestXmlCursorMappedTransportUTF16_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlCursorMappedTransport, vTyTestXmlCursorMappedTransportUTF16_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlCursorMappedTransport, vTyTestXmlCursorMappedTransportUTF32_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlCursorMappedTransport, vTyTestXmlCursorMappedTransportUTF32_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
                          //Combine( Values(true), Values( int(efceUTF32LE) ) ) );
// 3) Memory. We use fixed memory input and memstream output for these.
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char8_t, false_type >, false, false > > vTyTestXmlCursorMemoryTransportUTF8;
TEST_P( vTyTestXmlCursorMemoryTransportUTF8, TestXmlCursorMemoryTransportUTF8 ) { TestParserMemory(); }
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char16_t, false_type >, false, false > > vTyTestXmlCursorMemoryTransportUTF16_NSE;
TEST_P( vTyTestXmlCursorMemoryTransportUTF16_NSE, TestXmlCursorMemoryTransportUTF16 ) { TestParserMemory(); }
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char16_t, false_type >, false, false > > vTyTestXmlCursorMemoryTransportUTF16_SE;
TEST_P( vTyTestXmlCursorMemoryTransportUTF16_SE, TestXmlCursorMemoryTransportUTF16 ) { TestParserMemory(); }
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char32_t, false_type >, false, false > > vTyTestXmlCursorMemoryTransportUTF32_NSE;
TEST_P( vTyTestXmlCursorMemoryTransportUTF32_NSE, TestXmlCursorMemoryTransportUTF32 ) { TestParserMemory(); }
typedef XmlpTestParser< xml_traits< _l_transport_fixedmem< char32_t, false_type >, false, false > > vTyTestXmlCursorMemoryTransportUTF32_SE;
TEST_P( vTyTestXmlCursorMemoryTransportUTF32_SE, TestXmlCursorMemoryTransportUTF32 ) { TestParserMemory(); }
// Give us a set of 10 tests for each scenario above. 8 of those tests will fail appropriately (or not if there is a bug).
INSTANTIATE_TEST_SUITE_P( TestXmlCursorMemoryTransport, vTyTestXmlCursorMemoryTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlCursorMemoryTransport, vTyTestXmlCursorMemoryTransportUTF16_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlCursorMemoryTransport, vTyTestXmlCursorMemoryTransportUTF16_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlCursorMemoryTransport, vTyTestXmlCursorMemoryTransportUTF32_NSE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestXmlCursorMemoryTransport, vTyTestXmlCursorMemoryTransportUTF32_SE,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

typedef _l_transport_var< _l_transport_file< char8_t, false_type >, _l_transport_mapped< char8_t, false_type >, _l_transport_fixedmem< char8_t, false_type > > vTyTransportVarChar8;
typedef XmlpTestParser< xml_traits< vTyTransportVarChar8, false, false > > vTyTestXmlCursorVarTransportUTF8;
TEST_P( vTyTestXmlCursorVarTransportUTF8, TestXmlCursorVarTransportUTF8 ) 
{ 
  // _l_transport_mapped is mostly the same as _l_transport_fixedmem so we don't really need to test _l_transport_fixedmem.
  TestParserFileTransportVar< _l_transport_mapped >(); 
  TestParserFileTransportVar< _l_transport_file >(); 
  TestParserMemoryTransportVar< _l_transport_fixedmem >();
}
INSTANTIATE_TEST_SUITE_P( TestXmlCursorVarTransport, vTyTestXmlCursorVarTransportUTF8,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

typedef _l_transport_var< _l_transport_file< char16_t, false_type >, _l_transport_file< char16_t, true_type >, _l_transport_mapped< char16_t, false_type >, _l_transport_mapped< char16_t, true_type >, 
                          _l_transport_fixedmem< char16_t, false_type >, _l_transport_fixedmem< char16_t, true_type > > vTyTransportVarChar16;
typedef XmlpTestParser< xml_traits< vTyTransportVarChar16, false, false > > vTyTestXmlCursorVarTransportUTF16;
TEST_P( vTyTestXmlCursorVarTransportUTF16, TestXmlCursorVarTransportUTF16 ) 
{ 
  TestParserFileTransportVar< _l_transport_mapped >(); 
  TestParserFileTransportVar< _l_transport_file >(); 
  TestParserMemoryTransportVar< _l_transport_fixedmem >();
}
INSTANTIATE_TEST_SUITE_P( TestXmlCursorVarTransport, vTyTestXmlCursorVarTransportUTF16,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

typedef _l_transport_var< _l_transport_file< char32_t, false_type >, _l_transport_file< char32_t, true_type >, _l_transport_mapped< char32_t, false_type >, _l_transport_mapped< char32_t, true_type >, 
                          _l_transport_fixedmem< char32_t, false_type >, _l_transport_fixedmem< char32_t, true_type > > vTyTransportVarChar32;
typedef XmlpTestParser< xml_traits< vTyTransportVarChar32, false, false > > vTyTestXmlCursorVarTransportUTF32;
TEST_P( vTyTestXmlCursorVarTransportUTF32, TestXmlCursorVarTransportUTF32 ) 
{ 
  TestParserFileTransportVar< _l_transport_mapped >(); 
  TestParserFileTransportVar< _l_transport_file >();
  TestParserMemoryTransportVar< _l_transport_fixedmem >();
}
INSTANTIATE_TEST_SUITE_P( TestXmlCursorVarTransport, vTyTestXmlCursorVarTransportUTF32,
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
    typedef typename _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
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
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.OpenFile( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    }
  }
  template < class t_TyXmlWriteTransport >
  void _WriteCursorToFile( typename _TyXmlParser::_TyReadCursorVar & _rxrc, filesystem::path _pathOutputPath, const _TyMapTestFiles::value_type * _pvtGoldenFile )
  {
    typedef xml_writer< t_TyXmlWriteTransport > _TyXmlWriter;
    _rxrc.AssertValid();
    {//B
      _TyXmlWriter xwWriter;
      xwWriter.SetWriteBOM( _pvtGoldenFile->first.second );
      xwWriter.SetWriteXMLDecl( !_rxrc.FPseudoXMLDecl() ); // Only write the XMLDecl if we got one in the source.
      // For this test we won't rewrite the encoding with any new encoding value - we'll just leave it the same.
      // The default operation (and we will test this elsewhere) will be to write the current encoding into the encoding="" statement.
      typedef typename _TyXmlWriter::_TyXMLDeclProperties _TyXMLDeclProperties;
      _TyXMLDeclProperties xdpTranslated;
      _rxrc.GetXMLDeclProperties( xdpTranslated );
      
      // Without an output format (and no whitespace token filter, etc) we expect the output to exactly match the input except
      //  for encoding of course. This is the nature of the current unit test.
      xwWriter.OpenFile( _pathOutputPath.string().c_str(), xdpTranslated, nullptr, true ); // true indicates "keep encoding present in XMLDecl".
      size_t nTagsWritten = xwWriter.NWriteFromReadCursor( _rxrc );
      Assert( 1 == nTagsWritten ); // since we started in the prologue.
      VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );
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
    typedef typename _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
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
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.template OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.template OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.template OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.template OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.template OpenFileVar< t_tempTransport >( m_citTestFile->second.c_str() );
          _WriteCursorToFile< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
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
    typedef typename _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
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
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.OpenMemory( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        default:
          VerifyThrow( false );
        break;
      }
    }
  }
  template < class t_TyXmlWriteTransport >
  void _WriteCursorToMemory( typename _TyXmlParser::_TyReadCursorVar & _rxrc, filesystem::path _pathOutputPath, const _TyMapTestFiles::value_type * _pvtGoldenFile )
  {
    // We'll write the memstream to the file so that we test that part of the MemStream impl as well and then we can have a look
    //  at whatever might be wrong also.
    typedef xml_writer< t_TyXmlWriteTransport > _TyXmlWriter;
    _rxrc.AssertValid();
    {//B
      _TyXmlWriter xwWriter;
      xwWriter.SetWriteBOM( _pvtGoldenFile->first.second );
      xwWriter.SetWriteXMLDecl( !_rxrc.FPseudoXMLDecl() ); // Only write the XMLDecl if we got one in the source.
      // For this test we won't rewrite the encoding with any new encoding value - we'll just leave it the same.
      // The default operation (and we will test this elsewhere) will be to write the current encoding into the encoding="" statement.
      typedef typename _TyXmlWriter::_TyXMLDeclProperties _TyXMLDeclProperties;
      _TyXMLDeclProperties xdpTranslated;
      _rxrc.GetXMLDeclProperties( xdpTranslated );
      
      // Without an output format (and no whitespace token filter, etc) we expect the output to exactly match the input except
      //  for encoding of course. This is the nature of the current unit test.
      xwWriter.OpenMemFile( xdpTranslated, nullptr, true ); // true indicates "keep encoding present in XMLDecl".
      size_t nTagsWritten = xwWriter.NWriteFromReadCursor( _rxrc );
      Assert( 1 == nTagsWritten ); // since we started in the prologue.
      VerifyThrowSz( !m_fExpectFailure, "We expected to fail but we succeeded. No bueno." );

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
    typedef typename _TyXmlParser::_TyReadCursorVar _TyReadCursorVar;
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
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.template OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.template OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF16LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char16_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.template OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32BE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsLittleEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.template OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
        }
        break;
        case efceUTF32LE:
        {
          typedef typename map_input_to_any_output_transport< _TyTransport, char32_t, integral_constant< bool, vkfIsBigEndian > >::_TyXmlWriteTransport _TyXmlWriteTransport;
          _TyXmlParser xmlParser;
          _TyReadCursorVar xrcReadCursor = xmlParser.template OpenMemoryVar< t_tempTransport >( fmo.Pv(), nbySizeBytes );
          _WriteCursorToMemory< _TyXmlWriteTransport >( xrcReadCursor, pathOutput, pvtGoldenFile );
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

typedef XmlpTestVariantParser<_l_transport_mapped> vTyTestVarCursorMappedTransport;
TEST_P( vTyTestVarCursorMappedTransport, TestVarCursorMappedTransport )
{
  TestParserFile();
}
typedef XmlpTestVariantParser<_l_transport_file> vTyTestVarCursorFileTransport;
TEST_P( vTyTestVarCursorFileTransport, TestVarCursorFileTransport )
{
  TestParserFile();
}
typedef XmlpTestVariantParser<_l_transport_fixedmem> vTyTestVarCursorMemoryTransport;
TEST_P( vTyTestVarCursorMemoryTransport, TestVarCursorMemoryTransport )
{
  TestParserMemory();
}
// Give us some tests.
INSTANTIATE_TEST_SUITE_P( TestVarCursor, vTyTestVarCursorMappedTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
                          //Combine( Values(true), Values( int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestVarCursor, vTyTestVarCursorFileTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );
INSTANTIATE_TEST_SUITE_P( TestVarCursor, vTyTestVarCursorMemoryTransport,
                          Combine( Bool(), Values( int(efceUTF8), int(efceUTF16BE), int(efceUTF16LE), int(efceUTF32BE), int(efceUTF32LE) ) ) );

// Test variant transport with the variant parser.
typedef XmlpTestVariantParser< xml_var_get_var_transport_t, tuple< tuple< char8_t >, tuple< char16_t >, tuple< char32_t > > > vTyTestVarCursorVarTransport;
TEST_P( vTyTestVarCursorVarTransport, TestVarCursorVarTransport )
{
  TestParserFileTransportVar<_l_transport_mapped>();
  TestParserFileTransportVar<_l_transport_file>();
  TestParserMemoryTransportVar<_l_transport_fixedmem>();
}
INSTANTIATE_TEST_SUITE_P( TestVarCursor, vTyTestVarCursorVarTransport,
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
