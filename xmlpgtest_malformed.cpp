// xmlpgtest_malformed.cpp
// Google test stuff for XML Parser.
// Malformed-input testing of the lexical analyzer's triggers.
//
// The problem:
//  As the lexical analyzer scans a token its triggers fire and record data: positions (e.g. _l_trigger_position) and
//  ranges appended to string stores (e.g. _l_trigger_string). That data is consumed and cleared when a token is accepted.
//  On malformed input a scan can fire triggers and then fail. If the data those triggers recorded is not cleared then it is
//  still there for the next scan - and in a release build it can end up in a later token: a bogus translation.
//  The XML parser does scan the same input twice: it first tries the optional XMLDecl from its own start state and when that
//  fails it scans again from the start state for everything (xml_read_cursor). So <?xml version="x.0"?> fired the
//  VersionNum triggers, failed, and the second scan started with their data still present.
//
// The solution (in lexang _l_lxobj.h):
//  FGetToken() and FGetTokens() call ClearTokenData() when no accepting state is found.
//
// How this tests it:
//  LEXOBJ_STRICT is defined below - even in release builds - so that the lexical analyzer verifies that all trigger and token
//  data is clear at the start of each FGetToken() and after each token: a failure throws "Token id[...] still has data in it".
//  Each seed document covers a set of the XML grammar's triggers. Every seed is truncated at every offset, and at every offset
//  each of a set of significant characters is both inserted and substituted. Each variant is parsed into a document in UTF-8,
//  UTF-16 and UTF-32. A variant may parse or may fail with a parse error - either is fine - but it must never fail the
//  strict "still has data" check.
// dbien
// 17NOV2026

#define LEXOBJ_STRICT // Must precede the lexang includes - see above.

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

#include <string>
#include <vector>
#include <cstring>
#include <filesystem>
#include "syslogmgr.inl"
#include "_compat.inl"
#include "xml_inc.h"
#include "xml_tag.h"
#include "xml_tag_var.h"
#include "gtest/gtest.h"

__BIENUTIL_USING_NAMESPACE
std::string g_strProgramName;

namespace ns_XMLGTestMalformed
{
__XMLP_USING_NAMESPACE
__XMLPLEX_USING_NAMESPACE

// Each seed is well-formed and exercises the triggers named in its comment.
static const char * s_rgszSeeds[] =
{
  // XMLDecl: VersionNum, EncodingName, Standalone, double quotes.
  "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"yes\"?><a/>",
  // XMLDecl: the same with single quotes.
  "<?xml version='1.0' encoding='UTF-8' standalone='no'?><a/>",
  // PI: PITargetStart/End, PITargetMeatBegin.
  "<?xml-stylesheet type=\"text/xsl\" href=\"s.xsl\"?><a><?pi meat?></a>",
  // Comment: CommentBegin/End.
  "<!-- comment --><a/><!-- c2 -->",
  // Note: there are no DOCTYPE seeds: xmlpgen doesn't add the DOCTYPE productions to any DFA, so the SystemLiteral,
  //  PubidLiteral and PEReference triggers can't fire. Add seeds for them if that changes.
  // Tags and attributes: Prefix, LocalPart, PrefixedAttName, DefaultAttName, AttValue quotes.
  "<p:a xmlns:p=\"urn:p\" p:b=\"x\" c='y'><p:c/></p:a>",
  // References within attribute values: EntityRef, CharDecRef, CharHexRef, CharDataSingleQuote.
  "<a b=\"t&amp;u&#65;v&#x42;w\" c='t&lt;u&#66;v&#x43;w'>x</a>",
  // References within character data: CharData, EntityRef, CharDecRef, CharHexRef.
  "<a>abc&amp;def&#65;ghi&#x41;jkl</a>",
  // CDATA section.
  "<a><![CDATA[x]]y]]></a>",
  // Mixed content, end tags.
  "<a>t<?pi meat?>u<b>v</b>w</a>",
  // Reserved attributes and whitespace character data.
  "<a xml:lang=\"en\" xmlns=\"urn:d\">text </a>",
};
static const size_t s_knSeeds = sizeof( s_rgszSeeds ) / sizeof( s_rgszSeeds[0] );

// Characters that are significant to the grammar - plus space and a name character.
static const char s_rgcCorrupt[] = { '<', '>', '&', '#', ';', '"', '\'', '=', ']', '[', '?', '!', '-', ':', '%', ' ', 'x' };

enum EMalformedEncoding { emeUTF8, emeUTF16, emeUTF32, emeEncodingCount };

typedef xml_parser_var< xml_var_get_var_transport_t, tuple< tuple< char8_t >, tuple< char16_t >, tuple< char32_t > > > _TyXmlParser;
typedef _TyXmlParser::_TyReadCursorVar _TyXmlReadCursor;
typedef xml_document_var< _TyXmlParser::_TyTpTransports > _TyXmlDocument;

// Encode the (ASCII) variant with a little endian BOM for UTF-16 and UTF-32.
static std::string StrEncode( std::string const & _str, EMalformedEncoding _eme )
{
  if ( emeUTF8 == _eme )
    return _str;
  const size_t knBytes = ( emeUTF16 == _eme ) ? 2 : 4;
  std::string strEnc;
  strEnc.reserve( ( _str.size() + 1 ) * knBytes );
  strEnc.push_back( '\xFF' );
  strEnc.push_back( '\xFE' );
  if ( 4 == knBytes )
    strEnc.append( 2, '\0' );
  for ( char c : _str )
  {
    strEnc.push_back( c );
    strEnc.append( knBytes - 1, '\0' );
  }
  return strEnc;
}

// A document is to be in the encoding which it declares ( 4.3.3 ): the seed declares the one which it is encoded in.
static std::string StrDeclaringEncoding( std::string _str, EMalformedEncoding _eme )
{
  const char * const kpszName = ( emeUTF8 == _eme ) ? "UTF-8" : ( ( emeUTF16 == _eme ) ? "UTF-16" : "UTF-32" );
  for ( const char * pszDeclared : { "encoding=\"UTF-8\"", "encoding='UTF-8'" } )
  {
    const size_t knPos = _str.find( pszDeclared );
    if ( std::string::npos != knPos )
      _str.replace( knPos + 10, 5, kpszName ); // After "encoding=" and its quote.
  }
  return _str;
}

enum EParseResult { eprParsed, eprParseError, eprTriggerDataLeft };

// Parse the variant into a document. Return the result and the exception message if any.
static EParseResult EprParse( std::string const & _str, EMalformedEncoding _eme, std::string & _rstrWhat )
{
  const std::string strEnc = StrEncode( _str, _eme );
  try
  {
    _TyXmlParser xmlParser;
    _TyXmlReadCursor xmlReadCursor = xmlParser.OpenMemoryVar( strEnc.data(), strEnc.size() );
    auto prXmlDoc = _TyXmlDocument::PrCreateXmlDocument();
    prXmlDoc.first->FromXmlStream( xmlReadCursor, prXmlDoc.second );
    return eprParsed;
  }
  catch ( std::exception const & rexc )
  {
    _rstrWhat = rexc.what();
    // Parse errors are VerifyFailedExceptions as well - the strict check is identified by its message.
    return std::string::npos != _rstrWhat.find( "still has data" ) ? eprTriggerDataLeft : eprParseError;
  }
}

// Every variant of a seed: truncations at each offset and an insertion and a substitution of each corrupting character at each offset.
static std::vector< std::string > RgstrVariants( std::string const & _strSeed )
{
  std::vector< std::string > rgstr;
  for ( size_t n = 0; n < _strSeed.size(); ++n )
  {
    rgstr.push_back( _strSeed.substr( 0, n ) );
    for ( char c : s_rgcCorrupt )
    {
      std::string str( _strSeed );
      str.insert( n, 1, c );
      rgstr.push_back( std::move( str ) );
      if ( _strSeed[n] != c )
      {
        str = _strSeed;
        str[n] = c;
        rgstr.push_back( std::move( str ) );
      }
    }
  }
  return rgstr;
}

class XmlpTestMalformed : public testing::TestWithParam< std::tuple< size_t, int > >
{
};

TEST_P( XmlpTestMalformed, NoTriggerDataLeft )
{
  const size_t knSeed = std::get< 0 >( GetParam() );
  const EMalformedEncoding keme = (EMalformedEncoding)std::get< 1 >( GetParam() );
  const std::string strSeed = StrDeclaringEncoding( s_rgszSeeds[ knSeed ], keme );
  std::string strWhat;
  // The seed itself must parse - otherwise its variants aren't testing what we think they are.
  ASSERT_EQ( eprParsed, EprParse( strSeed, keme, strWhat ) ) << "Seed [" << strSeed << "]: " << strWhat;
  size_t nVariants = 0, nLeft = 0;
  for ( std::string const & strVariant : RgstrVariants( strSeed ) )
  {
    ++nVariants;
    if ( eprTriggerDataLeft == EprParse( strVariant, keme, strWhat ) )
    {
      ++nLeft;
      if ( nLeft <= 5 ) // Enough to diagnose without flooding the output.
        ADD_FAILURE() << "Trigger data left by variant [" << strVariant << "]: " << strWhat;
    }
  }
  EXPECT_EQ( 0u, nLeft ) << nLeft << " of " << nVariants << " variants of seed [" << strSeed << "] left trigger data.";
}

INSTANTIATE_TEST_SUITE_P( Malformed, XmlpTestMalformed,
  testing::Combine( testing::Range( size_t( 0 ), s_knSeeds ), testing::Range( int( emeUTF8 ), int( emeEncodingCount ) ) ) );

} // namespace ns_XMLGTestMalformed

int main( int argc, char ** argv )
{
  g_strProgramName = *argv;
#ifdef WIN32
  // Sometimes in windows we get called from cmake and the path is using Linux path separators:
  g_strProgramName = std::filesystem::path( g_strProgramName ).lexically_normal().string();
#endif //WIN32
  // The malformed variants log tens of thousands of expected parse errors: don't echo them to stderr (no LOG_PERROR)
  //  and don't write them to a JSON log file (that would be ~75MB per run).
  n_SysLog::SetGenerateUniqueJSONLogFile( false );
  n_SysLog::InitSysLog( g_strProgramName.c_str(), 0, LOG_USER );
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}
