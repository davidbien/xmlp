// xmlpgtest_dtd.cpp
// Google test stuff for XML Parser.
// The DTD: the tokens of the DTD DFAs of the lexical analyzers and the parser generated for the DTD ( xmlpgen_dtd.cpp ).
// The text of a document type declaration, an external subset or the replacement text of a parameter entity is parsed
//  in UTF-8, UTF-16 and UTF-32 - the one parser with each of the three lexical analyzers - into the parse tree: that
//  tests the tokens and the grammar. And it is read into an xml_dtd by xml_dtd_parser ( xml_dtd.h ): that tests what is
//  made of the declarations. And a document with a document type declaration is read by the cursor ( xml_cursor.h ) when
//  the XML traits support the DTD: the cursor then has the xml_dtd and the entities of the document are those declared.
// And the W3C XML Conformance Test Suite ( unittests/xmlconf ): the document type declarations of its documents and its
//  external subsets are parsed - see DtdConformance below for what is expected of them.
// LEXOBJ_STRICT is defined so that the lexical analyzer verifies that no trigger data is left behind by a token or by a
//  DFA which found no token - the parser tries more than one DFA for some tokens. See xmlpgtest_malformed.cpp.
// 19NOV2026

#define LEXOBJ_STRICT // Must precede the lexang includes.

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
#include <algorithm>
#include <filesystem>
#include <fstream>
#include <map>
#include <optional>
#include <set>
#include <sstream>
#include "syslogmgr.inl"
#include "_compat.inl"
#include "xml_inc.h"
#include "xml_inc.h"
#include "xml_tag.h"
#include "xml_tag_var.h"
#include "xml_transport.h"
#include "xml_writer.h"
#include "xml_compare.h"
#include "xml_dtd.h"
#include "gtest/gtest.h"

__BIENUTIL_USING_NAMESPACE
std::string g_strProgramName;

namespace ns_XMLGTestDtd
{
__XMLP_USING_NAMESPACE
__XMLPLEX_USING_NAMESPACE
USE_PGENOBJ_NAMESPACE

// The start states of the DTD DFAs of the lexical analyzer of a character type - in the order of the modes of the parser.
template < class t_TyChar >
struct DtdStartStates;
template <>
struct DtdStartStates< char8_t >
{
  template < class t_TyLexTraits, class t_TyStateProto >
  static std::initializer_list< const t_TyStateProto * > Get()
  {
    static const std::initializer_list< const t_TyStateProto * > s_il = {
      (const t_TyStateProto *)&startUTF8DtdTextDecl< t_TyLexTraits >, (const t_TyStateProto *)&startUTF8DtdKeyword< t_TyLexTraits >,
      (const t_TyStateProto *)&startUTF8DtdNmtoken< t_TyLexTraits >, (const t_TyStateProto *)&startUTF8DtdEntityValue< t_TyLexTraits >,
      (const t_TyStateProto *)&startUTF8DtdAttValue< t_TyLexTraits >, (const t_TyStateProto *)&startUTF8DtdSystemLiteral< t_TyLexTraits >,
      (const t_TyStateProto *)&startUTF8DtdPubidLiteral< t_TyLexTraits >, (const t_TyStateProto *)&startUTF8Dtd< t_TyLexTraits >,
      (const t_TyStateProto *)&startUTF8DtdIgnore< t_TyLexTraits > };
    return s_il;
  }
  template < class t_TyLexTraits, class t_TyStateProto >
  static const t_TyStateProto * GetAll() { return (const t_TyStateProto *)&startUTF8All< t_TyLexTraits >; }
};
template <>
struct DtdStartStates< char16_t >
{
  template < class t_TyLexTraits, class t_TyStateProto >
  static std::initializer_list< const t_TyStateProto * > Get()
  {
    static const std::initializer_list< const t_TyStateProto * > s_il = {
      (const t_TyStateProto *)&startUTF16DtdTextDecl< t_TyLexTraits >, (const t_TyStateProto *)&startUTF16DtdKeyword< t_TyLexTraits >,
      (const t_TyStateProto *)&startUTF16DtdNmtoken< t_TyLexTraits >, (const t_TyStateProto *)&startUTF16DtdEntityValue< t_TyLexTraits >,
      (const t_TyStateProto *)&startUTF16DtdAttValue< t_TyLexTraits >, (const t_TyStateProto *)&startUTF16DtdSystemLiteral< t_TyLexTraits >,
      (const t_TyStateProto *)&startUTF16DtdPubidLiteral< t_TyLexTraits >, (const t_TyStateProto *)&startUTF16Dtd< t_TyLexTraits >,
      (const t_TyStateProto *)&startUTF16DtdIgnore< t_TyLexTraits > };
    return s_il;
  }
  template < class t_TyLexTraits, class t_TyStateProto >
  static const t_TyStateProto * GetAll() { return (const t_TyStateProto *)&startUTF16All< t_TyLexTraits >; }
};
template <>
struct DtdStartStates< char32_t >
{
  template < class t_TyLexTraits, class t_TyStateProto >
  static std::initializer_list< const t_TyStateProto * > Get()
  {
    static const std::initializer_list< const t_TyStateProto * > s_il = {
      (const t_TyStateProto *)&startUTF32DtdTextDecl< t_TyLexTraits >, (const t_TyStateProto *)&startUTF32DtdKeyword< t_TyLexTraits >,
      (const t_TyStateProto *)&startUTF32DtdNmtoken< t_TyLexTraits >, (const t_TyStateProto *)&startUTF32DtdEntityValue< t_TyLexTraits >,
      (const t_TyStateProto *)&startUTF32DtdAttValue< t_TyLexTraits >, (const t_TyStateProto *)&startUTF32DtdSystemLiteral< t_TyLexTraits >,
      (const t_TyStateProto *)&startUTF32DtdPubidLiteral< t_TyLexTraits >, (const t_TyStateProto *)&startUTF32Dtd< t_TyLexTraits >,
      (const t_TyStateProto *)&startUTF32DtdIgnore< t_TyLexTraits > };
    return s_il;
  }
  template < class t_TyLexTraits, class t_TyStateProto >
  static const t_TyStateProto * GetAll() { return (const t_TyStateProto *)&startUTF32All< t_TyLexTraits >; }
};

// The result of parsing a text of the DTD.
struct DtdParseResult
{
  bool m_fAccepted{false};
  std::string m_strError; // The syntax error of the parser or the exception of the lexical analyzer.
  std::string m_strTree; // The parse tree: "(non-terminal children...)" - a token is its text, white space is "_".
  std::vector< std::string > m_rgstrNames; // The text of each Name token.
  size_t m_nTokens{0};
  vtyTokenIdent m_tidNext{0}; // The token of the document which follows what was parsed - if asked for.
};

template < class t_TyChar >
struct DtdTester
{
  typedef t_TyChar _TyChar;
  typedef _l_transport_fixedmem< _TyChar > _TyTransport;
  typedef xml_traits< _TyTransport, false, false > _TyXmlTraits;
  typedef typename _TyXmlTraits::_TyLexTraits _TyLexTraits;
  typedef xml_parser< _TyXmlTraits > _TyXmlParser;
  typedef typename _TyXmlParser::_TyLexicalAnalyzer _TyAnalyzer;
  typedef typename _TyAnalyzer::_TyToken _TyLexToken;
  typedef typename _TyAnalyzer::_TyStateProto _TyStateProto;
  typedef std::unique_ptr< _TyLexToken > _TyToken;
  struct _GetTokenId
  {
    static vtyTokenIdent GetTokenId( _TyToken const & _rupToken ) { return _rupToken->GetTokenId(); }
  };
  typedef ParseTreeTraits< _TyToken, vtyTokenIdent, _GetTokenId > _TyParseTraits;
  typedef ParseTreeNode< _TyToken > _TyNode;

  static std::string StrTokenText( _TyLexToken const & _rtok )
  {
    _l_data_range dr;
    _rtok.GetTokenDataRange( dr );
    std::basic_string_view< _TyChar > sv;
    _rtok.KGetStringView( sv, dr );
    std::string str;
    for ( _TyChar ch : sv )
      str += ( ( ch >= 0x20 ) && ( ch < 0x7f ) ) ? char( ch ) : '?';
    return str;
  }
  static void Walk( _TyNode const & _rnode, DtdParseResult & _rresult )
  {
    if ( _rnode.FIsToken() )
    {
      ++_rresult.m_nTokens;
      const vtyTokenIdent tid = (*_rnode.m_optToken)->GetTokenId();
      if ( s_knTokenDtdS == tid )
        _rresult.m_strTree += "_";
      else
        _rresult.m_strTree += StrTokenText( **_rnode.m_optToken );
      if ( s_knTokenDtdName == tid )
        _rresult.m_rgstrNames.push_back( StrTokenText( **_rnode.m_optToken ) );
      return;
    }
    ParseProductionInfo const & rpi = ns_xmlpdtd::s_viewParseTable.GetProduction( _rnode.m_nProduction );
    // The repetitions, options and groups of the grammar are not shown - only what the specification names.
    const bool fShow = ( EParseProductionKind::eppkUser == rpi.m_eKind );
    if ( fShow )
      _rresult.m_strTree += std::string( "(" ) + ns_xmlpdtd::s_viewParseTable.PszNonTerminalName( rpi.m_nNonTerminal );
    for ( _TyNode const & rnodeChild : _rnode.m_rgChildren )
    {
      if ( _rresult.m_strTree.empty() || ( _rresult.m_strTree.back() != ' ' ) )
        _rresult.m_strTree += " ";
      Walk( rnodeChild, _rresult );
    }
    while ( !_rresult.m_strTree.empty() && ( _rresult.m_strTree.back() == ' ' ) )
      _rresult.m_strTree.pop_back();
    if ( fShow )
      _rresult.m_strTree += ")";
  }
  // Parse the text from the entry. _fGetNext: get the next token of the document after what was parsed.
  static DtdParseResult Parse( const char * _pszText, uint32_t _nEntry, bool _fGetNext = false )
  {
    const std::basic_string< _TyChar > strText( _pszText, _pszText + strlen( _pszText ) );
    return ParseText( strText, _nEntry, _fGetNext );
  }
  static DtdParseResult ParseText( std::basic_string_view< _TyChar > _svText, uint32_t _nEntry, bool _fGetNext = false )
  {
    DtdParseResult result;
    try
    {
      _TyXmlParser xmlParser;
      xmlParser.emplaceTransport( _svText.data(), _svText.length() );
      _TyAnalyzer & rAnalyzer = xmlParser.GetLexicalAnalyzer();
      _p_lexang_token_source< _TyAnalyzer > source( rAnalyzer );
      source.SetModeStartStates( DtdStartStates< _TyChar >::template Get< _TyLexTraits, _TyStateProto >() );
      ns_xmlpdtd::_parser< _TyParseTraits > parser;
      ParseTreeHandler< _TyToken > handler;
      _TyNode nodeRoot;
      result.m_fAccepted = parser.FParse( source, handler, nodeRoot, _nEntry );
      if ( !result.m_fAccepted )
        result.m_strError = parser.StrError();
      else
      {
        Walk( nodeRoot, result );
        if ( _fGetNext )
        {
          _TyToken upNext;
          if ( rAnalyzer.FGetToken( upNext, nullptr, nullptr, DtdStartStates< _TyChar >::template GetAll< _TyLexTraits, _TyStateProto >(), false ) )
            result.m_tidNext = upNext->GetTokenId();
        }
      }
    }
    catch ( std::exception const & rexc )
    {
      result.m_fAccepted = false;
      result.m_strError = rexc.what();
    }
    return result;
  }
};

template < class t_TyChar >
class DtdTest : public testing::Test
{
};
typedef testing::Types< char8_t, char16_t, char32_t > TyDtdCharTypes;
struct DtdCharTypeNames
{
  template < class t_TyChar >
  static std::string GetName( int )
  {
    return std::is_same_v< t_TyChar, char8_t > ? "UTF8" : ( std::is_same_v< t_TyChar, char16_t > ? "UTF16" : "UTF32" );
  }
};
TYPED_TEST_SUITE( DtdTest, TyDtdCharTypes, DtdCharTypeNames );

using ns_xmlpdtd::s_knEntry_doctypedecl;
using ns_xmlpdtd::s_knEntry_extSubset;
using ns_xmlpdtd::s_knEntry_extSubsetDecl;

// The document type declaration without an internal subset - [28], [75].
TYPED_TEST( DtdTest, DoctypeDecl )
{
  typedef DtdTester< TypeParam > TyTester;
  auto lambdaTree = []( const char * _psz ) -> std::string
  {
    const DtdParseResult result = TyTester::Parse( _psz, s_knEntry_doctypedecl );
    return result.m_fAccepted ? result.m_strTree : ( "ERROR: " + result.m_strError );
  };
  EXPECT_EQ( lambdaTree( "<!DOCTYPE a>" ), "(doctypedecl <!DOCTYPE _ a >)" );
  EXPECT_EQ( lambdaTree( "<!DOCTYPE a >" ), "(doctypedecl <!DOCTYPE _ a _ >)" );
  EXPECT_EQ( lambdaTree( "<!DOCTYPE a SYSTEM \"a.dtd\">" ), "(doctypedecl <!DOCTYPE _ a _ (ExternalID SYSTEM _ \"a.dtd\") >)" );
  EXPECT_EQ( lambdaTree( "<!DOCTYPE a\n PUBLIC '-//X//DTD Y//EN'\t\"http://x/y.dtd\" >" ),
    "(doctypedecl <!DOCTYPE _ a _ (ExternalID (PublicID PUBLIC _ '-//X//DTD Y//EN') _ \"http://x/y.dtd\") _ >)" );
  EXPECT_EQ( lambdaTree( "<!DOCTYPE a[]>" ), "(doctypedecl <!DOCTYPE _ a [ (intSubset) ] >)" );
  EXPECT_EQ( lambdaTree( "<!DOCTYPE a SYSTEM 'x' [ ] >" ), "(doctypedecl <!DOCTYPE _ a _ (ExternalID SYSTEM _ 'x') _ [ (intSubset (DeclSep _)) ] _ >)" );
  // A name with a colon and the other name characters.
  EXPECT_EQ( lambdaTree( "<!DOCTYPE x:a-b.c_9>" ), "(doctypedecl <!DOCTYPE _ x:a-b.c_9 >)" );
  for ( const char * pszBad : { "<!DOCTYPE>", "<!DOCTYPEa>", "<!DOCTYPE a b>", "<!DOCTYPE a SYSTEM>", "<!DOCTYPE a SYSTEM\"x\">", "<!DOCTYPE a PUBLIC \"x\">",
                                "<!DOCTYPE a PUBLIC \"{\" 'x'>", "<!DOCTYPE a [", "<!DOCTYPE a []", "<!DOCTYPE 1a>", "<!doctype a>", "" } )
    EXPECT_FALSE( TyTester::Parse( pszBad, s_knEntry_doctypedecl ).m_fAccepted ) << pszBad;
}

// The parse ends at the '>' of the declaration: the document which follows is the cursor's to read.
TYPED_TEST( DtdTest, StopsAtTheEnd )
{
  typedef DtdTester< TypeParam > TyTester;
  {
    const DtdParseResult result = TyTester::Parse( "<!DOCTYPE a [<!ELEMENT a EMPTY>]><a/>", s_knEntry_doctypedecl, true );
    ASSERT_TRUE( result.m_fAccepted ) << result.m_strError;
    EXPECT_EQ( result.m_tidNext, s_knTokenEmptyElemTag );
  }
  {
    const DtdParseResult result = TyTester::Parse( "<!DOCTYPE a><!-- c --><a/>", s_knEntry_doctypedecl, true );
    ASSERT_TRUE( result.m_fAccepted ) << result.m_strError;
    EXPECT_EQ( result.m_tidNext, s_knTokenComment );
  }
  {
    const DtdParseResult result = TyTester::Parse( "<!DOCTYPE a SYSTEM 'x'>text", s_knEntry_doctypedecl, true );
    ASSERT_TRUE( result.m_fAccepted ) << result.m_strError;
    EXPECT_EQ( result.m_tidNext, s_knTokenCharData );
  }
}

// The declarations of the internal subset - [28b], [29], [45] through [60], [70] through [83].
TYPED_TEST( DtdTest, InternalSubset )
{
  typedef DtdTester< TypeParam > TyTester;
  auto lambdaDecl = []( const char * _pszDecl ) -> std::string
  {
    const std::string strText = std::string( "<!DOCTYPE d[" ) + _pszDecl + "]>";
    const DtdParseResult result = TyTester::Parse( strText.c_str(), s_knEntry_doctypedecl );
    if ( !result.m_fAccepted )
      return "ERROR: " + result.m_strError;
    // Just the declaration: what is within "(doctypedecl <!DOCTYPE _ d [ (intSubset " and ") ] >)".
    const std::string strBegin = "(doctypedecl <!DOCTYPE _ d [ (intSubset ", strEnd = ") ] >)";
    if ( ( result.m_strTree.size() < strBegin.size() + strEnd.size() ) || result.m_strTree.compare( 0, strBegin.size(), strBegin ) )
      return result.m_strTree;
    return result.m_strTree.substr( strBegin.size(), result.m_strTree.size() - strBegin.size() - strEnd.size() );
  };
  // Elements.
  EXPECT_EQ( lambdaDecl( "<!ELEMENT a EMPTY>" ), "(markupdecl (elementdecl <!ELEMENT _ a _ (contentspec EMPTY) >))" );
  EXPECT_EQ( lambdaDecl( "<!ELEMENT a ANY >" ), "(markupdecl (elementdecl <!ELEMENT _ a _ (contentspec ANY) _ >))" );
  EXPECT_EQ( lambdaDecl( "<!ELEMENT a (#PCDATA)>" ), "(markupdecl (elementdecl <!ELEMENT _ a _ (contentspec (Mixed ( #PCDATA ))) >))" );
  EXPECT_EQ( lambdaDecl( "<!ELEMENT a (#PCDATA)*>" ), "(markupdecl (elementdecl <!ELEMENT _ a _ (contentspec (Mixed ( #PCDATA ) *)) >))" );
  EXPECT_EQ( lambdaDecl( "<!ELEMENT a ( #PCDATA | b|c )*>" ), "(markupdecl (elementdecl <!ELEMENT _ a _ (contentspec (Mixed ( _ #PCDATA _ | _ b | c _ ) *)) >))" );
  EXPECT_EQ( lambdaDecl( "<!ELEMENT a (b)>" ), "(markupdecl (elementdecl <!ELEMENT _ a _ (contentspec (children (seq ( (cp b) )))) >))" );
  EXPECT_EQ( lambdaDecl( "<!ELEMENT a (b,c)>" ), "(markupdecl (elementdecl <!ELEMENT _ a _ (contentspec (children (seq ( (cp b) , (cp c) )))) >))" );
  EXPECT_EQ( lambdaDecl( "<!ELEMENT a (b|c)+>" ), "(markupdecl (elementdecl <!ELEMENT _ a _ (contentspec (children (choice ( (cp b) | (cp c) )) +)) >))" );
  EXPECT_EQ( lambdaDecl( "<!ELEMENT a ( b? , (c | d)* , e+ )?>" ),
    "(markupdecl (elementdecl <!ELEMENT _ a _ (contentspec (children (seq ( _ (cp b ?) _ , _ (cp (choice ( (cp c) _ | _ (cp d) )) *) _ , _ (cp e +) _ )) ?)) >))" );
  // Attribute lists.
  EXPECT_EQ( lambdaDecl( "<!ATTLIST a>" ), "(markupdecl (AttlistDecl <!ATTLIST _ a >))" );
  EXPECT_EQ( lambdaDecl( "<!ATTLIST a x CDATA #IMPLIED>" ), "(markupdecl (AttlistDecl <!ATTLIST _ a (AttDef _ x _ (AttType (StringType CDATA)) _ (DefaultDecl #IMPLIED)) >))" );
  EXPECT_EQ( lambdaDecl( "<!ATTLIST a x ID #REQUIRED y IDREFS \"v\" >" ),
    "(markupdecl (AttlistDecl <!ATTLIST _ a (AttDef _ x _ (AttType (TokenizedType ID)) _ (DefaultDecl #REQUIRED)) (AttDef _ y _ (AttType (TokenizedType IDREFS)) _ (DefaultDecl \"v\")) _ >))" );
  EXPECT_EQ( lambdaDecl( "<!ATTLIST a x (one| 2 |-three) #FIXED 'one'>" ),
    "(markupdecl (AttlistDecl <!ATTLIST _ a (AttDef _ x _ (AttType (EnumeratedType (Enumeration ( one | _ 2 _ | -three )))) _ (DefaultDecl #FIXED _ 'one')) >))" );
  EXPECT_EQ( lambdaDecl( "<!ATTLIST a x NOTATION (gif|jpg) 'a&amp;b&#65;'>" ),
    "(markupdecl (AttlistDecl <!ATTLIST _ a (AttDef _ x _ (AttType (EnumeratedType (NotationType NOTATION _ ( gif | jpg )))) _ (DefaultDecl 'a&amp;b&#65;')) >))" );
  for ( const char * pszType : { "IDREF", "ENTITY", "ENTITIES", "NMTOKEN", "NMTOKENS" } )
    EXPECT_EQ( lambdaDecl( ( std::string( "<!ATTLIST a x " ) + pszType + " #IMPLIED>" ).c_str() ),
      std::string( "(markupdecl (AttlistDecl <!ATTLIST _ a (AttDef _ x _ (AttType (TokenizedType " ) + pszType + ")) _ (DefaultDecl #IMPLIED)) >))" );
  // Entities.
  EXPECT_EQ( lambdaDecl( "<!ENTITY e \"v\">" ), "(markupdecl (EntityDecl (GEDecl <!ENTITY _ e _ (EntityDef \"v\") >)))" );
  EXPECT_EQ( lambdaDecl( "<!ENTITY e 'a %p; &b; &#x41; \"c\"' >" ), "(markupdecl (EntityDecl (GEDecl <!ENTITY _ e _ (EntityDef 'a %p; &b; &#x41; \"c\"' _) >)))" );
  EXPECT_EQ( lambdaDecl( "<!ENTITY e SYSTEM 's'>" ), "(markupdecl (EntityDecl (GEDecl <!ENTITY _ e _ (EntityDef (ExternalID SYSTEM _ 's')) >)))" );
  EXPECT_EQ( lambdaDecl( "<!ENTITY e PUBLIC 'p' 's' NDATA n >" ),
    "(markupdecl (EntityDecl (GEDecl <!ENTITY _ e _ (EntityDef (ExternalID (PublicID PUBLIC _ 'p') _ 's') (NDataDecl _ NDATA _ n) _) >)))" );
  EXPECT_EQ( lambdaDecl( "<!ENTITY % p \"v\">" ), "(markupdecl (EntityDecl (PEDecl <!ENTITY _ % _ p _ (PEDef \"v\") >)))" );
  EXPECT_EQ( lambdaDecl( "<!ENTITY % p SYSTEM \"s\" >" ), "(markupdecl (EntityDecl (PEDecl <!ENTITY _ % _ p _ (PEDef (ExternalID SYSTEM _ \"s\")) _ >)))" );
  // Notations.
  EXPECT_EQ( lambdaDecl( "<!NOTATION n SYSTEM 's'>" ), "(markupdecl (NotationDecl <!NOTATION _ n _ (ExternalID SYSTEM _ 's') >))" );
  EXPECT_EQ( lambdaDecl( "<!NOTATION n PUBLIC 'p'>" ), "(markupdecl (NotationDecl <!NOTATION _ n _ (PublicID PUBLIC _ 'p') >))" );
  EXPECT_EQ( lambdaDecl( "<!NOTATION n PUBLIC 'p' >" ), "(markupdecl (NotationDecl <!NOTATION _ n _ (PublicID PUBLIC _ 'p') _ >))" );
  EXPECT_EQ( lambdaDecl( "<!NOTATION n PUBLIC 'p' 's'>" ), "(markupdecl (NotationDecl <!NOTATION _ n _ (ExternalID (PublicID PUBLIC _ 'p') _ 's') >))" );
  // What is between declarations, comments and processing instructions.
  EXPECT_EQ( lambdaDecl( "%p;" ), "(DeclSep %p;)" );
  EXPECT_EQ( lambdaDecl( "<!-- a - b -->" ), "(markupdecl <!-- a - b -->)" );
  EXPECT_EQ( lambdaDecl( "<?pi some stuff ?>" ), "(markupdecl <?pi some stuff ?>)" );
  EXPECT_EQ( lambdaDecl( " %p; <!ELEMENT a EMPTY>\n<?pi?>" ),
    "(DeclSep _) (DeclSep %p;) (DeclSep _) (markupdecl (elementdecl <!ELEMENT _ a _ (contentspec EMPTY) >)) (DeclSep _) (markupdecl <?pi?>)" );
  // Not declarations.
  for ( const char * pszBad : {
          "<!ELEMENT a>", "<!ELEMENTa EMPTY>", "<!ELEMENT a EMPTY b>", "<!ELEMENT a (b,c|d)>", "<!ELEMENT a (b|c,d)>", "<!ELEMENT a ()>", "<!ELEMENT a (b) *>",
          "<!ELEMENT a (b)**>", "<!ELEMENT a (#PCDATA|b)>", "<!ELEMENT a (#PCDATA,b)*>", "<!ELEMENT a (b|#PCDATA)*>", "<!ELEMENT a (b ?)>", "<!ELEMENT a empty>",
          "<!ATTLIST>", "<!ATTLIST a x>", "<!ATTLIST a x CDATA>", "<!ATTLIST a x CDATA#IMPLIED>", "<!ATTLIST a x cdata #IMPLIED>", "<!ATTLIST a x () #IMPLIED>",
          "<!ATTLIST a x (a,b) #IMPLIED>", "<!ATTLIST a x NOTATION #IMPLIED>", "<!ATTLIST a x NOTATION (1) #IMPLIED>", "<!ATTLIST a x CDATA #FIXED>",
          "<!ATTLIST a x CDATA #FIXED'v'>", "<!ATTLIST a x CDATA 'a<b'>", "<!ATTLIST a x CDATA 'a&b'>", "<!ATTLIST a x CDATA #REQUIRED 'v'>",
          "<!ENTITY e>", "<!ENTITY e 'a%b'>", "<!ENTITY e 'a&b'>", "<!ENTITY e \"a\"b\">", "<!ENTITY e SYSTEM 's' NDATA>", "<!ENTITY e SYSTEM 's'NDATA n>",
          "<!ENTITY %p 'v'>", "<!ENTITY % p 'v' NDATA n>", "<!ENTITY % p SYSTEM 's' NDATA n>", "<!ENTITY e PUBLIC 'p'>",
          "<!NOTATION n>", "<!NOTATION n 's'>", "<!NOTATION n SYSTEM>", "<!NOTATION n PUBLIC 'p''s'>",
          "<![INCLUDE[ ]]>", "<![IGNORE[ ]]>", "<a/>", "text", "&e;", "<!ELEMENT a EMPTY", "%p", "<!-- a -- b -->", "<?xml version='1.0'?>" } )
    EXPECT_EQ( lambdaDecl( pszBad ).substr( 0, 6 ), "ERROR:" ) << pszBad;
}

// A keyword is a name wherever a name is what the grammar has there: the parser chooses the DFA of the lexical analyzer.
TYPED_TEST( DtdTest, KeywordsAreNames )
{
  typedef DtdTester< TypeParam > TyTester;
  const DtdParseResult result = TyTester::Parse(
    "<!DOCTYPE EMPTY [\n"
    "<!ELEMENT ANY ( ID | CDATA|SYSTEM )>\n"
    "<!ELEMENT ID (#PCDATA|PUBLIC|NDATA|INCLUDE|IGNORE)*>\n"
    "<!ELEMENT ANYTHING EMPTY>\n"
    "<!ELEMENT EMPTY ANY>\n"
    "<!ATTLIST CDATA NMTOKEN NMTOKEN #IMPLIED ENTITY (ID|NMTOKEN|ENTITY|7up) 'ID' NOTATION NOTATION ( NOTATION | IDREFS ) #REQUIRED ENTITIES ENTITIES 'x'>\n"
    "<!ENTITY PUBLIC \"x\">\n"
    "<!ENTITY SYSTEM SYSTEM 's' NDATA NDATA>\n"
    "<!ENTITY % NDATA PUBLIC 'p' 's'>\n"
    "<!NOTATION PUBLIC PUBLIC 'p'>\n"
    "]>", s_knEntry_doctypedecl );
  ASSERT_TRUE( result.m_fAccepted ) << result.m_strError;
  EXPECT_EQ( result.m_rgstrNames, ( std::vector< std::string >{ "EMPTY", "ANY", "ID", "CDATA", "SYSTEM", "ID", "PUBLIC", "NDATA", "INCLUDE", "IGNORE", "ANYTHING", "EMPTY",
    "CDATA", "NMTOKEN", "ENTITY", "NOTATION", "NOTATION", "IDREFS", "ENTITIES", "PUBLIC", "SYSTEM", "NDATA", "NDATA", "PUBLIC" } ) );
  // And a name which begins with a keyword is not the keyword where the keyword is what the grammar has.
  EXPECT_FALSE( TyTester::Parse( "<!DOCTYPE a [<!ELEMENT a EMPTYX>]>", s_knEntry_doctypedecl ).m_fAccepted );
  EXPECT_FALSE( TyTester::Parse( "<!DOCTYPE a [<!ATTLIST a x IDX #IMPLIED>]>", s_knEntry_doctypedecl ).m_fAccepted );
  EXPECT_FALSE( TyTester::Parse( "<!DOCTYPE a SYSTEMX 's'>", s_knEntry_doctypedecl ).m_fAccepted );
}

// The external subset - [30], [31] - with its text declaration [77] and conditional sections [61] through [65].
TYPED_TEST( DtdTest, ExternalSubset )
{
  typedef DtdTester< TypeParam > TyTester;
  auto lambdaTree = []( const char * _psz, uint32_t _nEntry = s_knEntry_extSubset ) -> std::string
  {
    const DtdParseResult result = TyTester::Parse( _psz, _nEntry );
    return result.m_fAccepted ? result.m_strTree : ( "ERROR: " + result.m_strError );
  };
  EXPECT_EQ( lambdaTree( "" ), "(extSubset (extSubsetDecl))" );
  EXPECT_EQ( lambdaTree( "<!ELEMENT a EMPTY>" ), "(extSubset (extSubsetDecl (markupdecl (elementdecl <!ELEMENT _ a _ (contentspec EMPTY) >))))" );
  EXPECT_EQ( lambdaTree( "<?xml version=\"1.0\" encoding='UTF-8'?><!ELEMENT a EMPTY>" ),
    "(extSubset <?xml version=\"1.0\" encoding='UTF-8'?> (extSubsetDecl (markupdecl (elementdecl <!ELEMENT _ a _ (contentspec EMPTY) >))))" );
  EXPECT_EQ( lambdaTree( "<?xml encoding=\"x\" ?>" ), "(extSubset <?xml encoding=\"x\" ?> (extSubsetDecl))" );
  // Not a text declaration: a processing instruction - the DFA of the text declaration is tried first and then the other.
  EXPECT_EQ( lambdaTree( "<?xml-stylesheet href='x'?>" ), "(extSubset (extSubsetDecl (markupdecl <?xml-stylesheet href='x'?>)))" );
  // A text declaration must have the encoding and is only at the beginning.
  EXPECT_EQ( lambdaTree( "<?xml version='1.0'?>" ).substr( 0, 6 ), "ERROR:" );
  EXPECT_EQ( lambdaTree( " <?xml encoding='x'?>" ).substr( 0, 6 ), "ERROR:" );
  // Conditional sections.
  EXPECT_EQ( lambdaTree( "<![INCLUDE[]]>" ), "(extSubset (extSubsetDecl (conditionalSect (includeSect <![ INCLUDE [ (extSubsetDecl) ]]>))))" );
  EXPECT_EQ( lambdaTree( "<![ INCLUDE [ <!ELEMENT a EMPTY> ]]>" ),
    "(extSubset (extSubsetDecl (conditionalSect (includeSect <![ _ INCLUDE _ [ (extSubsetDecl (DeclSep _) (markupdecl (elementdecl <!ELEMENT _ a _ (contentspec EMPTY) >)) (DeclSep _)) ]]>))))" );
  EXPECT_EQ( lambdaTree( "<![IGNORE[]]>" ), "(extSubset (extSubsetDecl (conditionalSect (ignoreSect <![ IGNORE [ (ignoreSectContents) ]]>))))" );
  EXPECT_EQ( lambdaTree( "<![IGNORE[ any <thing> at ]all] ]>]]>" ),
    "(extSubset (extSubsetDecl (conditionalSect (ignoreSect <![ IGNORE [ (ignoreSectContents  any < thing> at ] all ] ] >) ]]>))))" ); // The pieces: " any ", "<", "thing> at ", "]", "all", "]", " ", "]", ">".
  // Sections nest - also within what is ignored.
  EXPECT_EQ( lambdaTree( "<![IGNORE[a<![b<![c]]>d]]>e]]>" ),
    "(extSubset (extSubsetDecl (conditionalSect (ignoreSect <![ IGNORE [ (ignoreSectContents a <![ (ignoreSectContents b <![ (ignoreSectContents c) ]]> d) ]]> e) ]]>))))" );
  EXPECT_EQ( lambdaTree( "<![INCLUDE[<![IGNORE[x]]><![INCLUDE[]]>]]>" ),
    "(extSubset (extSubsetDecl (conditionalSect (includeSect <![ INCLUDE [ (extSubsetDecl (conditionalSect (ignoreSect <![ IGNORE [ (ignoreSectContents x) ]]>)) (conditionalSect (includeSect <![ INCLUDE [ (extSubsetDecl) ]]>))) ]]>))))" );
  for ( const char * pszBad : { "<![INCLUDE[", "<![IGNORE[ a <![ b ]]>", "<![include[]]>", "<![INCLUDE []]", "]]>", "<![INCLUDE[ text ]]>", "<!DOCTYPE a>", "<![ANY[]]>" } )
    EXPECT_EQ( lambdaTree( pszBad ).substr( 0, 6 ), "ERROR:" ) << pszBad;
  // The replacement text of a parameter entity between declarations: declarations - without a text declaration.
  EXPECT_EQ( lambdaTree( "", s_knEntry_extSubsetDecl ), "(extSubsetDecl)" );
  EXPECT_EQ( lambdaTree( "<!ELEMENT a EMPTY> %p;", s_knEntry_extSubsetDecl ),
    "(extSubsetDecl (markupdecl (elementdecl <!ELEMENT _ a _ (contentspec EMPTY) >)) (DeclSep _) (DeclSep %p;))" );
  EXPECT_EQ( lambdaTree( "<?xml encoding='x'?>", s_knEntry_extSubsetDecl ).substr( 0, 6 ), "ERROR:" );
}

// Reading the declarations into an xml_dtd - xml_dtd_parser.
template < class t_TyChar >
std::string StrNarrow( std::basic_string_view< t_TyChar > _sv )
{
  std::string str;
  for ( t_TyChar ch : _sv )
    str += ( ( ch >= 0x20 ) && ( ch < 0x7f ) ) ? char( ch ) : '?';
  return str;
}
template < class t_TyChar >
std::string StrNarrow( std::basic_string< t_TyChar > const & _rstr )
{
  return StrNarrow( std::basic_string_view< t_TyChar >( _rstr ) );
}
// Read the text: a document from its "<!DOCTYPE" or an external subset. Return the error - empty if there is none.
template < class t_TyChar >
std::string StrReadDtdText( std::basic_string_view< t_TyChar > _svText, bool _fExternalSubset, xml_dtd< t_TyChar > & _rdtd, bool _fStandalone = false )
{
  try
  {
    xml_dtd_parser< t_TyChar > dtdParser( _rdtd );
    dtdParser.SetStandalone( _fStandalone );
    if ( _fExternalSubset )
      dtdParser.ParseExternalSubset( _svText );
    else
    {
      xml_parser< xml_traits< _l_transport_fixedmem< t_TyChar >, false, false > > xmlParser;
      xmlParser.emplaceTransport( _svText.data(), _svText.length() );
      dtdParser.ParseDoctypeDecl( xmlParser.GetLexicalAnalyzer() );
    }
  }
  catch ( std::exception const & rexc )
  {
    std::string strError = rexc.what();
    return strError.empty() ? std::string( "error" ) : strError;
  }
  return std::string();
}
template < class t_TyChar >
std::string StrReadDtd( const char * _pszText, xml_dtd< t_TyChar > & _rdtd, bool _fExternalSubset = false, bool _fStandalone = false )
{
  const std::basic_string< t_TyChar > strText( _pszText, _pszText + strlen( _pszText ) );
  return StrReadDtdText< t_TyChar >( strText, _fExternalSubset, _rdtd, _fStandalone );
}
// A content model as it is written - without white space.
template < class t_TyChar >
std::string StrContentParticle( xml_dtd_content_particle< t_TyChar > const & _rcp )
{
  std::string str;
  if ( exdpName == _rcp.m_eParticle )
    str = StrNarrow( _rcp.m_strName );
  else
  {
    str = "(";
    for ( size_t n = 0; n < _rcp.m_rgChildren.size(); ++n )
      str += ( !n ? "" : ( ( exdpChoice == _rcp.m_eParticle ) ? "|" : "," ) ) + StrContentParticle( _rcp.m_rgChildren[ n ] );
    str += ")";
  }
  static const char * const s_rgpszOccurrence[] = { "", "?", "*", "+" };
  return str + s_rgpszOccurrence[ _rcp.m_eOccurrence ];
}
template < class t_TyChar >
std::string StrJoin( std::vector< std::basic_string< t_TyChar > > const & _rrgstr )
{
  std::string str;
  for ( std::basic_string< t_TyChar > const & rstr : _rrgstr )
    str += ( str.empty() ? "" : " " ) + StrNarrow( rstr );
  return str;
}
template < class t_TyChar >
std::basic_string< t_TyChar > StrWiden( const char * _psz )
{
  return std::basic_string< t_TyChar >( _psz, _psz + strlen( _psz ) );
}

// What the declarations declare.
TYPED_TEST( DtdTest, Declarations )
{
  typedef TypeParam TyChar;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  xml_dtd< TyChar > dtd;
  ASSERT_EQ( StrReadDtd< TyChar >(
    "<!DOCTYPE doc PUBLIC '-//X//DTD doc//EN' \"doc.dtd\" [\n"
    "<!ELEMENT doc (head, (para | list)*, foot?)>\n"
    "<!ELEMENT head (#PCDATA)>\n"
    "<!ELEMENT para (#PCDATA | em | strong)*>\n"
    "<!ELEMENT list (item+)>\n"
    "<!ELEMENT item ANY>\n"
    "<!ELEMENT br EMPTY>\n"
    "<!ELEMENT deep ((a|b)+,(c,(d|e)?)*)+ >\n"
    "<!ATTLIST doc id ID #REQUIRED\n"
    "              lang NMTOKEN 'en'\n"
    "              version CDATA #FIXED \"1.0\">\n"
    "<!ATTLIST para align (left | right|center) \"left\" refs IDREFS #IMPLIED pic ENTITY #IMPLIED fmt NOTATION (gif|png) #IMPLIED>\n"
    "<!ATTLIST para align CDATA #IMPLIED extra NMTOKENS #IMPLIED>\n" // align is declared: the first binds - extra is added.
    "<!ATTLIST undeclared x ENTITIES #IMPLIED y IDREF #IMPLIED>\n"
    "<!ENTITY copy \"&#169; &#x41;&#66; &amp; &other; 'q'\">\n"
    "<!ENTITY copy 'the second declaration'>\n"
    "<!ENTITY empty ''>\n"
    "<!ENTITY chapter SYSTEM 'chapter.xml'>\n"
    "<!ENTITY logo PUBLIC \"-//X//logo\" 'logo.gif' NDATA gif>\n"
    "<!ENTITY % pe \"a parameter entity\">\n"
    "<!ENTITY % ext SYSTEM 'ext.ent'>\n"
    "<!NOTATION gif SYSTEM 'image/gif'>\n"
    "<!NOTATION png PUBLIC '-//X//png'>\n"
    "<!NOTATION jpg PUBLIC '-//X//jpg' \"image/jpeg\">\n"
    "<!-- a comment --><?pi a processing instruction?>\n"
    "]>", dtd ), "" );
  // The document type declaration.
  EXPECT_EQ( StrNarrow( dtd.m_strRootName ), "doc" );
  ASSERT_TRUE( dtd.m_optExternalId.has_value() );
  EXPECT_EQ( StrNarrow( *dtd.m_optExternalId->m_optstrPublic ), "-//X//DTD doc//EN" );
  EXPECT_EQ( StrNarrow( *dtd.m_optExternalId->m_optstrSystem ), "doc.dtd" );
  EXPECT_TRUE( dtd.m_fHasInternalSubset );
  EXPECT_FALSE( dtd.m_fStoppedProcessingDeclarations );
  EXPECT_TRUE( dtd.m_rgstrValidityErrors.empty() );
  // The element types.
  EXPECT_EQ( dtd.m_mapElements.size(), 7u );
  ASSERT_TRUE( !!dtd.PGetElement( W( "doc" ) ) );
  EXPECT_EQ( dtd.PGetElement( W( "doc" ) )->m_eContent, exdcChildren );
  EXPECT_EQ( StrContentParticle( dtd.PGetElement( W( "doc" ) )->m_cpChildren ), "(head,(para|list)*,foot?)" );
  EXPECT_EQ( StrContentParticle( dtd.PGetElement( W( "list" ) )->m_cpChildren ), "(item+)" );
  EXPECT_EQ( StrContentParticle( dtd.PGetElement( W( "deep" ) )->m_cpChildren ), "((a|b)+,(c,(d|e)?)*)+" );
  EXPECT_EQ( dtd.PGetElement( W( "head" ) )->m_eContent, exdcMixed );
  EXPECT_TRUE( dtd.PGetElement( W( "head" ) )->m_rgstrMixedNames.empty() );
  EXPECT_EQ( dtd.PGetElement( W( "para" ) )->m_eContent, exdcMixed );
  EXPECT_EQ( StrJoin( dtd.PGetElement( W( "para" ) )->m_rgstrMixedNames ), "em strong" );
  EXPECT_EQ( dtd.PGetElement( W( "item" ) )->m_eContent, exdcAny );
  EXPECT_EQ( dtd.PGetElement( W( "br" ) )->m_eContent, exdcEmpty );
  EXPECT_TRUE( !dtd.PGetElement( W( "em" ) ) );
  // The attributes.
  ASSERT_TRUE( !!dtd.PGetAttList( W( "doc" ) ) );
  ASSERT_EQ( dtd.PGetAttList( W( "doc" ) )->size(), 3u );
  EXPECT_EQ( dtd.PGetAttDef( W( "doc" ), W( "id" ) )->m_eType, exdatID );
  EXPECT_EQ( dtd.PGetAttDef( W( "doc" ), W( "id" ) )->m_eDefault, exdadRequired );
  EXPECT_EQ( dtd.PGetAttDef( W( "doc" ), W( "lang" ) )->m_eType, exdatNMTOKEN );
  EXPECT_EQ( dtd.PGetAttDef( W( "doc" ), W( "lang" ) )->m_eDefault, exdadDefault );
  EXPECT_EQ( StrNarrow( dtd.PGetAttDef( W( "doc" ), W( "lang" ) )->m_strDefault ), "en" );
  EXPECT_EQ( dtd.PGetAttDef( W( "doc" ), W( "version" ) )->m_eType, exdatCDATA );
  EXPECT_EQ( dtd.PGetAttDef( W( "doc" ), W( "version" ) )->m_eDefault, exdadFixed );
  EXPECT_EQ( StrNarrow( dtd.PGetAttDef( W( "doc" ), W( "version" ) )->m_strDefault ), "1.0" );
  ASSERT_EQ( dtd.PGetAttList( W( "para" ) )->size(), 5u ); // align, refs, pic, fmt and extra from the second declaration.
  EXPECT_EQ( dtd.PGetAttDef( W( "para" ), W( "align" ) )->m_eType, exdatEnumeration ); // The first declaration.
  EXPECT_EQ( StrJoin( dtd.PGetAttDef( W( "para" ), W( "align" ) )->m_rgstrEnumeration ), "left right center" );
  EXPECT_EQ( StrNarrow( dtd.PGetAttDef( W( "para" ), W( "align" ) )->m_strDefault ), "left" );
  EXPECT_EQ( dtd.PGetAttDef( W( "para" ), W( "refs" ) )->m_eType, exdatIDREFS );
  EXPECT_EQ( dtd.PGetAttDef( W( "para" ), W( "refs" ) )->m_eDefault, exdadImplied );
  EXPECT_EQ( dtd.PGetAttDef( W( "para" ), W( "pic" ) )->m_eType, exdatENTITY );
  EXPECT_EQ( dtd.PGetAttDef( W( "para" ), W( "fmt" ) )->m_eType, exdatNOTATION );
  EXPECT_EQ( StrJoin( dtd.PGetAttDef( W( "para" ), W( "fmt" ) )->m_rgstrEnumeration ), "gif png" );
  EXPECT_EQ( dtd.PGetAttDef( W( "para" ), W( "extra" ) )->m_eType, exdatNMTOKENS );
  EXPECT_EQ( dtd.PGetAttDef( W( "undeclared" ), W( "x" ) )->m_eType, exdatENTITIES ); // Attributes may be declared for an element type which isn't.
  EXPECT_EQ( dtd.PGetAttDef( W( "undeclared" ), W( "y" ) )->m_eType, exdatIDREF );
  EXPECT_TRUE( !dtd.PGetAttDef( W( "doc" ), W( "none" ) ) );
  EXPECT_TRUE( !dtd.PGetAttList( W( "br" ) ) );
  // The entities. The replacement text: character references are replaced and general entity references are not.
  EXPECT_EQ( dtd.m_mapGeneralEntities.size(), 4u );
  ASSERT_TRUE( !!dtd.PGetGeneralEntity( W( "copy" ) ) );
  {
    std::basic_string< TyChar > strExpected;
    const char32_t chCopyright = 0xA9;
    ConvertString( strExpected, &chCopyright, 1 ); // One, two or one code units.
    strExpected += W( " AB &amp; &other; 'q'" );
    EXPECT_TRUE( dtd.PGetGeneralEntity( W( "copy" ) )->m_strValue == strExpected ); // The first declaration.
  }
  EXPECT_TRUE( dtd.PGetGeneralEntity( W( "copy" ) )->m_fInternal );
  EXPECT_FALSE( dtd.PGetGeneralEntity( W( "copy" ) )->m_fParameter );
  EXPECT_FALSE( dtd.PGetGeneralEntity( W( "copy" ) )->m_fDeclaredExternally );
  EXPECT_TRUE( dtd.PGetGeneralEntity( W( "empty" ) )->m_strValue.empty() );
  EXPECT_FALSE( dtd.PGetGeneralEntity( W( "chapter" ) )->m_fInternal );
  EXPECT_FALSE( dtd.PGetGeneralEntity( W( "chapter" ) )->FIsUnparsed() );
  EXPECT_EQ( StrNarrow( *dtd.PGetGeneralEntity( W( "chapter" ) )->m_externalId.m_optstrSystem ), "chapter.xml" );
  EXPECT_FALSE( dtd.PGetGeneralEntity( W( "chapter" ) )->m_externalId.m_optstrPublic.has_value() );
  EXPECT_TRUE( dtd.PGetGeneralEntity( W( "logo" ) )->FIsUnparsed() );
  EXPECT_EQ( StrNarrow( dtd.PGetGeneralEntity( W( "logo" ) )->m_strNotation ), "gif" );
  EXPECT_EQ( StrNarrow( *dtd.PGetGeneralEntity( W( "logo" ) )->m_externalId.m_optstrPublic ), "-//X//logo" );
  EXPECT_EQ( StrNarrow( *dtd.PGetGeneralEntity( W( "logo" ) )->m_externalId.m_optstrSystem ), "logo.gif" );
  EXPECT_EQ( dtd.m_mapParameterEntities.size(), 2u );
  EXPECT_EQ( StrNarrow( dtd.PGetParameterEntity( W( "pe" ) )->m_strValue ), "a parameter entity" );
  EXPECT_TRUE( dtd.PGetParameterEntity( W( "pe" ) )->m_fParameter );
  EXPECT_FALSE( dtd.PGetParameterEntity( W( "ext" ) )->m_fInternal );
  EXPECT_TRUE( !dtd.PGetGeneralEntity( W( "pe" ) ) ); // General and parameter entities are different names.
  // The notations.
  EXPECT_EQ( dtd.m_mapNotations.size(), 3u );
  EXPECT_EQ( StrNarrow( *dtd.PGetNotation( W( "gif" ) )->m_externalId.m_optstrSystem ), "image/gif" );
  EXPECT_FALSE( dtd.PGetNotation( W( "gif" ) )->m_externalId.m_optstrPublic.has_value() );
  EXPECT_EQ( StrNarrow( *dtd.PGetNotation( W( "png" ) )->m_externalId.m_optstrPublic ), "-//X//png" );
  EXPECT_FALSE( dtd.PGetNotation( W( "png" ) )->m_externalId.m_optstrSystem.has_value() );
  EXPECT_EQ( StrNarrow( *dtd.PGetNotation( W( "jpg" ) )->m_externalId.m_optstrSystem ), "image/jpeg" );

  // Without an external subset or an internal one.
  xml_dtd< TyChar > dtdBare;
  ASSERT_EQ( StrReadDtd< TyChar >( "<!DOCTYPE a>", dtdBare ), "" );
  EXPECT_EQ( StrNarrow( dtdBare.m_strRootName ), "a" );
  EXPECT_FALSE( dtdBare.m_optExternalId.has_value() );
  EXPECT_FALSE( dtdBare.m_fHasInternalSubset );
  // A character which is more than one code unit in UTF-16 and in UTF-8.
  xml_dtd< TyChar > dtdChar;
  ASSERT_EQ( StrReadDtd< TyChar >( "<!DOCTYPE a [<!ENTITY e 'x&#x1F600;y'>]>", dtdChar ), "" );
  EXPECT_EQ( dtdChar.PGetGeneralEntity( W( "e" ) )->m_strValue.length(), 2u + ( ( sizeof( TyChar ) == 1 ) ? 4u : ( ( sizeof( TyChar ) == 2 ) ? 2u : 1u ) ) );
  // Not a character.
  xml_dtd< TyChar > dtdBadChar;
  EXPECT_NE( StrReadDtd< TyChar >( "<!DOCTYPE a [<!ENTITY e 'x&#0;y'>]>", dtdBadChar ), "" );
  // Not well formed: the error of the parser.
  xml_dtd< TyChar > dtdBad;
  EXPECT_NE( StrReadDtd< TyChar >( "<!DOCTYPE a [<!ELEMENT a (b,c|d)>]>", dtdBad ).find( "The DTD is not well formed: syntax error: unexpected ['|']" ), std::string::npos );
}

// The validity constraints which the declarations themselves don't meet are recorded - the DTD is read all the same.
TYPED_TEST( DtdTest, ValidityOfDeclarations )
{
  typedef TypeParam TyChar;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  xml_dtd< TyChar > dtd;
  ASSERT_EQ( StrReadDtd< TyChar >(
    "<!DOCTYPE a [\n"
    "<!ELEMENT a EMPTY>\n"
    "<!ELEMENT a ANY>\n"
    "<!ELEMENT b (#PCDATA|x|y|x)*>\n"
    "<!ATTLIST a e (one|two|one) #IMPLIED n NOTATION (g|g) #IMPLIED>\n"
    "<!NOTATION g SYSTEM 's'>\n"
    "<!NOTATION g SYSTEM 't'>\n"
    "]>", dtd ), "" );
  EXPECT_EQ( dtd.m_rgstrValidityErrors, ( std::vector< std::string >{
    "Element type [a] is declared more than once.",
    "Element type [x] is more than once in a mixed content declaration.",
    "[one] is more than once in the enumerated type of an attribute.",
    "[g] is more than once in the enumerated type of an attribute.",
    "Notation [g] is declared more than once.",
    // And what is known once all of the declarations have been read:
    "The attribute [n] of the element type [a] is of type NOTATION and the element type is declared EMPTY." } ) );
  // What was declared first is what there is.
  EXPECT_EQ( dtd.PGetElement( W( "a" ) )->m_eContent, exdcEmpty );
  EXPECT_EQ( StrJoin( dtd.PGetElement( W( "b" ) )->m_rgstrMixedNames ), "x y" );
  EXPECT_EQ( StrJoin( dtd.PGetAttDef( W( "a" ), W( "e" ) )->m_rgstrEnumeration ), "one two" );
  EXPECT_EQ( StrNarrow( *dtd.PGetNotation( W( "g" ) )->m_externalId.m_optstrSystem ), "s" );
}

// References to parameter entities.
TYPED_TEST( DtdTest, ParameterEntities )
{
  typedef TypeParam TyChar;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  // Between declarations: the replacement text is declarations.
  {
    xml_dtd< TyChar > dtd;
    ASSERT_EQ( StrReadDtd< TyChar >(
      "<!DOCTYPE a [\n"
      "<!ENTITY % decls \"<!ELEMENT a (b)> <!ATTLIST a x CDATA #IMPLIED> <!ENTITY inner 'from decls'>\">\n"
      "<!ENTITY % outer '&#37;decls; <!ELEMENT b EMPTY>'>\n" // '%' by a character reference: a reference may not be written within a declaration here.
      "<!ENTITY % empty ''>\n"
      "%outer; %empty;\n"
      "<!ENTITY inner 'the second declaration'>\n"
      "<!ELEMENT c EMPTY>\n"
      "]>", dtd ), "" );
    EXPECT_EQ( dtd.m_mapElements.size(), 3u );
    EXPECT_EQ( StrContentParticle( dtd.PGetElement( W( "a" ) )->m_cpChildren ), "(b)" );
    EXPECT_EQ( dtd.PGetElement( W( "b" ) )->m_eContent, exdcEmpty );
    EXPECT_TRUE( !!dtd.PGetAttDef( W( "a" ), W( "x" ) ) );
    EXPECT_EQ( StrNarrow( dtd.PGetGeneralEntity( W( "inner" ) )->m_strValue ), "from decls" );
    EXPECT_TRUE( dtd.PGetGeneralEntity( W( "inner" ) )->m_fDeclaredExternally ); // Declared in a parameter entity.
    EXPECT_FALSE( dtd.PGetParameterEntity( W( "decls" ) )->m_fDeclaredExternally );
    EXPECT_EQ( StrNarrow( dtd.PGetParameterEntity( W( "outer" ) )->m_strValue ), "%decls; <!ELEMENT b EMPTY>" );
    EXPECT_FALSE( dtd.m_fStoppedProcessingDeclarations );
  }
  // Well-formedness constraints.
  auto lambdaError = []( const char * _pszSubset, bool _fStandalone = false ) -> std::string
  {
    xml_dtd< TyChar > dtd;
    return StrReadDtd< TyChar >( ( std::string( "<!DOCTYPE a [" ) + _pszSubset + "]>" ).c_str(), dtd, false, _fStandalone );
  };
  // A parameter entity must be declared before it is referenced: for a standalone document that is well-formedness.
  EXPECT_NE( lambdaError( "%nope;", true ).find( "the parameter entity [nope] is referenced but not declared" ), std::string::npos );
  EXPECT_NE( lambdaError( "%later; <!ENTITY % later ''>", true ).find( "the parameter entity [later] is referenced but not declared" ), std::string::npos );
  // Otherwise it is validity: it is recorded, and the entity and attribute-list declarations after it are not processed.
  {
    xml_dtd< TyChar > dtd;
    ASSERT_EQ( StrReadDtd< TyChar >( "<!DOCTYPE a [<!ENTITY before 'b'> %nope; <!ENTITY after 'a'> <!ELEMENT a EMPTY>]>", dtd ), "" );
    EXPECT_EQ( dtd.m_rgstrValidityErrors, ( std::vector< std::string >{ "Parameter entity [nope] is referenced but not declared." } ) );
    EXPECT_TRUE( dtd.m_fStoppedProcessingDeclarations );
    EXPECT_TRUE( !!dtd.PGetGeneralEntity( W( "before" ) ) );
    EXPECT_TRUE( !dtd.PGetGeneralEntity( W( "after" ) ) );
    EXPECT_TRUE( !!dtd.PGetElement( W( "a" ) ) );
  }
  EXPECT_NE( lambdaError( "<!ENTITY % a '&#37;a;'> %a;" ).find( "the parameter entity [a] contains a reference to itself" ), std::string::npos );
  EXPECT_NE( lambdaError( "<!ENTITY % a '&#37;b;'> <!ENTITY % b '&#37;a;'> %a;" ).find( "contains a reference to itself" ), std::string::npos );
  EXPECT_NE( lambdaError( "<!ENTITY % p 'v'> <!ENTITY e 'a %p; b'>" ).find( "the parameter entity reference [%p;] is within a declaration in the internal subset" ), std::string::npos );
  // The replacement text must be declarations.
  EXPECT_NE( lambdaError( "<!ENTITY % half '<!ELEMENT a'> %half; EMPTY>" ).find( "The DTD is not well formed: syntax error" ), std::string::npos );
  EXPECT_NE( lambdaError( "<!ENTITY % text 'just text'> %text;" ).find( "The DTD is not well formed: syntax error" ), std::string::npos );
  EXPECT_EQ( lambdaError( "<!ENTITY % ok '<!ELEMENT a EMPTY>'> %ok;" ), "" );

  // An external parameter entity isn't read: the entity and attribute-list declarations after the reference are not
  //  processed - the element type and notation declarations are.
  {
    xml_dtd< TyChar > dtd;
    ASSERT_EQ( StrReadDtd< TyChar >(
      "<!DOCTYPE a [\n"
      "<!ENTITY before 'b'>\n"
      "<!ENTITY % ext SYSTEM 'ext.ent'>\n"
      "%ext;\n"
      "<!ENTITY after 'a'>\n"
      "<!ENTITY % pafter 'p'>\n"
      "<!ATTLIST a x CDATA #IMPLIED>\n"
      "<!ELEMENT a EMPTY>\n"
      "<!NOTATION n SYSTEM 's'>\n"
      "%unknown;\n" // It may have been declared in what wasn't read: not an error of well-formedness - even for a standalone document.
      "]>", dtd, false, true ), "" );
    EXPECT_TRUE( dtd.m_fStoppedProcessingDeclarations );
    EXPECT_TRUE( !!dtd.PGetGeneralEntity( W( "before" ) ) );
    EXPECT_TRUE( !dtd.PGetGeneralEntity( W( "after" ) ) );
    EXPECT_TRUE( !dtd.PGetParameterEntity( W( "pafter" ) ) );
    EXPECT_TRUE( !dtd.PGetAttList( W( "a" ) ) );
    EXPECT_TRUE( !!dtd.PGetElement( W( "a" ) ) );
    EXPECT_TRUE( !!dtd.PGetNotation( W( "n" ) ) );
    EXPECT_EQ( dtd.m_rgstrValidityErrors, ( std::vector< std::string >{ "Parameter entity [unknown] is referenced but not declared." } ) );
  }
}

// The external subset: a parameter entity reference may be within an entity value there, and conditional sections.
TYPED_TEST( DtdTest, ExternalSubsetDeclarations )
{
  typedef TypeParam TyChar;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  xml_dtd< TyChar > dtd;
  ASSERT_EQ( StrReadDtd< TyChar >(
    "<?xml version='1.0' encoding='UTF-8'?>\n"
    "<!ENTITY % p 'P&#38;Q'>\n"
    "<!ENTITY e \"a %p; b %p;\">\n"
    "<![INCLUDE[ <!ELEMENT included EMPTY> <![IGNORE[ <!ELEMENT ignored EMPTY> <![ nested ]]> ]]> ]]>\n"
    "<![ IGNORE [ <!ELEMENT ignored2 EMPTY> ]]>\n"
    "<!ELEMENT last ANY>\n", dtd, true ), "" );
  EXPECT_EQ( StrNarrow( dtd.PGetParameterEntity( W( "p" ) )->m_strValue ), "P&Q" );
  EXPECT_EQ( StrNarrow( dtd.PGetGeneralEntity( W( "e" ) )->m_strValue ), "a P&Q b P&Q" );
  EXPECT_TRUE( dtd.PGetGeneralEntity( W( "e" ) )->m_fDeclaredExternally );
  EXPECT_EQ( dtd.m_mapElements.size(), 2u );
  EXPECT_TRUE( !!dtd.PGetElement( W( "included" ) ) );
  EXPECT_TRUE( !!dtd.PGetElement( W( "last" ) ) );
  EXPECT_TRUE( !dtd.PGetElement( W( "ignored" ) ) );
  // A parameter entity must be declared before it is referenced within an entity value as well.
  xml_dtd< TyChar > dtdBad;
  EXPECT_NE( StrReadDtd< TyChar >( "<!ENTITY e 'a %p; b'>", dtdBad, true, true ).find( "the parameter entity [p] is referenced but not declared" ), std::string::npos );
  xml_dtd< TyChar > dtdInvalid;
  EXPECT_EQ( StrReadDtd< TyChar >( "<!ENTITY e 'a %p; b'>", dtdInvalid, true ), "" );
  EXPECT_EQ( dtdInvalid.m_rgstrValidityErrors.size(), 1u );
}

// The cursor: a document read with XML traits which support the DTD - xml_traits< transport, true, false >.
template < class t_TyChar, bool t_kfSupportDTD = true, bool t_kfValidating = false >
struct CursorDocument
{
  typedef xml_traits< _l_transport_fixedmem< t_TyChar >, t_kfSupportDTD, t_kfValidating > _TyXmlTraits;
  typedef xml_parser< _TyXmlTraits > _TyXmlParser;
  typedef xml_read_cursor< _TyXmlTraits > _TyCursor;
  std::basic_string< t_TyChar > m_strText; // The cursor reads this in place.
  _TyXmlParser m_xmlParser;
  _TyCursor m_cursor;
  std::string m_strError; // If the document couldn't be opened.

  // _rresolver: how the external entities of the DTD are had - if they are to be read.
  // _pszBase: the location of the document, for the resolver.
  // _fUseXMLNamespaces: whether the document is read with Namespaces in XML.
  // _plimits: what the document may have the parser do by its DTD - null for the parser's own.
  explicit CursorDocument(  const char * _pszText, xml_external_entity_resolver< t_TyChar > const & _rresolver = nullptr, const char * _pszBase = nullptr, bool _fUseXMLNamespaces = true,
                            const xml_dtd_limits * _plimits = nullptr )
  {
    if ( sizeof( t_TyChar ) > 1 )
      m_strText += t_TyChar( 0xFEFF ); // The byte order mark: how the encoding of what is in memory is known.
    m_strText.append( _pszText, _pszText + strlen( _pszText ) );
    try
    {
      if ( !!_rresolver )
        m_xmlParser.SetExternalEntityResolver( _rresolver );
      m_xmlParser.SetUseXMLNamespaces( _fUseXMLNamespaces );
      if ( !!_plimits )
        m_xmlParser.SetDtdLimits( *_plimits );
      if ( !!_pszBase )
        m_xmlParser.SetBaseLocation( std::basic_string< t_TyChar >( _pszBase, _pszBase + strlen( _pszBase ) ) );
      m_cursor = m_xmlParser.OpenMemory( m_strText.data(), m_strText.length() * sizeof( t_TyChar ) );
    }
    catch ( std::exception const & rexc )
    {
      m_strError = rexc.what();
      if ( m_strError.empty() )
        m_strError = "error";
    }
  }
  bool FOpened() const
  {
    return m_strError.empty();
  }
  // The character data of the current tag - up to its first child - as the cursor has it: with the references replaced.
  std::basic_string< t_TyChar > StrContent()
  {
    std::basic_string< t_TyChar > str;
    m_cursor.GetContextCur().GetContentString( str );
    return str;
  }
  // The attributes of the current tag: name="value" for each, in order.
  std::string StrAttributes()
  {
    std::string str;
    auto & rltokTag = m_cursor.GetTagCur().GetLexToken();
    auto & rrgAttrs = rltokTag.GetValue()[ vknAttributesIdx ];
    for ( size_t nAttr = 0; nAttr < rrgAttrs.GetSize(); ++nAttr )
    {
      std::basic_string_view< t_TyChar > svName, svValue;
      // What the tag has is in the document and what was added is a string: asking the token or asking the value is the same.
      rltokTag.GetStringView( svName, rrgAttrs[ nAttr ][ vknNameIdx ] );
      rrgAttrs[ nAttr ][ vknAttr_ValueIdx ].GetStringView( rltokTag, svValue );
      str += ( str.empty() ? "" : " " ) + StrNarrow( svName ) + "=\"" + StrNarrow( svValue ) + "\"";
    }
    return str;
  }
};

TYPED_TEST( DtdTest, Cursor )
{
  typedef TypeParam TyChar;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  static const char s_szDocument[] =
    "<?xml version=\"1.0\"?>\n"
    "<!-- before -->\n"
    "<!DOCTYPE doc SYSTEM 'doc.dtd' [\n"
    "<!ELEMENT doc (#PCDATA|item)*>\n"
    "<!ELEMENT item EMPTY>\n"
    "<!ATTLIST doc title CDATA #IMPLIED>\n"
    "<!ENTITY company \"Bien &amp; Co\">\n"
    "<!ENTITY nested 'at &company; (&#169;) &#38;#60;tag>'>\n"
    "<!ENTITY markup '<b>bold</b>'>\n"
    "<!ENTITY lt 'redeclared'>\n"
    "<!ENTITY external SYSTEM 'external.xml'>\n"
    "]>\n"
    "<?pi after?>\n"
    "<doc title=\"about &company;\">Made &nested;.<item/> &lt;</doc>\n";
  CursorDocument< TyChar > doc( s_szDocument );
  ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
  // The document type declaration has been read: it is in the prolog and the cursor is at the first tag.
  ASSERT_TRUE( doc.m_cursor.FHasDtd() );
  xml_dtd< TyChar > const & rdtd = doc.m_cursor.GetDtd();
  EXPECT_EQ( StrNarrow( rdtd.m_strRootName ), "doc" );
  EXPECT_EQ( StrNarrow( *rdtd.m_optExternalId->m_optstrSystem ), "doc.dtd" );
  EXPECT_EQ( rdtd.m_mapElements.size(), 2u );
  EXPECT_TRUE( !!rdtd.PGetAttDef( W( "doc" ), W( "title" ) ) );
  EXPECT_EQ( rdtd.m_mapGeneralEntities.size(), 5u );
  // The entities of the document: those which are text alone - and the predefined ones are what they were.
  auto const & rmapEntities = doc.m_xmlParser.GetUserObj().GetEntityMap();
  EXPECT_EQ( rmapEntities.size(), 7u ); // The five predefined, company and nested.
  ASSERT_TRUE( rmapEntities.end() != rmapEntities.find( W( "company" ) ) );
  EXPECT_EQ( StrNarrow( rmapEntities.find( W( "company" ) )->second ), "Bien & Co" );
  ASSERT_TRUE( rmapEntities.end() != rmapEntities.find( W( "nested" ) ) );
  EXPECT_TRUE( rmapEntities.end() == rmapEntities.find( W( "markup" ) ) );
  EXPECT_TRUE( rmapEntities.end() == rmapEntities.find( W( "external" ) ) );
  EXPECT_EQ( StrNarrow( rmapEntities.find( W( "lt" ) )->second ), "<" );
  {
    // "&#38;#60;" in the literal is "&#60;" in the replacement text and that is the character '<' - not markup - where it is referenced.
    std::basic_string< TyChar > strNested = W( "at Bien & Co (" );
    const char32_t chCopyright = 0xA9;
    std::basic_string< TyChar > strCopyright;
    ConvertString( strCopyright, &chCopyright, 1 );
    strNested += strCopyright + W( ") <tag>" );
    EXPECT_TRUE( rmapEntities.find( W( "nested" ) )->second == strNested );
    // The document: the references in its character data are replaced.
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_TRUE( doc.StrContent() == W( "Made " ) + strNested + W( "." ) );
  }
  // And those in the values of its attributes.
  {
    auto & rltokTag = doc.m_cursor.GetTagCur().GetLexToken();
    auto & rvalTitle = rltokTag.GetValue()[ vknAttributesIdx ][ 0 ][ vknAttr_ValueIdx ];
    // While the value is the positions of the pieces in the document - it has a reference so there is no view of it.
    ASSERT_TRUE( rvalTitle.FHasTypedData() );
    {
      std::basic_string_view< TyChar > svTok, svVal;
      std::basic_string< TyChar > strTok, strVal, strGetTok, strGetVal;
      EXPECT_FALSE( rltokTag.FGetStringViewOrString( svTok, strTok, rvalTitle ) );
      EXPECT_FALSE( rvalTitle.FGetStringViewOrString( rltokTag, svVal, strVal ) );
      rltokTag.GetString( strGetTok, rvalTitle );
      rvalTitle.GetString( rltokTag, strGetVal );
      EXPECT_EQ( StrNarrow( svTok ), "about Bien & Co" );
      EXPECT_EQ( StrNarrow( svVal ), "about Bien & Co" );
      EXPECT_EQ( StrNarrow( strGetTok ), "about Bien & Co" );
      EXPECT_EQ( StrNarrow( strGetVal ), "about Bien & Co" );
      EXPECT_TRUE( rvalTitle.FHasTypedData() );
    }
    // GetStringView() leaves the string in the value: any number of times, through the token or through the value.
    for ( int nRepeat = 0; nRepeat < 2; ++nRepeat )
    {
      std::basic_string_view< TyChar > svTok, svVal, svKTok, svKVal, svOrTok, svOrVal;
      std::basic_string< TyChar > strOrTok, strOrVal, strGetTok, strGetVal;
      rltokTag.GetStringView( svTok, rvalTitle );
      rvalTitle.GetStringView( rltokTag, svVal );
      EXPECT_FALSE( rvalTitle.FHasTypedData() );
      rltokTag.KGetStringView( svKTok, rvalTitle );
      rvalTitle.KGetStringView( rltokTag, svKVal );
      EXPECT_TRUE( rltokTag.FGetStringViewOrString( svOrTok, strOrTok, rvalTitle ) );
      EXPECT_TRUE( rvalTitle.FGetStringViewOrString( rltokTag, svOrVal, strOrVal ) );
      rltokTag.GetString( strGetTok, rvalTitle );
      rvalTitle.GetString( rltokTag, strGetVal );
      for ( std::basic_string_view< TyChar > sv : { svTok, svVal, svKTok, svKVal, svOrTok, svOrVal, std::basic_string_view< TyChar >( strGetTok ), std::basic_string_view< TyChar >( strGetVal ) } )
        EXPECT_EQ( StrNarrow( sv ), "about Bien & Co" );
    }
  }
}

// The document type declaration and the cursor: where it may be and what is an error.
TYPED_TEST( DtdTest, CursorDoctype )
{
  typedef TypeParam TyChar;
  // A document without a document type declaration is read as it always was.
  {
    CursorDocument< TyChar > doc( "<?xml version='1.0'?><!-- c --><a>text &amp; more</a>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    EXPECT_FALSE( doc.m_cursor.FHasDtd() );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( StrNarrow( doc.StrContent() ), "text & more" );
  }
  // Without an XML declaration, without anything before it and without an internal subset.
  {
    CursorDocument< TyChar > doc( "<!DOCTYPE a><a/>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FHasDtd() );
    EXPECT_EQ( StrNarrow( doc.m_cursor.GetDtd().m_strRootName ), "a" );
    EXPECT_FALSE( doc.m_cursor.GetDtd().m_fHasInternalSubset );
  }
  // An entity which has markup: what it has is content of the document where it is referenced - see EntitiesWithMarkup below.
  {
    CursorDocument< TyChar > doc( "<!DOCTYPE a [<!ENTITY m '<b/>'>]><a>x &m; y</a>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( StrNarrow( doc.StrContent() ), "x " ); // Up to its first sub-tag - which is the entity's.
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( StrNarrow( doc.m_cursor.SvTagCur() ), "b" );
  }
  // Not well formed: the errors of the DTD are the errors of the document.
  EXPECT_NE( CursorDocument< TyChar >( "<!DOCTYPE a [<!ELEMENT a (b,c|d)>]><a/>" ).m_strError.find( "The DTD is not well formed" ), std::string::npos );
  EXPECT_NE( CursorDocument< TyChar >( "<!DOCTYPE a [<!ENTITY % p 'v'><!ENTITY e '%p;'>]><a/>" ).m_strError.find( "within a declaration in the internal subset" ), std::string::npos );
  EXPECT_FALSE( CursorDocument< TyChar >( "<!DOCTYPE a [" ).FOpened() );
  // The document is declared standalone: a reference to a parameter entity which isn't declared is not well formed.
  EXPECT_NE( CursorDocument< TyChar >( "<?xml version='1.0' standalone='yes'?><!DOCTYPE a [ %p; ]><a/>" ).m_strError.find( "the parameter entity [p] is referenced but not declared" ), std::string::npos );
  {
    CursorDocument< TyChar > doc( "<?xml version='1.0' standalone='no'?><!DOCTYPE a [ %p; ]><a/>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    EXPECT_EQ( doc.m_cursor.GetDtd().m_rgstrValidityErrors.size(), 1u );
  }
  // Only one, and only a document type declaration.
  EXPECT_FALSE( CursorDocument< TyChar >( "<!DOCTYPE a><!DOCTYPE a><a/>" ).FOpened() );
  EXPECT_NE( CursorDocument< TyChar >( "<!ELEMENT a EMPTY><a/>" ).m_strError.find( "neither a document type declaration nor the document's element" ), std::string::npos );
  // XML traits which don't support the DTD: a document type declaration is not read - as it was before.
  EXPECT_FALSE( ( CursorDocument< TyChar, false >( "<!DOCTYPE a><a/>" ).FOpened() ) );
  EXPECT_TRUE( ( CursorDocument< TyChar, false >( "<a/>" ).FOpened() ) );
}

// The default values of attributes: what a declaration makes of the literal - 3.3.3.
TYPED_TEST( DtdTest, AttributeDefaultValues )
{
  typedef TypeParam TyChar;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  xml_dtd< TyChar > dtd;
  ASSERT_EQ( StrReadDtd< TyChar >(
    "<!DOCTYPE a [\n"
    "<!ENTITY co 'Bien\t&amp;  Co'>\n"
    "<!ENTITY deep '[&co;]'>\n"
    "<!ATTLIST a\n"
    "  plain CDATA 'a value'\n"
    "  spaces CDATA '  two  spaces\there  '\n"
    "  tokens NMTOKENS '  one   two\n three '\n"
    "  refs CDATA 'x&#32;&#x9;y &lt;&#38;&gt; &deep;'\n"
    "  id IDREF ' &#32; i '\n"
    "  fixed CDATA #FIXED \"f\"\n"
    "  required CDATA #REQUIRED\n"
    "  implied CDATA #IMPLIED\n"
    "  empty CDATA ''>\n"
    "]>", dtd ), "" );
  auto lambdaDefault = [&]( const char * _pszAttr ) -> std::string
  {
    const xml_dtd_attdef< TyChar > * pad = dtd.PGetAttDef( W( "a" ), W( _pszAttr ) );
    return !pad ? std::string( "NONE" ) : ( pad->m_fHasDefaultValue ? ( "[" + StrNarrow( pad->m_strDefaultValue ) + "]" ) : std::string( "no default" ) );
  };
  EXPECT_EQ( lambdaDefault( "plain" ), "[a value]" );
  EXPECT_EQ( lambdaDefault( "spaces" ), "[  two  spaces here  ]" ); // CDATA: each white space character is a space - nothing more.
  EXPECT_EQ( lambdaDefault( "tokens" ), "[one two three]" ); // Not CDATA: trimmed and spaces in a row are one.
  // A character reference to white space is that character. The white space of an entity's replacement text is a space.
  EXPECT_EQ( lambdaDefault( "refs" ), "[x ?y <&> [Bien &  Co]]" ); // The '?' is the tab: StrNarrow() shows what isn't printable so.
  EXPECT_EQ( lambdaDefault( "id" ), "[i]" );
  EXPECT_EQ( lambdaDefault( "fixed" ), "[f]" );
  EXPECT_EQ( lambdaDefault( "required" ), "no default" );
  EXPECT_EQ( lambdaDefault( "implied" ), "no default" );
  EXPECT_EQ( lambdaDefault( "empty" ), "[]" );
  EXPECT_EQ( StrNarrow( dtd.PGetAttDef( W( "a" ), W( "tokens" ) )->m_strDefault ), "  one   two? three " ); // As written.
  // Well-formedness: the entities which a default value refers to - 3.1.
  auto lambdaError = []( const char * _pszSubset, bool _fStandalone = false ) -> std::string
  {
    xml_dtd< TyChar > dtdError;
    return StrReadDtd< TyChar >( ( std::string( "<!DOCTYPE a [" ) + _pszSubset + "]>" ).c_str(), dtdError, false, _fStandalone );
  };
  EXPECT_NE( lambdaError( "<!ENTITY e SYSTEM 'e.xml'><!ATTLIST a x CDATA '&e;'>" ).find( "the default value of attribute [x] refers to an external entity" ), std::string::npos );
  EXPECT_NE( lambdaError( "<!ENTITY e '<b/>'><!ATTLIST a x CDATA '&e;'>" ).find( "has a '<' in its replacement text" ), std::string::npos );
  EXPECT_NE( lambdaError( "<!ENTITY e '&#38;f;'><!ENTITY f '&#38;e;'><!ATTLIST a x CDATA '&e;'>" ).find( "contains a reference to itself" ), std::string::npos );
  EXPECT_EQ( lambdaError( "<!ENTITY e '&#38;#60;'><!ATTLIST a x CDATA '&e;'>" ), "" ); // The character '<' - not markup.
  // An entity must be declared before the default value which refers to it ( 4.1 WFC: Entity Declared ): that is a matter
  //  of well-formedness if the document is standalone - or has no external subset and no reference to a parameter entity.
  EXPECT_NE( lambdaError( "<!ATTLIST a x CDATA '&later;'><!ENTITY later 'v'>", true ).find( "refers to an entity which is not declared" ), std::string::npos );
  {
    xml_dtd< TyChar > dtdNotWellFormed;
    EXPECT_NE( StrReadDtd< TyChar >( "<!DOCTYPE a [<!ATTLIST a x CDATA '&later;' y CDATA 'ok'><!ENTITY later 'v'>]>", dtdNotWellFormed ).find( "refers to an entity which is not declared" ), std::string::npos );
  }
  // Otherwise it is a matter of validity: the declaration of the entity may be where it isn't read.
  for ( const char * pszDoctype : { "<!DOCTYPE a SYSTEM 'a.dtd' [<!ATTLIST a x CDATA '&later;' y CDATA 'ok'><!ENTITY later 'v'>]>",
                                    "<!DOCTYPE a [<!ATTLIST a x CDATA '&later;' y CDATA 'ok'><!ENTITY later 'v'><!ENTITY % pe ''>%pe;]>" } )
  {
    xml_dtd< TyChar > dtdInvalid;
    ASSERT_EQ( StrReadDtd< TyChar >( pszDoctype, dtdInvalid ), "" ) << pszDoctype;
    EXPECT_EQ( dtdInvalid.m_rgstrValidityErrors, ( std::vector< std::string >{ "The default value of attribute [x] refers to an entity which is not declared." } ) );
    EXPECT_FALSE( dtdInvalid.PGetAttDef( W( "a" ), W( "x" ) )->m_fHasDefaultValue );
    EXPECT_TRUE( dtdInvalid.PGetAttDef( W( "a" ), W( "y" ) )->m_fHasDefaultValue );
  }
}

// The cursor adds the attributes which a tag doesn't have and which have a default value - 3.3.2.
TYPED_TEST( DtdTest, CursorAttributeDefaults )
{
  typedef TypeParam TyChar;
  {
    CursorDocument< TyChar > doc(
      "<!DOCTYPE doc [\n"
      "<!ENTITY co 'Bien &amp; Co'>\n"
      "<!ATTLIST doc lang NMTOKEN ' en ' by CDATA '&co;' version CDATA #FIXED '1.0' id ID #REQUIRED note CDATA #IMPLIED>\n"
      "<!ATTLIST item kind (a|b) 'a'>\n"
      "<!ATTLIST item extra CDATA 'x'>\n"
      "]>\n"
      "<doc id='d1'><item/><item kind='b' other='o'/><none/><item extra=\"e\"></item></doc>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    // What the tag has and then the defaults - in the order in which they are declared. Not those without a default.
    EXPECT_EQ( doc.StrAttributes(), "id=\"d1\" lang=\"en\" by=\"Bien & Co\" version=\"1.0\"" );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( doc.StrAttributes(), "kind=\"a\" extra=\"x\"" );
    ASSERT_TRUE( doc.m_cursor.FNextTag() );
    EXPECT_EQ( doc.StrAttributes(), "kind=\"b\" other=\"o\" extra=\"x\"" ); // What the tag has is what it has.
    ASSERT_TRUE( doc.m_cursor.FNextTag() );
    EXPECT_EQ( doc.StrAttributes(), "" ); // An element type for which no attributes are declared.
    ASSERT_TRUE( doc.m_cursor.FNextTag() );
    EXPECT_EQ( doc.StrAttributes(), "extra=\"e\" kind=\"a\"" );
  }
  // The document read into a tree: the attributes which were added are attributes like the others.
  {
    typedef typename CursorDocument< TyChar >::_TyXmlTraits TyXmlTraits;
    CursorDocument< TyChar > doc( "<!DOCTYPE doc [<!ATTLIST doc a CDATA 'default'><!ATTLIST item b CDATA 'B'>]><doc><item c='C'>text</item></doc>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    auto prXmlDoc = xml_document< TyXmlTraits >::PrCreateXmlDocument();
    EXPECT_NO_THROW( prXmlDoc.first->FromXmlStream( doc.m_cursor, prXmlDoc.second ) );
  }
  // Namespaces: an attribute with a default value may declare the namespace of the tag which doesn't have it.
  {
    CursorDocument< TyChar > doc(
      "<!DOCTYPE p:doc [\n"
      "<!ATTLIST p:doc xmlns:p CDATA #FIXED 'urn:p' p:a CDATA 'pa' plain CDATA 'v'>\n"
      "<!ATTLIST dflt xmlns CDATA 'urn:default'>\n"
      "]>\n"
      "<p:doc><dflt/><p:inner/></p:doc>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( doc.StrAttributes(), "xmlns:p=\"urn:p\" p:a=\"pa\" plain=\"v\"" );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( doc.StrAttributes(), "xmlns=\"urn:default\"" );
    ASSERT_TRUE( doc.m_cursor.FNextTag() ); // p:inner: the prefix is declared by the default of p:doc.
    EXPECT_EQ( doc.StrAttributes(), "" );
  }
  // The namespace which a default declares is declared for that tag only - as one written in the tag is.
  {
    CursorDocument< TyChar > doc( "<!DOCTYPE doc [<!ATTLIST p:a xmlns:p CDATA 'urn:p'>]><doc><p:a/><p:b/></doc>" );
    bool fError = !doc.FOpened();
    try
    {
      fError = fError || !doc.m_cursor.FMoveDown() || !doc.m_cursor.FMoveDown(); // p:a
      fError = fError || !doc.m_cursor.FNextTag(); // p:b - the prefix is not declared.
    }
    catch ( std::exception const & )
    {
      fError = true;
    }
    EXPECT_TRUE( fError );
  }
  // Without the default the prefix of the tag is not declared.
  {
    CursorDocument< TyChar > doc( "<!DOCTYPE p:doc [<!ATTLIST p:doc other CDATA 'v'>]><p:doc/>" );
    bool fError = !doc.FOpened();
    try
    {
      fError = fError || !doc.m_cursor.FMoveDown();
    }
    catch ( std::exception const & )
    {
      fError = true;
    }
    EXPECT_TRUE( fError );
  }
}

// The type of the writer which the tests use: it writes to memory in the character type of the document which is read.
template < class t_TyChar >
using TyTestXmlWriter = xml_writer< typename map_input_to_any_output_transport< _l_transport_fixedmem< t_TyChar >, t_TyChar, false_type >::_TyXmlWriteTransport >;
// What a writer has written to memory.
template < class t_TyChar, class t_TyXmlWriter >
std::basic_string< t_TyChar > StrWritten( t_TyXmlWriter & _rxwWriter )
{
  auto & rmsOut = _rxwWriter.GetTransportOut().GetMemStream();
  const size_t knbyWritten = (size_t)rmsOut.GetEndPos();
  std::basic_string< t_TyChar > str( knbyWritten / sizeof( t_TyChar ), 0 );
  rmsOut.Seek( 0, vkSeekBegin );
  rmsOut.Read( str.data(), knbyWritten );
  return str;
}
// The document as the writer writes it from the cursor, which is at the beginning of the document: in the character type
//  of the cursor and without a byte order mark.
// _fWriteDoctypeDecl: whether to write the document type declaration of the document.
// _nWriteDefaultedAttributes: whether to write the attributes which the DTD gave the tags: 1, 0 - or -1 for the default
//  of the writer, which is to write them unless the document type declaration was written.
template < class t_TyChar, bool t_kfSupportDTD >
std::basic_string< t_TyChar > StrWriteFromCursor( CursorDocument< t_TyChar, t_kfSupportDTD > & _rdoc, bool _fWriteDoctypeDecl = true, int _nWriteDefaultedAttributes = -1 )
{
  typedef TyTestXmlWriter< t_TyChar > _TyXmlWriter;
  _TyXmlWriter xwWriter;
  xwWriter.SetWriteBOM( false );
  xwWriter.SetWriteXMLDecl( !_rdoc.m_cursor.FPseudoXMLDecl() );
  xwWriter.SetWriteDoctypeDecl( _fWriteDoctypeDecl );
  xwWriter.SetUseNamespaces( _rdoc.m_cursor.FUseXMLNamespaces() );
  if ( _nWriteDefaultedAttributes >= 0 )
    xwWriter.SetWriteDefaultedAttributes( !!_nWriteDefaultedAttributes );
  typename _TyXmlWriter::_TyXMLDeclProperties xdp( _rdoc.m_cursor.GetXMLDeclProperties() );
  xwWriter.OpenMemFile( xdp, nullptr, true );
  xwWriter.NWriteFromReadCursor( _rdoc.m_cursor );
  return StrWritten< t_TyChar >( xwWriter );
}
// The value of an attribute of the current tag by its position - had without changing what the token holds.
template < class t_TyChar, bool t_kfSupportDTD >
std::basic_string< t_TyChar > StrAttrValue( CursorDocument< t_TyChar, t_kfSupportDTD > & _rdoc, size_t _nAttr )
{
  auto const & rltokTag = _rdoc.m_cursor.GetTagCur().GetLexToken();
  std::basic_string< t_TyChar > str;
  rltokTag.GetString( str, rltokTag.GetValue()[ vknAttributesIdx ][ _nAttr ][ vknAttr_ValueIdx ] );
  return str;
}

// The value of an attribute - 3.3.3: a tab or an end of line which is written in the value is a space in it, and an end
//  of line written as #xD #xA is one ( 2.11 ). A character reference to one of them is that character.
// What the document has is still what the token has: the writer writes the value as it was written.
template < class t_TyChar, bool t_kfSupportDTD >
void TestAttributeValueWhitespace()
{
  auto W = []( const char * _psz ) { return StrWiden< t_TyChar >( _psz ); };
  static const char s_szDocument[] =
    "<a x=\"one\ttwo\r\nthree\n\nfour\" y='\t&#9;b\r' z=\"\n\" plain=\"p q\" r='&#13;&#10;'>text\ttab\n<b w=\"\r\n\r\n\"/></a>";
  {
    CursorDocument< t_TyChar, t_kfSupportDTD > doc( s_szDocument );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_TRUE( StrAttrValue( doc, 0 ) == W( "one two three  four" ) );
    EXPECT_TRUE( StrAttrValue( doc, 1 ) == W( " \tb " ) ); // The reference is to a tab and that is what the value has.
    EXPECT_TRUE( StrAttrValue( doc, 2 ) == W( " " ) );
    EXPECT_TRUE( StrAttrValue( doc, 3 ) == W( "p q" ) );
    EXPECT_TRUE( StrAttrValue( doc, 4 ) == W( "\r\n" ) );
    EXPECT_TRUE( doc.StrContent() == W( "text\ttab\n" ) ); // Character data is not the value of an attribute.
    // In another character type, and a view where there is one: a value all in one piece which needs nothing done to it.
    {
      auto const & rltokTag = doc.m_cursor.GetTagCur().GetLexToken();
      auto const & rrgAttrs = rltokTag.GetValue()[ vknAttributesIdx ];
      std::u32string str32;
      rltokTag.GetString( str32, rrgAttrs[ 0 ][ vknAttr_ValueIdx ] );
      EXPECT_TRUE( str32 == U"one two three  four" );
      std::basic_string_view< t_TyChar > svPlain, svWhitespace;
      std::basic_string< t_TyChar > strPlain, strWhitespace;
      EXPECT_TRUE( rltokTag.FGetStringViewOrString( svPlain, strPlain, rrgAttrs[ 3 ][ vknAttr_ValueIdx ] ) );
      EXPECT_TRUE( svPlain == W( "p q" ) );
      EXPECT_FALSE( rltokTag.FGetStringViewOrString( svWhitespace, strWhitespace, rrgAttrs[ 2 ][ vknAttr_ValueIdx ] ) );
      EXPECT_TRUE( svWhitespace == W( " " ) );
    }
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_TRUE( StrAttrValue( doc, 0 ) == W( "  " ) );
  }
  // The document as it is written from the cursor is the document as it was read.
  {
    CursorDocument< t_TyChar, t_kfSupportDTD > doc( s_szDocument );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< t_TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
    EXPECT_TRUE( strWritten == W( s_szDocument ) ) << StrNarrow( strWritten );
  }
}
TYPED_TEST( DtdTest, AttributeValueWhitespace )
{
  TestAttributeValueWhitespace< TypeParam, false >();
  TestAttributeValueWhitespace< TypeParam, true >();
}

// The value of an attribute whose declared type is not CDATA - 3.3.3: the spaces at its ends are not in it and a
//  sequence of spaces within it is one space. The spaces are those of the value: what a tab which is written became, and
//  what a character reference to a space stands for.
// And still what the document has is what the token has: the writer writes the value as it was written.
TYPED_TEST( DtdTest, CursorTokenizedAttributeValues )
{
  typedef TypeParam TyChar;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  static const char s_szDoctype[] =
    "<!DOCTYPE doc [\n"
    "<!ATTLIST doc id ID #IMPLIED toks NMTOKENS #IMPLIED cd CDATA #IMPLIED refs IDREFS #IMPLIED e (a|b) #IMPLIED empty NMTOKENS #IMPLIED>\n"
    "]>";
  static const char s_szRoot[] =
    "<doc id='  d1 ' toks=\" a   b\tc \n\" cd='  x   y ' refs='&#32;r1  &#x20; r2&#32;' e=' a' empty='  ' other='  o  '><doc id=\"d2\"/></doc>";
  const std::string strDocument = std::string( s_szDoctype ) + s_szRoot;
  {
    CursorDocument< TyChar > doc( strDocument.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_TRUE( StrAttrValue( doc, 0 ) == W( "d1" ) );
    EXPECT_TRUE( StrAttrValue( doc, 1 ) == W( "a b c" ) );
    EXPECT_TRUE( StrAttrValue( doc, 2 ) == W( "  x   y " ) ); // CDATA: as it is.
    EXPECT_TRUE( StrAttrValue( doc, 3 ) == W( "r1 r2" ) );
    EXPECT_TRUE( StrAttrValue( doc, 4 ) == W( "a" ) );
    EXPECT_TRUE( StrAttrValue( doc, 5 ) == W( "" ) );
    EXPECT_TRUE( StrAttrValue( doc, 6 ) == W( "  o  " ) ); // Not declared: as CDATA.
    auto & rltokTag = doc.m_cursor.GetTagCur().GetLexToken();
    auto & rrgAttrs = rltokTag.GetValue()[ vknAttributesIdx ];
    // In another character type.
    {
      std::u32string str32;
      rltokTag.GetString( str32, rrgAttrs[ 1 ][ vknAttr_ValueIdx ] );
      EXPECT_TRUE( str32 == U"a b c" );
    }
    // A view of a value which is all in one piece of the document: the piece without the spaces at its ends.
    {
      std::basic_string_view< TyChar > svId, svE, svEmpty, svOr, svToks;
      std::basic_string< TyChar > strOr;
      rltokTag.KGetStringView( svId, rrgAttrs[ 0 ][ vknAttr_ValueIdx ] );
      rrgAttrs[ 4 ][ vknAttr_ValueIdx ].KGetStringView( rltokTag, svE );
      rltokTag.KGetStringView( svEmpty, rrgAttrs[ 5 ][ vknAttr_ValueIdx ] );
      EXPECT_TRUE( svId == W( "d1" ) );
      EXPECT_TRUE( svE == W( "a" ) );
      EXPECT_TRUE( svEmpty.empty() );
      EXPECT_TRUE( rltokTag.FGetStringViewOrString( svOr, strOr, rrgAttrs[ 0 ][ vknAttr_ValueIdx ] ) );
      EXPECT_TRUE( svOr == W( "d1" ) );
      EXPECT_TRUE( rrgAttrs[ 0 ][ vknAttr_ValueIdx ].FHasTypedData() ); // None of those changed the value.
      // There is no view of a value in more than one piece - as of one with a reference.
      EXPECT_ANY_THROW( rltokTag.KGetStringView( svToks, rrgAttrs[ 1 ][ vknAttr_ValueIdx ] ) );
    }
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_TRUE( StrAttrValue( doc, 0 ) == W( "d2" ) );
  }
  // The document as it is written from the cursor is the document as it was read.
  {
    CursorDocument< TyChar > doc( strDocument.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
    EXPECT_TRUE( strWritten == W( strDocument.c_str() ) ) << StrNarrow( strWritten );
  }
  // Without its document type declaration if the writer is told not to write that: then nothing says what the types of
  //  the attributes are to whoever reads what is written, so the values which the types had normalized are written as
  //  the values which they are. The others are written as they were.
  {
    CursorDocument< TyChar > doc( strDocument.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc, false ) );
    EXPECT_EQ( StrNarrow( strWritten ), "<doc id='d1' toks=\"a b c\" cd='  x   y ' refs='r1 r2' e='a' empty='' other='  o  '><doc id=\"d2\"/></doc>" );
  }
  // GetStringView() - which isn't const - leaves the string in the value in place of the pieces: then that is what the tag has.
  {
    CursorDocument< TyChar > doc( strDocument.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( doc.StrAttributes(), "id=\"d1\" toks=\"a b c\" cd=\"  x   y \" refs=\"r1 r2\" e=\"a\" empty=\"\" other=\"  o  \"" );
    EXPECT_EQ( doc.StrAttributes(), "id=\"d1\" toks=\"a b c\" cd=\"  x   y \" refs=\"r1 r2\" e=\"a\" empty=\"\" other=\"  o  \"" );
  }
}

// An attribute which the DTD gave a tag is known for what it is - its value is an xml_attr_default_value - until its
//  value is set to something else. The writer writes such attributes as any others unless it is told not to: then the
//  tags are written as they were read.
TYPED_TEST( DtdTest, CursorDefaultedAttributesAreKnown )
{
  typedef TypeParam TyChar;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  static const char s_szDoctype[] =
    "<!DOCTYPE doc [<!ENTITY co 'Bien &amp; Co'>"
    "<!ATTLIST doc by CDATA '&co;' lang NMTOKEN ' en ' xmlns:p CDATA #FIXED 'urn:p'><!ATTLIST p:item kind (a|b) \"a\">]>";
  static const char s_szRoot[] = "<doc lang=' fr'><p:item/><p:item kind=\"b\">text</p:item></doc>";
  const std::string strDocument = std::string( s_szDoctype ) + s_szRoot;
  {
    CursorDocument< TyChar > doc( strDocument.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    auto & rltokTag = doc.m_cursor.GetTagCur().GetLexToken();
    auto & rrgAttrs = rltokTag.GetValue()[ vknAttributesIdx ];
    ASSERT_EQ( rrgAttrs.GetSize(), 3u );
    EXPECT_FALSE( FIsAttrDefaultValue( rrgAttrs[ 0 ][ vknAttr_ValueIdx ] ) ); // lang: the tag has it.
    EXPECT_TRUE( FIsAttrDefaultValue( rrgAttrs[ 1 ][ vknAttr_ValueIdx ] ) ); // by
    EXPECT_TRUE( FIsAttrDefaultValue( rrgAttrs[ 2 ][ vknAttr_ValueIdx ] ) ); // xmlns:p
    // The value every way - and it is still what it was: nothing is kept in its place.
    EXPECT_TRUE( StrAttrValue( doc, 1 ) == W( "Bien & Co" ) );
    for ( int nRepeat = 0; nRepeat < 2; ++nRepeat )
    {
      std::basic_string_view< TyChar > svTok, svVal, svKTok, svOr;
      std::basic_string< TyChar > strOr;
      std::u32string str32, strOr32;
      std::u32string_view svOr32;
      rltokTag.GetStringView( svTok, rrgAttrs[ 1 ][ vknAttr_ValueIdx ] );
      rrgAttrs[ 1 ][ vknAttr_ValueIdx ].GetStringView( rltokTag, svVal );
      rltokTag.KGetStringView( svKTok, rrgAttrs[ 1 ][ vknAttr_ValueIdx ] );
      EXPECT_TRUE( rltokTag.FGetStringViewOrString( svOr, strOr, rrgAttrs[ 1 ][ vknAttr_ValueIdx ] ) );
      rltokTag.GetString( str32, rrgAttrs[ 1 ][ vknAttr_ValueIdx ] );
      EXPECT_EQ( rltokTag.FGetStringViewOrString( svOr32, strOr32, rrgAttrs[ 1 ][ vknAttr_ValueIdx ] ), sizeof( TyChar ) == sizeof( char32_t ) );
      EXPECT_TRUE( svTok == W( "Bien & Co" ) );
      EXPECT_TRUE( svVal == W( "Bien & Co" ) );
      EXPECT_TRUE( svKTok == W( "Bien & Co" ) );
      EXPECT_TRUE( svOr == W( "Bien & Co" ) );
      EXPECT_TRUE( str32 == U"Bien & Co" );
      EXPECT_TRUE( svOr32 == U"Bien & Co" );
      EXPECT_TRUE( FIsAttrDefaultValue( rrgAttrs[ 1 ][ vknAttr_ValueIdx ] ) );
    }
    EXPECT_EQ( doc.StrAttributes(), "lang=\"fr\" by=\"Bien & Co\" xmlns:p=\"urn:p\"" );
    // Set to something else it is no longer the DTD's.
    rrgAttrs[ 1 ][ vknAttr_ValueIdx ].emplaceVal( W( "someone else" ) );
    EXPECT_FALSE( FIsAttrDefaultValue( rrgAttrs[ 1 ][ vknAttr_ValueIdx ] ) );
    EXPECT_EQ( doc.StrAttributes(), "lang=\"fr\" by=\"someone else\" xmlns:p=\"urn:p\"" );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( doc.StrAttributes(), "kind=\"a\"" );
    EXPECT_TRUE( FIsAttrDefaultValue( doc.m_cursor.GetTagCur().GetLexToken().GetValue()[ vknAttributesIdx ][ 0 ][ vknAttr_ValueIdx ] ) );
    ASSERT_TRUE( doc.m_cursor.FNextTag() );
    EXPECT_EQ( doc.StrAttributes(), "kind=\"b\"" );
    EXPECT_FALSE( FIsAttrDefaultValue( doc.m_cursor.GetTagCur().GetLexToken().GetValue()[ vknAttributesIdx ][ 0 ][ vknAttr_ValueIdx ] ) );
  }
  // Written with its document type declaration - as it is unless the writer is told otherwise: the attributes which
  //  the DTD gave the tags are not written, since whoever reads the document has them from the declaration again. What
  //  is written is the document as it was read.
  {
    CursorDocument< TyChar > doc( strDocument.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
    EXPECT_EQ( StrNarrow( strWritten ), strDocument );
    // And read again it is the same document: the tags have the attributes from the DTD.
    const std::string strWrittenNarrow( StrNarrow( strWritten ) );
    CursorDocument< TyChar > docWritten( strWrittenNarrow.c_str() );
    ASSERT_TRUE( docWritten.FOpened() ) << docWritten.m_strError;
    ASSERT_TRUE( docWritten.m_cursor.FMoveDown() );
    EXPECT_EQ( docWritten.StrAttributes(), "lang=\"fr\" by=\"Bien & Co\" xmlns:p=\"urn:p\"" );
  }
  // Written without its document type declaration: the attributes which the DTD gave the tags are attributes of the
  //  tags - they are what the document means.
  {
    CursorDocument< TyChar > doc( strDocument.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc, false ) );
    EXPECT_EQ( StrNarrow( strWritten ), "<doc lang='fr' by=\"Bien &amp; Co\" xmlns:p=\"urn:p\"><p:item kind=\"a\"/><p:item kind=\"b\">text</p:item></doc>" );
  }
  // Either may be had whatever is done about the declaration: without it and without them, and with both.
  {
    CursorDocument< TyChar > doc( strDocument.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc, false, 0 ) );
    EXPECT_EQ( StrNarrow( strWritten ), "<doc lang='fr'><p:item/><p:item kind=\"b\">text</p:item></doc>" );
  }
  {
    CursorDocument< TyChar > doc( strDocument.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc, true, 1 ) );
    EXPECT_EQ( StrNarrow( strWritten ), std::string( s_szDoctype ) +
      "<doc lang=' fr' by=\"Bien &amp; Co\" xmlns:p=\"urn:p\"><p:item kind=\"a\"/><p:item kind=\"b\">text</p:item></doc>" );
  }
  // The value is written so that whoever reads it has the same value: the characters which can't be themselves between
  //  the quotes are references - and so are a tab and an end of line, which read as themselves would be spaces.
  {
    CursorDocument< TyChar > doc( "<!DOCTYPE doc [<!ATTLIST doc q CDATA 'say &quot;hi&quot; it&apos;s &#9;tab &lt; &amp; &#13;&#10;'>]><doc/>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc, false ) );
    EXPECT_EQ( StrNarrow( strWritten ), "<doc q=\"say &quot;hi&quot; it's &#9;tab &lt; &amp; &#13;&#10;\"/>" );
    // And read again without the DTD it is the value which the DTD gave.
    const std::string strWrittenNarrow( StrNarrow( strWritten ) );
    CursorDocument< TyChar, false > docWritten( strWrittenNarrow.c_str() );
    ASSERT_TRUE( docWritten.FOpened() ) << docWritten.m_strError;
    ASSERT_TRUE( docWritten.m_cursor.FMoveDown() );
    EXPECT_TRUE( StrAttrValue( docWritten, 0 ) == W( "say \"hi\" it's \ttab < & \r\n" ) );
  }
  // Read into a tree: the values there are copies of their own - they don't refer to the DTD of the cursor.
  {
    typedef typename CursorDocument< TyChar >::_TyXmlTraits TyXmlTraits;
    CursorDocument< TyChar > doc( strDocument.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    auto prXmlDoc = xml_document< TyXmlTraits >::PrCreateXmlDocument();
    ASSERT_NO_THROW( prXmlDoc.first->FromXmlStream( doc.m_cursor, prXmlDoc.second ) );
  }
}

// The document type declaration is one of the tokens of the prolog - where it was among the comments, the processing
//  instructions and the white space there - and its value is its text as it was. The writer writes that: a document
//  with a DTD which is written from the cursor is the document as it was read.
TYPED_TEST( DtdTest, DoctypeDeclIsWrittenAsItWasRead )
{
  typedef TypeParam TyChar;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  static const char s_szBefore[] =
    "<?xml version=\"1.0\"?>\n"
    "<!-- before -->\n";
  static const char s_szDoctype[] =
    "<!DOCTYPE doc SYSTEM 'doc.dtd' [\n"
    "  <!-- a comment in the subset -->\n"
    "  <!ENTITY % decls \"<!ELEMENT item EMPTY>\">\n"
    "  <!ELEMENT doc ( #PCDATA | item )* >\n"
    "  %decls;\n"
    "  <?pi in the subset?>\n"
    "  <!ATTLIST doc\n\ttitle CDATA #IMPLIED\n\tkind (a|b) 'a'\n\tids IDREFS #IMPLIED>\n"
    "  <!ENTITY company \"Bien &amp; Co\">\n"
    "]  >";
  static const char s_szAfter[] =
    "\n<?pi after?>\n"
    "<doc title=\"about\t&company;\" ids=' x1  x2 '>Made by &company;.<item/> &lt;</doc>\n"
    "<!-- after -->\n";
  const std::string strDocument = std::string( s_szBefore ) + s_szDoctype + s_szAfter;
  // The tokens of the prolog: one of them is the declaration.
  {
    CursorDocument< TyChar > doc( strDocument.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FHasDtd() );
    typedef typename CursorDocument< TyChar >::_TyXmlTraits::_TyXmlToken TyXmlToken;
    auto & rctxtProlog = doc.m_cursor.GetContextCur();
    size_t nDoctypeDecls = 0, nComments = 0, nProcessingInstructions = 0, nTokensBeforeDoctype = 0;
    std::basic_string< TyChar > strDoctypeText;
    rctxtProlog.ApplyContent( 0, rctxtProlog.NContentTokens(),
      [&]( const TyXmlToken * _pxtBegin, const TyXmlToken * _pxtEnd )
      {
        for ( const TyXmlToken * pxtCur = _pxtBegin; _pxtEnd != pxtCur; ++pxtCur )
        {
          if ( pxtCur->FIsDoctypeDecl() )
          {
            ++nDoctypeDecls;
            pxtCur->GetLexToken().GetString( strDoctypeText );
          }
          else
          {
            nComments += pxtCur->FIsComment();
            nProcessingInstructions += pxtCur->FIsProcessingInstruction();
            nTokensBeforeDoctype += !nDoctypeDecls;
          }
        }
      } );
    EXPECT_EQ( nDoctypeDecls, 1u );
    EXPECT_TRUE( strDoctypeText == W( s_szDoctype ) ) << StrNarrow( strDoctypeText );
    EXPECT_EQ( nComments, 1u ); // That of the prolog - not the one within the declaration.
    EXPECT_EQ( nProcessingInstructions, 1u );
    EXPECT_GE( nTokensBeforeDoctype, 1u ); // The comment at least: the declaration is where it was.
  }
  // Written from the cursor.
  {
    CursorDocument< TyChar > doc( strDocument.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
    EXPECT_TRUE( strWritten == W( strDocument.c_str() ) ) << StrNarrow( strWritten );
  }
  // A document without one is written as it always was.
  {
    const std::string strWithout = std::string( s_szBefore ) + "<doc a='1'>text</doc>\n";
    CursorDocument< TyChar > doc( strWithout.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
    EXPECT_TRUE( strWritten == W( strWithout.c_str() ) ) << StrNarrow( strWritten );
  }
  // The writer and a declaration which doesn't come from a cursor: its text is a string. It is written once, in the prolog.
  {
    typedef typename CursorDocument< TyChar >::_TyXmlTraits::_TyXmlToken TyXmlToken;
    CursorDocument< TyChar > doc( "<doc/>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    TyXmlToken xtDoctype( doc.m_xmlParser.GetUserObj(), s_knTokenDoctypeDecl );
    xtDoctype.GetValue().emplaceVal( W( "<!DOCTYPE doc [<!ATTLIST doc a CDATA 'v'>]>" ) );
    TyXmlToken xtNotOne( doc.m_xmlParser.GetUserObj(), s_knTokenDoctypeDecl );
    xtNotOne.GetValue().emplaceVal( W( "<!ELEMENT doc ANY>" ) );
    {
      TyTestXmlWriter< TyChar > xwWriter;
      xwWriter.SetWriteBOM( false );
      xwWriter.SetWriteXMLDecl( false );
      xwWriter.OpenMemFile();
      EXPECT_FALSE( xwWriter.FWroteDoctypeDecl() );
      EXPECT_TRUE( xwWriter.FWriteDefaultedAttributes() );
      EXPECT_ANY_THROW( xwWriter.WriteToken( xtNotOne ) );
      ASSERT_NO_THROW( xwWriter.WriteToken( xtDoctype ) );
      EXPECT_TRUE( xwWriter.FWroteDoctypeDecl() );
      EXPECT_FALSE( xwWriter.FWriteDefaultedAttributes() ); // By default they follow the declaration.
      EXPECT_ANY_THROW( xwWriter.WriteToken( xtDoctype ) ); // There may be only one.
      ASSERT_NO_THROW( xwWriter.NWriteFromReadCursor( doc.m_cursor ) );
      EXPECT_EQ( StrNarrow( StrWritten< TyChar >( xwWriter ) ), "<!DOCTYPE doc [<!ATTLIST doc a CDATA 'v'>]><doc/>" );
      EXPECT_ANY_THROW( xwWriter.WriteToken( xtDoctype ) ); // Not after the first tag.
    }
  }
}

// The writer and values which are strings - not the pieces of a document which was read: the value of an attribute or
//  character data which was set to a string, or which GetStringView() left as one. Such a string is the characters of
//  the value: what can't be written as itself is written as a reference - and whether an '&' in it begins a reference
//  which is to be written as one is as the writer or the tag is told ( EDetectReferences ).
// _edrDetectReferences: edrDetectReferencesCount for the default of the writer.
// Returns what was written for <doc a='...' b="..." c='3'>...</doc>: the value given is that of a and b and it is the
//  character data. c has its value from the document - by GetStringView(), which leaves the value as a string.
template < class t_TyChar >
std::basic_string< t_TyChar > StrWriteStringValues( const char * _pszValue, EDetectReferences _edrDetectReferences )
{
  typedef CursorDocument< t_TyChar, false > _TyCursorDocument;
  typedef typename _TyCursorDocument::_TyXmlTraits::_TyXmlToken _TyXmlToken;
  const std::basic_string< t_TyChar > strValue = StrWiden< t_TyChar >( _pszValue );
  _TyCursorDocument doc( "<doc a='1' b=\"2\" c='3'>text</doc>" );
  VerifyThrowSz( doc.FOpened(), "%s", doc.m_strError.c_str() );
  TyTestXmlWriter< t_TyChar > xwWriter;
  xwWriter.SetWriteBOM( false );
  xwWriter.SetWriteXMLDecl( false );
  if ( edrDetectReferencesCount != _edrDetectReferences )
    xwWriter.SetDefaultDetectReferences( _edrDetectReferences );
  xwWriter.OpenMemFile();
  VerifyThrow( doc.m_cursor.FMoveDown() );
  {
    auto & rxtTag = doc.m_cursor.GetTagCur();
    auto & rrgAttrs = rxtTag.GetLexToken().GetValue()[ vknAttributesIdx ];
    rrgAttrs[ 0 ][ vknAttr_ValueIdx ].emplaceVal( std::basic_string< t_TyChar >( strValue ) );
    rrgAttrs[ 1 ][ vknAttr_ValueIdx ].emplaceVal( std::basic_string< t_TyChar >( strValue ) );
    std::basic_string_view< t_TyChar > svC;
    rxtTag.GetLexToken().GetStringView( svC, rrgAttrs[ 2 ][ vknAttr_ValueIdx ] );
    VerifyThrow( !rrgAttrs[ 2 ][ vknAttr_ValueIdx ].FHasTypedData() );
    auto xwtDoc = xwWriter.StartTag( std::move( rxtTag ), true );
    xwtDoc.Commit();
    _TyXmlToken xtCharData( doc.m_xmlParser.GetUserObj(), s_knTokenCharData );
    xtCharData.GetValue().emplaceVal( std::basic_string< t_TyChar >( strValue ) );
    xwWriter.WriteToken( xtCharData );
  } // The tag is ended.
  return StrWritten< t_TyChar >( xwWriter );
}
// What was written read again: the values of a and b and the character data.
template < class t_TyChar >
void ExpectReadBack( std::basic_string< t_TyChar > const & _rstrWritten, const char * _pszValueA, const char * _pszValueB, const char * _pszContent )
{
  auto W = []( const char * _psz ) { return StrWiden< t_TyChar >( _psz ); };
  std::string strWritten; // What the tests write is ASCII - with the tabs and the ends of lines as they are.
  for ( t_TyChar ch : _rstrWritten )
    strWritten += ( ch < 0x7f ) ? char( ch ) : '?';
  CursorDocument< t_TyChar, false > doc( strWritten.c_str() );
  ASSERT_TRUE( doc.FOpened() ) << doc.m_strError << ": " << strWritten;
  ASSERT_TRUE( doc.m_cursor.FMoveDown() );
  EXPECT_TRUE( StrAttrValue( doc, 0 ) == W( _pszValueA ) ) << StrNarrow( StrAttrValue( doc, 0 ) );
  EXPECT_TRUE( StrAttrValue( doc, 1 ) == W( _pszValueB ) ) << StrNarrow( StrAttrValue( doc, 1 ) );
  EXPECT_TRUE( StrAttrValue( doc, 2 ) == W( "3" ) );
  EXPECT_TRUE( doc.StrContent() == W( _pszContent ) ) << StrNarrow( doc.StrContent() );
}

TYPED_TEST( DtdTest, WriterStringValues )
{
  typedef TypeParam TyChar;
  // Plain values.
  {
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteStringValues< TyChar >( "plain value", edrDetectReferencesCount ) );
    EXPECT_EQ( StrNarrow( strWritten ), "<doc a='plain value' b=\"plain value\" c='3'>plain value</doc>" );
    ExpectReadBack( strWritten, "plain value", "plain value", "plain value" );
  }
  // An empty value.
  {
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteStringValues< TyChar >( "", edrDetectReferencesCount ) );
    EXPECT_EQ( StrNarrow( strWritten ), "<doc a='' b=\"\" c='3'></doc>" );
  }
  // The characters which can't be written as themselves - and those which can be where they are: the other quote in
  //  the value of an attribute, and both of them and "]]>" where each may be.
  static const char s_szMarkup[] = "it's <\"quoted\"> ]]> x";
  {
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteStringValues< TyChar >( s_szMarkup, edrDetectReferencesCount ) );
    EXPECT_EQ( StrNarrow( strWritten ),
      "<doc a='it&apos;s &lt;\"quoted\"> ]]> x' b=\"it's &lt;&quot;quoted&quot;> ]]> x\" c='3'>it's &lt;\"quoted\"> ]]&gt; x</doc>" );
    ExpectReadBack( strWritten, s_szMarkup, s_szMarkup, s_szMarkup );
  }
  // At the ends of the value, and one after another.
  {
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteStringValues< TyChar >( "<<'\"&&]]>]]>", edrNoReferences ) );
    ExpectReadBack( strWritten, "<<'\"&&]]>]]>", "<<'\"&&]]>]]>", "<<'\"&&]]>]]>" );
  }
  // A tab and an end of line in the value of an attribute are written as references: read as themselves they would be spaces.
  // And a #xD in character data is: read as itself it would be the end of a line - #xA ( 2.11 ).
  {
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteStringValues< TyChar >( "a\tb\r\nc", edrDetectReferencesCount ) );
    EXPECT_TRUE( strWritten == StrWiden< TyChar >( "<doc a='a&#9;b&#13;&#10;c' b=\"a&#9;b&#13;&#10;c\" c='3'>a\tb&#13;\nc</doc>" ) ) << StrNarrow( strWritten );
    ExpectReadBack( strWritten, "a\tb\r\nc", "a\tb\r\nc", "a\tb\r\nc" );
  }
  // '&': with edrNoReferences every one of them is a character of the value - what is read is what was given.
  static const char s_szReferences[] = "A & B &amp; &#65;&#x42; &nosuch; &#1; &lt";
  {
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteStringValues< TyChar >( s_szReferences, edrNoReferences ) );
    EXPECT_EQ( StrNarrow( strWritten ),
      "<doc a='A &amp; B &amp;amp; &amp;#65;&amp;#x42; &amp;nosuch; &amp;#1; &amp;lt' b=\"A &amp; B &amp;amp; &amp;#65;&amp;#x42; &amp;nosuch; &amp;#1; &amp;lt\" c='3'>"
      "A &amp; B &amp;amp; &amp;#65;&amp;#x42; &amp;nosuch; &amp;#1; &amp;lt</doc>" );
    ExpectReadBack( strWritten, s_szReferences, s_szReferences, s_szReferences );
  }
  // By default ( edrAutoReferenceNoError ) what reads as a reference and is one - to an entity which is known or to a
  //  character - is written as it is, and any other '&' is a character of the value.
  {
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteStringValues< TyChar >( s_szReferences, edrDetectReferencesCount ) );
    EXPECT_EQ( StrNarrow( strWritten ),
      "<doc a='A &amp; B &amp; &#65;&#x42; &amp;nosuch; &amp;#1; &amp;lt' b=\"A &amp; B &amp; &#65;&#x42; &amp;nosuch; &amp;#1; &amp;lt\" c='3'>"
      "A &amp; B &amp; &#65;&#x42; &amp;nosuch; &amp;#1; &amp;lt</doc>" );
    ExpectReadBack( strWritten, "A & B & AB &nosuch; &#1; &lt", "A & B & AB &nosuch; &#1; &lt", "A & B & AB &nosuch; &#1; &lt" );
  }
  // edrAutoReference: what reads as a reference and isn't one is an error - an '&' which doesn't read as one is still a character.
  {
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteStringValues< TyChar >( "A & B &amp; &#65; &lt", edrAutoReference ) );
    ExpectReadBack( strWritten, "A & B & A &lt", "A & B & A &lt", "A & B & A &lt" );
    EXPECT_ANY_THROW( StrWriteStringValues< TyChar >( "&nosuch;", edrAutoReference ) );
    EXPECT_ANY_THROW( StrWriteStringValues< TyChar >( "&#1;", edrAutoReference ) );
    EXPECT_ANY_THROW( StrWriteStringValues< TyChar >( "&#x110000;", edrAutoReference ) );
  }
  // edrAlwaysReference: every '&' must begin a reference.
  {
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteStringValues< TyChar >( "&amp;&lt;&#65;", edrAlwaysReference ) );
    EXPECT_EQ( StrNarrow( strWritten ), "<doc a='&amp;&lt;&#65;' b=\"&amp;&lt;&#65;\" c='3'>&amp;&lt;&#65;</doc>" );
    EXPECT_ANY_THROW( StrWriteStringValues< TyChar >( "A & B", edrAlwaysReference ) );
    EXPECT_ANY_THROW( StrWriteStringValues< TyChar >( "&nosuch;", edrAlwaysReference ) );
  }
}

// Whether two documents are the same document however each is written - FCompareXmlDocuments() of xml_compare.h.
// The documents are read by cursors of the character types and with or without support for the DTD as given.
// Returns the difference: empty if they are the same.
template < class t_TyCharLeft, bool t_kfSupportDTDLeft, class t_TyCharRight, bool t_kfSupportDTDRight >
std::string StrDifference( const char * _pszLeft, const char * _pszRight, xml_compare_options const & _roptions = xml_compare_options() )
{
  CursorDocument< t_TyCharLeft, t_kfSupportDTDLeft > docLeft( _pszLeft );
  if ( !docLeft.FOpened() )
    return "left wasn't opened: " + docLeft.m_strError;
  CursorDocument< t_TyCharRight, t_kfSupportDTDRight > docRight( _pszRight );
  if ( !docRight.FOpened() )
    return "right wasn't opened: " + docRight.m_strError;
  try
  {
    std::string strDifference;
    if ( FCompareXmlDocuments( docLeft.m_cursor, docRight.m_cursor, _roptions, &strDifference ) )
      return std::string();
    return strDifference.empty() ? std::string( "different - and nothing says how" ) : strDifference;
  }
  catch ( std::exception const & rexc )
  {
    return std::string( "exception: " ) + rexc.what();
  }
}
// A string with the tabs and the ends of lines which it has - StrNarrow() has '?' for those.
template < class t_TyChar >
std::string StrNarrowExact( std::basic_string< t_TyChar > const & _rstr )
{
  std::string str;
  for ( t_TyChar ch : _rstr )
    str += ( ch < 0x7f ) ? char( ch ) : '?';
  return str;
}

TYPED_TEST( DtdTest, CompareDocuments )
{
  typedef TypeParam TyChar;
  // The other document is read in another character type: what is compared doesn't depend on that.
  typedef std::conditional_t< sizeof( TyChar ) == sizeof( char8_t ), char32_t, char8_t > TyCharOther;
  auto Difference = []( const char * _pszLeft, const char * _pszRight, xml_compare_options const & _roptions = xml_compare_options() )
  {
    return StrDifference< TyChar, false, TyCharOther, false >( _pszLeft, _pszRight, _roptions );
  };
  // The same however they are written: the order of the attributes and their quotes, references and CDATA sections,
  //  an empty element, the XML declaration and the white space outside of the document's tag.
  EXPECT_EQ( Difference( "<a x='1' y=\"2\">t<b/></a>", "<?xml version=\"1.0\"?>\n<a y='2' x=\"1\">t<b></b></a>\n" ), "" );
  EXPECT_EQ( Difference( "<a v='x&amp;y&#65;&lt;'>x&amp;y&#65;<![CDATA[<]]>z</a>", "<a v='x&#38;yA&#60;'>x&#38;yA&lt;<![CDATA[z]]></a>" ), "" );
  EXPECT_EQ( Difference( "<a v='p\tq\r\nr'/>", "<a v='p q r'/>" ), "" ); // The value of an attribute is normalized.
  // What is different - and where and how is said.
  EXPECT_EQ( Difference( "<a><b x='1'/></a>", "<a><b x='2'/></a>" ), "/a/b: the value of the attribute [x] is [1] in left and [2] in right" );
  EXPECT_EQ( Difference( "<a><b x='1' y='2'/></a>", "<a><b y='2'/></a>" ), "/a/b: left has the attribute [x] and right doesn't" );
  EXPECT_EQ( Difference( "<a><b y='2'/></a>", "<a><b x='1' y='2'/></a>" ), "/a/b: right has the attribute [x] and left doesn't" );
  EXPECT_EQ( Difference( "<a><b y='2'/></a>", "<a><b y='2' z='3'/></a>" ), "/a/b: right has the attribute [z] and left doesn't" );
  EXPECT_EQ( Difference( "<a><b/></a>", "<a><c/></a>" ), "/a: the tag is <b> in left and <c> in right" );
  EXPECT_EQ( Difference( "<a><b/><c/></a>", "<a><b/></a>" ), "/a: left has a start tag <c> where right has an end tag" );
  EXPECT_EQ( Difference( "<a><b/></a>", "<a><b><c/></b></a>" ), "/a/b: left has an end tag where right has a start tag <c>" );
  EXPECT_EQ( Difference( "<a>one<b/>two</a>", "<a>one<b/>too</a>" ), "/a: the character data is [two] in left and [too] in right" );
  EXPECT_EQ( Difference( "<a>one<b/></a>", "<a><b/>one</a>" ), "/a: left has character data [one] where right has a start tag <b>" );
  EXPECT_EQ( Difference( "<a/>", "<b/>" ), "/: the tag is <a> in left and <b> in right" );
  // The order of the attributes if it is to be compared.
  {
    xml_compare_options options;
    options.m_fCompareAttributeOrder = true;
    EXPECT_EQ( Difference( "<a x='1' y='2'/>", "<a x='1' y='2'/>", options ), "" );
    EXPECT_EQ( Difference( "<a x='1' y='2'/>", "<a y='2' x='1'/>", options ), "/a: attribute 1 is [x] in left and [y] in right" );
  }
  // Comments and processing instructions - or not. Without them the character data on either side of one is one piece.
  EXPECT_EQ( Difference( "<!--c--><a>x<!--d-->y<?pi z?></a>", "<!--c--><a>x<!--d-->y<?pi z?></a>" ), "" );
  EXPECT_EQ( Difference( "<a>x<!--d-->y</a>", "<a>xy</a>" ), "/a: the character data is [x] in left and [xy] in right" );
  EXPECT_EQ( Difference( "<a><!--d-->y</a>", "<a>y</a>" ), "/a: left has a comment [d] where right has character data [y]" );
  EXPECT_EQ( Difference( "<a><!--d--></a>", "<a><!--e--></a>" ), "/a: the comment is [d] in left and [e] in right" );
  EXPECT_EQ( Difference( "<a><?pi z?></a>", "<a><?pi y?></a>" ), "/a: the processing instruction is [pi z] in left and [pi y] in right" );
  {
    xml_compare_options options;
    options.m_fCompareComments = false;
    options.m_fCompareProcessingInstructions = false;
    EXPECT_EQ( Difference( "<!--c--><a>x<!--d-->y<?pi z?></a><?after?>", "<a>xy</a>", options ), "" );
    EXPECT_EQ( Difference( "<a>x<!--d-->y</a>", "<a>xz</a>", options ), "/a: the character data is [xy] in left and [xz] in right" );
  }
  // White space is character data - unless character data which is all white space is to be ignored.
  EXPECT_NE( Difference( "<a> <b/>\n</a>", "<a><b/></a>" ), "" );
  {
    xml_compare_options options;
    options.m_fIgnoreWhitespaceText = true;
    EXPECT_EQ( Difference( "<a> <b/>\n</a>", "<a><b/></a>", options ), "" );
    EXPECT_NE( Difference( "<a> x <b/></a>", "<a>x<b/></a>", options ), "" );
  }
  // Namespaces: names are the same if their namespaces are and their local parts are - and their prefixes, unless those
  //  are not to be compared. Then the declarations of the namespaces must not be either.
  EXPECT_EQ( Difference( "<p:a xmlns:p='urn:u' p:x='1'><p:b/><c xmlns='urn:d'/></p:a>", "<p:a p:x='1' xmlns:p='urn:u'><p:b/><c xmlns='urn:d'/></p:a>" ), "" );
  EXPECT_NE( Difference( "<p:a xmlns:p='urn:u'><p:b/></p:a>", "<q:a xmlns:q='urn:u'><q:b/></q:a>" ), "" );
  EXPECT_NE( Difference( "<p:a xmlns:p='urn:u'/>", "<p:a xmlns:p='urn:v'/>" ), "" );
  {
    xml_compare_options options;
    options.m_fComparePrefixes = false;
    options.m_fCompareNamespaceDeclarations = false;
    EXPECT_EQ( Difference( "<p:a xmlns:p='urn:u' p:x='1'><p:b/></p:a>", "<q:a xmlns:q='urn:u' q:x='1'><q:b/></q:a>", options ), "" );
    EXPECT_EQ( Difference( "<p:a xmlns:p='urn:u'><p:b/></p:a>", "<a xmlns='urn:u'><b/></a>", options ), "" );
    EXPECT_NE( Difference( "<p:a xmlns:p='urn:u'/>", "<p:a xmlns:p='urn:v'/>", options ), "" ); // Another namespace is another name.
  }

  // A document which has a DTD - which gives its tags attributes, declares the types of some of their attributes and
  //  declares entities - and the same document written without the DTD: the attributes are in the tags, the values are
  //  as the types made them and the references to the entities are the text of the entities. Read without any DTD the
  //  second is the same document as the first read with its DTD.
  static const char s_szDoctype[] =
    "<!DOCTYPE doc [\n"
    "<!ENTITY co 'Bien &amp; Co'>\n"
    "<!ATTLIST doc by CDATA '&co;' lang NMTOKEN ' en ' ids IDREFS #IMPLIED title CDATA #IMPLIED xmlns:p CDATA #FIXED 'urn:p'>\n"
    "<!ATTLIST p:item kind (a|b) \"a\">\n"
    "]>\n";
  static const char s_szRoot[] =
    "<doc ids=' x1   x2 ' title='About &co; &lt;here&gt;'>By &co;, <![CDATA[<raw>]]>&#65;<p:item/><!-- c -->\n"
    "<p:item kind=\" b\">text</p:item><?pi data?></doc>\n";
  const std::string strWithDtd = std::string( s_szDoctype ) + s_szRoot;
  std::string strWithoutDtd;
  {
    CursorDocument< TyChar > doc( strWithDtd.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc, false ) );
    strWithoutDtd = StrNarrowExact( strWritten );
  }
  EXPECT_EQ( strWithoutDtd,
    "\n<doc ids='x1 x2' title='About Bien &amp; Co &lt;here&gt;' by=\"Bien &amp; Co\" lang=\"en\" xmlns:p=\"urn:p\">By Bien &amp; Co, <![CDATA[<raw>]]>&#65;<p:item kind=\"a\"/><!-- c -->\n"
    "<p:item kind=\"b\">text</p:item><?pi data?></doc>\n" );
  EXPECT_EQ( ( StrDifference< TyChar, true, TyCharOther, false >( strWithDtd.c_str(), strWithoutDtd.c_str() ) ), "" );
  EXPECT_EQ( ( StrDifference< TyCharOther, false, TyChar, true >( strWithoutDtd.c_str(), strWithDtd.c_str() ) ), "" );
  // And written with the DTD it is the document as it was - which is the same document of course.
  {
    CursorDocument< TyChar > doc( strWithDtd.c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
    EXPECT_EQ( StrNarrowExact( strWritten ), strWithDtd );
    EXPECT_EQ( ( StrDifference< TyChar, true, TyCharOther, true >( strWithDtd.c_str(), StrNarrowExact( strWritten ).c_str() ) ), "" );
  }
  // Read without what the DTD does it is another document: the document without its declaration isn't well formed -
  //  it refers to an entity which nothing declares - and without that the tags don't have the attributes.
  EXPECT_NE( ( StrDifference< TyChar, true, TyCharOther, false >( strWithDtd.c_str(), s_szRoot ) ), "" );
  EXPECT_EQ( ( StrDifference< TyChar, true, TyCharOther, false >( "<!DOCTYPE doc [<!ATTLIST doc a CDATA 'v'>]><doc/>", "<doc/>" ) ),
             "/doc: left has the attribute [a] and right doesn't" );
  EXPECT_EQ( ( StrDifference< TyChar, true, TyCharOther, false >( "<!DOCTYPE doc [<!ATTLIST doc a CDATA 'v'>]><doc/>", "<doc a='v'/>" ) ), "" );
  // The declaration itself is compared - as text - only if that is asked for.
  {
    xml_compare_options options;
    options.m_fCompareDoctypeDecl = true;
    EXPECT_EQ( ( StrDifference< TyChar, true, TyCharOther, true >( strWithDtd.c_str(), strWithDtd.c_str(), options ) ), "" );
    EXPECT_NE( ( StrDifference< TyChar, true, TyCharOther, false >( strWithDtd.c_str(), strWithoutDtd.c_str(), options ) ), "" );
  }
}

// Validation: a document read with XML traits which are validating - xml_traits< transport, true, true > - is checked
//  against its DTD as it is read: see xml_validate.h.
// Returns what is not valid about the document - each violation on a line, those of the declarations first - or that
//  it couldn't be read.
// _fMoveThroughIt: the cursor is moved to every tag of the document. Otherwise it is moved to the document's element
//  and then beyond it - which reads all that is within it as well.
template < class t_TyChar >
std::string StrValidate( const char * _pszDocument, bool _fMoveThroughIt = false )
{
  CursorDocument< t_TyChar, true, true > doc( _pszDocument );
  if ( !doc.FOpened() )
    return "not opened: " + doc.m_strError;
  try
  {
    if ( _fMoveThroughIt )
    {
      for ( size_t nDepth = 0; ; )
      {
        if ( doc.m_cursor.FMoveDown() )
        {
          ++nDepth;
          continue;
        }
        if ( !nDepth )
          break;
        if ( !doc.m_cursor.FNextTag() )
          --nDepth;
      }
    }
    else
    {
      if ( doc.m_cursor.FMoveDown() )
        (void)doc.m_cursor.FNextTag();
    }
  }
  catch ( std::exception const & rexc )
  {
    return std::string( "exception: " ) + rexc.what();
  }
  std::string strErrors;
  if ( doc.m_cursor.FHasDtd() )
    for ( std::string const & rstrError : doc.m_cursor.GetDtd().m_rgstrValidityErrors )
      strErrors += rstrError + "\n";
  for ( std::string const & rstrError : doc.m_cursor.RgValidityErrors() )
    strErrors += rstrError + "\n";
  if ( doc.m_cursor.FIsValid() != strErrors.empty() )
    strErrors += "FIsValid() doesn't agree with the violations\n";
  return strErrors;
}

TYPED_TEST( DtdTest, Validation )
{
  typedef TypeParam TyChar;
  static const char s_szDoctype[] =
    "<!DOCTYPE doc [\n"
    "<!ELEMENT doc (head, (item | group)*, tail?)>\n"
    "<!ELEMENT head (#PCDATA)>\n"
    "<!ELEMENT item EMPTY>\n"
    "<!ELEMENT group (item+)>\n"
    "<!ELEMENT tail (#PCDATA | b)*>\n"
    "<!ELEMENT b ANY>\n"
    "<!ATTLIST item id ID #REQUIRED ref IDREF #IMPLIED kind (a|b) 'a' refs IDREFS #IMPLIED tok NMTOKEN #IMPLIED toks NMTOKENS #IMPLIED fixed CDATA #FIXED 'f'>\n"
    "<!NOTATION gif SYSTEM 'gif'>\n"
    "<!ENTITY pic SYSTEM 'pic.gif' NDATA gif>\n"
    "<!ENTITY text 'some text'>\n"
    "<!ENTITY ws ' '>\n"
    "<!ATTLIST b src ENTITY #IMPLIED srcs ENTITIES #IMPLIED note NOTATION (gif) #IMPLIED>\n"
    "]>\n";
  // Each of these is the document's element and what is within it, for the declarations above. Returns the violations.
  auto Validate = []( const char * _pszRoot, bool _fMoveThroughIt = false )
  {
    return StrValidate< TyChar >( ( std::string( s_szDoctype ) + _pszRoot ).c_str(), _fMoveThroughIt );
  };
  // A valid document.
  static const char s_szValid[] =
    "<doc><head>title &text;</head>\n"
    " <item id='i1'/><!-- c --><?pi?>&ws;<group><item id='i2' ref='i1' refs=' i1  i3 ' kind=' b' tok='1.5' toks='a 2 -c' fixed='f'/></group><item id=\"i3\"/>\n"
    " <tail>text <b src='pic' srcs='pic pic' note='gif'>any <item id='i4'/><head/> thing</b> more <![CDATA[<x>]]>&#65;</tail></doc>\n";
  EXPECT_EQ( Validate( s_szValid ), "" );
  EXPECT_EQ( Validate( s_szValid, true ), "" );
  EXPECT_EQ( Validate( "<doc><head/></doc>" ), "" ); // The least which the content model allows.
  EXPECT_EQ( Validate( "<doc><head></head><group><item id='a'/><item id='b'></item></group><tail/></doc>" ), "" );

  // VC: Root Element Type.
  EXPECT_EQ( Validate( "<head>text</head>" ), "The document's element is <head> and its document type declaration is for <doc>.\n" );
  // VC: Element Valid - the element type is declared. Nothing is known of what may be within such an element.
  EXPECT_EQ( Validate( "<doc><head/><tail><nosuch><item id='a'/></nosuch></tail></doc>" ),
    "The element <nosuch> may not be within the element <tail>: it is not among the element types of its mixed content.\n"
    "The element type <nosuch> is not declared.\n" );
  // Element content: the elements in the order of the content model.
  EXPECT_EQ( Validate( "<doc><item id='a'/></doc>" ), "The element <item> may not be here within the element <doc>: its content model has <head>.\n" );
  EXPECT_EQ( Validate( "<doc></doc>" ), "The content of the element <doc> ends where its content model has more: <head>.\n" );
  EXPECT_EQ( Validate( "<doc><head/><tail/><item id='a'/></doc>" ), "The element <item> may not be here within the element <doc>: its content model has the end of the element.\n" );
  EXPECT_EQ( Validate( "<doc><head/><head/></doc>" ), "The element <head> may not be here within the element <doc>: its content model has <group>, <item>, <tail> or the end of the element.\n" );
  EXPECT_EQ( Validate( "<doc><head/><group/></doc>" ), "The content of the element <group> ends where its content model has more: <item>.\n" );
  // Among the elements of element content: white space, comments and processing instructions - not character data,
  //  nor a CDATA section or a reference to a character even if what they have is white space.
  EXPECT_EQ( Validate( "<doc><head/>text</doc>" ), "The element <doc> is declared to have element content and has character data.\n" );
  EXPECT_EQ( Validate( "<doc><head/><![CDATA[ ]]></doc>" ), "The element <doc> is declared to have element content and has character data.\n" );
  EXPECT_EQ( Validate( "<doc><head/>&#32;</doc>" ), "The element <doc> is declared to have element content and has character data.\n" );
  EXPECT_EQ( Validate( "<doc><head/>&text;</doc>" ), "The element <doc> is declared to have element content and has character data.\n" );
  // EMPTY: nothing at all.
  EXPECT_EQ( Validate( "<doc><head/><item id='a'> </item></doc>" ), "The element <item> is declared EMPTY and has white space.\n" );
  EXPECT_EQ( Validate( "<doc><head/><item id='a'>x</item></doc>" ), "The element <item> is declared EMPTY and has character data.\n" );
  EXPECT_EQ( Validate( "<doc><head/><item id='a'><!-- c --></item></doc>" ), "The element <item> is declared EMPTY and has a comment or a processing instruction.\n" );
  EXPECT_EQ( Validate( "<doc><head/><item id='a'><head/></item></doc>" ), "The element <item> is declared EMPTY and has the element <head>.\n" );
  // Mixed content: character data and the element types which are listed - each one which isn't is said.
  EXPECT_EQ( Validate( "<doc><head>a<b/></head></doc>" ), "The element <b> may not be within the element <head>: it is not among the element types of its mixed content.\n" );
  EXPECT_EQ( Validate( "<doc><head/><tail><item id='a'/>x<head/></tail></doc>" ),
    "The element <item> may not be within the element <tail>: it is not among the element types of its mixed content.\n"
    "The element <head> may not be within the element <tail>: it is not among the element types of its mixed content.\n" );

  // VC: Attribute Value Type - the attribute is declared.
  EXPECT_EQ( Validate( "<doc other='x'><head/></doc>" ), "The attribute [other] of the element <doc> is not declared.\n" );
  EXPECT_EQ( Validate( "<doc><head/><item id='a' xmlns:p='urn:p'/></doc>" ), "The attribute [xmlns:p] of the element <item> is not declared.\n" );
  // VC: Required Attribute.
  EXPECT_EQ( Validate( "<doc><head/><item/></doc>" ), "The attribute [id] of the element <item> is declared #REQUIRED and the tag doesn't have it.\n" );
  // VC: ID.
  EXPECT_EQ( Validate( "<doc><head/><item id='1a'/></doc>" ), "The value [1a] of the attribute [id] of the element <item> is not a Name.\n" );
  EXPECT_EQ( Validate( "<doc><head/><item id='a b'/></doc>" ), "The value [a b] of the attribute [id] of the element <item> is not a Name.\n" );
  EXPECT_EQ( Validate( "<doc><head/><item id='a'/><item id=' a '/></doc>" ), "The ID [a] - of the attribute [id] of an element <item> - is the ID of another element as well.\n" );
  // VC: IDREF - an ID of the document, which may come after the reference.
  EXPECT_EQ( Validate( "<doc><head/><item id='a' ref='b'/><item id='b' refs='a b'/></doc>" ), "" );
  EXPECT_EQ( Validate( "<doc><head/><item id='a' ref='b'/></doc>" ), "The IDREF [b] is not the ID of any element of the document.\n" );
  EXPECT_EQ( Validate( "<doc><head/><item id='a' refs='a c  d'/></doc>" ),
    "The IDREF [c] is not the ID of any element of the document.\nThe IDREF [d] is not the ID of any element of the document.\n" );
  EXPECT_EQ( Validate( "<doc><head/><item id='a' refs=''/></doc>" ), "The value [] of the attribute [refs] of the element <item> is not Names.\n" );
  // VC: Enumeration.
  EXPECT_EQ( Validate( "<doc><head/><item id='a' kind='c'/></doc>" ), "The value [c] of the attribute [kind] of the element <item> is not one of the values declared for it.\n" );
  // VC: Name Token.
  EXPECT_EQ( Validate( "<doc><head/><item id='a' tok='a b'/></doc>" ), "The value [a b] of the attribute [tok] of the element <item> is not a name token.\n" );
  EXPECT_EQ( Validate( "<doc><head/><item id='a' toks='a b!'/></doc>" ), "The value [a b!] of the attribute [toks] of the element <item> is not name tokens.\n" );
  // VC: Fixed Attribute Default.
  EXPECT_EQ( Validate( "<doc><head/><item id='a' fixed='g'/></doc>" ), "The attribute [fixed] of the element <item> is declared #FIXED as [f] and its value is [g].\n" );
  // VC: Entity Name.
  EXPECT_EQ( Validate( "<doc><head/><tail><b src='text'/></tail></doc>" ), "The value [text] of the attribute [src] of the element <b> is not the name of an unparsed entity which is declared.\n" );
  EXPECT_EQ( Validate( "<doc><head/><tail><b srcs='pic nosuch'/></tail></doc>" ), "The value [nosuch] of the attribute [srcs] of the element <b> is not the name of an unparsed entity which is declared.\n" );
  // VC: Notation Attributes.
  EXPECT_EQ( Validate( "<doc><head/><tail><b note='jpeg'/></tail></doc>" ), "The value [jpeg] of the attribute [note] of the element <b> is not one of the notations declared for it.\n" );

  // What the cursor passes over is checked as what it stops at is: the same violations either way.
  static const char s_szSeveral[] = "<doc><head/><group><item/><item id='x' kind='z'/></group><tail><b><nosuch/></b></tail><item id='x'/></doc>";
  EXPECT_EQ( Validate( s_szSeveral ),
    "The attribute [id] of the element <item> is declared #REQUIRED and the tag doesn't have it.\n"
    "The value [z] of the attribute [kind] of the element <item> is not one of the values declared for it.\n"
    "The element type <nosuch> is not declared.\n"
    "The element <item> may not be here within the element <doc>: its content model has the end of the element.\n"
    "The ID [x] - of the attribute [id] of an element <item> - is the ID of another element as well.\n" );
  EXPECT_EQ( Validate( s_szSeveral, true ), Validate( s_szSeveral ) );

  // Content models: what may follow what - and a model which doesn't say which way it is going until later.
  {
    static const char s_szModels[] =
      "<!DOCTYPE x [\n"
      "<!ELEMENT x (((a, b) | (a, c)), (d?, e*)+, (f | (g, h))?)>\n"
      "<!ELEMENT a EMPTY><!ELEMENT b EMPTY><!ELEMENT c EMPTY><!ELEMENT d EMPTY><!ELEMENT e EMPTY><!ELEMENT f EMPTY><!ELEMENT g EMPTY><!ELEMENT h EMPTY>\n"
      "]>\n";
    auto ValidateModel = []( const char * _pszRoot )
    {
      return StrValidate< TyChar >( ( std::string( s_szModels ) + _pszRoot ).c_str() );
    };
    EXPECT_EQ( ValidateModel( "<x><a/><b/></x>" ), "" );
    EXPECT_EQ( ValidateModel( "<x><a/><c/></x>" ), "" );
    EXPECT_EQ( ValidateModel( "<x><a/><c/><d/><e/><e/><d/><d/><e/><g/><h/></x>" ), "" );
    EXPECT_EQ( ValidateModel( "<x><a/><b/><f/></x>" ), "" );
    EXPECT_EQ( ValidateModel( "<x><a/></x>" ), "The content of the element <x> ends where its content model has more: <b>, <c>.\n" );
    EXPECT_EQ( ValidateModel( "<x><a/><d/></x>" ), "The element <d> may not be here within the element <x>: its content model has <b>, <c>.\n" );
    EXPECT_EQ( ValidateModel( "<x><a/><b/><g/></x>" ), "The content of the element <x> ends where its content model has more: <h>.\n" );
    EXPECT_EQ( ValidateModel( "<x><a/><b/><f/><f/></x>" ), "The element <f> may not be here within the element <x>: its content model has the end of the element.\n" );
  }
  // A document without a document type declaration has nothing which it is valid against.
  EXPECT_EQ( StrValidate< TyChar >( "<doc><a/></doc>" ), "The document has no document type declaration: there is nothing which it is valid against.\n" );
  // The declarations themselves may not be valid: then the document isn't.
  EXPECT_NE( StrValidate< TyChar >( "<!DOCTYPE doc [<!ELEMENT doc EMPTY><!ELEMENT doc EMPTY>]><doc/>" ), "" );
  // A violation is thrown if that is asked for.
  {
    CursorDocument< TyChar, true, true > doc( ( std::string( s_szDoctype ) + "<doc><head/><item/></doc>" ).c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    doc.m_cursor.SetThrowOnValidityError();
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_ANY_THROW( doc.m_cursor.FNextTag() );
  }
  // And a document which is valid is read by a validating cursor as by any other: it is the same document.
  {
    const std::string strDocument = std::string( s_szDoctype ) + s_szValid;
    CursorDocument< TyChar, true, true > docValidating( strDocument.c_str() );
    CursorDocument< TyChar, true, false > docNot( strDocument.c_str() );
    ASSERT_TRUE( docValidating.FOpened() && docNot.FOpened() );
    std::string strDifference;
    EXPECT_TRUE( FCompareXmlDocuments( docValidating.m_cursor, docNot.m_cursor, xml_compare_options(), &strDifference ) ) << strDifference;
    EXPECT_TRUE( docValidating.m_cursor.FIsValid() );
  }
}

// External entities - the external subset of the DTD and its external parameter entities: they are read if the parser
//  has a resolver for them.
// The entities of a test: the text of each by its system identifier. What is asked for is recorded: each system
//  identifier with the base which it was asked with. The location of an entity is its system identifier.
struct TestEntities
{
  std::map< std::string, std::string > m_mapEntities;
  std::vector< std::string > m_rgstrAsked; // "system identifier <- base"

  template < class t_TyChar >
  xml_external_entity_resolver< t_TyChar > Resolver()
  {
    return [this]( xml_dtd_external_id< t_TyChar > const & _rexternalId, std::basic_string_view< t_TyChar > _svBase,
                   std::basic_string< t_TyChar > & _rstrText, std::basic_string< t_TyChar > & _rstrLocation ) -> bool
    {
      const std::string strSystem = _rexternalId.m_optstrSystem.has_value() ? StrNarrow( *_rexternalId.m_optstrSystem ) : std::string();
      m_rgstrAsked.push_back( strSystem + " <- " + StrNarrow( _svBase ) );
      std::map< std::string, std::string >::const_iterator cit = m_mapEntities.find( strSystem );
      if ( m_mapEntities.end() == cit )
        return false;
      _rstrText.assign( cit->second.begin(), cit->second.end() );
      _rstrLocation.assign( strSystem.begin(), strSystem.end() );
      return true;
    };
  }
};

TYPED_TEST( DtdTest, ExternalSubsetIsRead )
{
  typedef TypeParam TyChar;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  static const char s_szDocument[] =
    "<!DOCTYPE doc SYSTEM 'doc.dtd' [\n"
    "<!ATTLIST doc a CDATA 'internal'>\n"
    "<!ENTITY e 'internal e'>\n"
    "]>\n"
    "<doc>&e; &x;</doc>";
  TestEntities entities;
  entities.m_mapEntities[ "doc.dtd" ] =
    "<?xml version='1.0' encoding='utf-8'?>\n"
    "<!ELEMENT doc (#PCDATA)>\n"
    "<!ATTLIST doc a CDATA 'external' b NMTOKEN ' b '>\n"
    "<!ENTITY e 'external e'>\n"
    "<!ENTITY x 'external x'>\n";
  // The external subset is read after the internal subset: what the internal subset declares is what binds ( 2.8 ).
  {
    CursorDocument< TyChar, true, true > doc( s_szDocument, entities.Resolver< TyChar >(), "dir/doc.xml" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    EXPECT_EQ( entities.m_rgstrAsked, ( std::vector< std::string >{ "doc.dtd <- dir/doc.xml" } ) );
    xml_dtd< TyChar > const & rdtd = doc.m_cursor.GetDtd();
    EXPECT_TRUE( rdtd.m_fExternalSubsetRead );
    EXPECT_TRUE( rdtd.FAllDeclarationsRead() );
    EXPECT_TRUE( !!rdtd.PGetElement( W( "doc" ) ) );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( doc.StrAttributes(), "a=\"internal\" b=\"b\"" );
    EXPECT_EQ( StrNarrow( doc.StrContent() ), "internal e external x" );
    (void)doc.m_cursor.FNextTag();
    EXPECT_TRUE( doc.m_cursor.FIsValid() ) << ( doc.m_cursor.RgValidityErrors().empty() ? std::string() : doc.m_cursor.RgValidityErrors().front() );
  }
  // Without a resolver - or if the resolver doesn't have it - it is not read: that is no error, and what it declares is not known.
  {
    CursorDocument< TyChar > doc( s_szDocument );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    EXPECT_FALSE( doc.m_cursor.GetDtd().m_fExternalSubsetRead );
    EXPECT_FALSE( doc.m_cursor.GetDtd().FAllDeclarationsRead() );
    EXPECT_FALSE( !!doc.m_cursor.GetDtd().PGetElement( W( "doc" ) ) );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( doc.StrAttributes(), "a=\"internal\"" );
  }
  {
    TestEntities entitiesNone;
    CursorDocument< TyChar > doc( s_szDocument, entitiesNone.Resolver< TyChar >() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    EXPECT_EQ( entitiesNone.m_rgstrAsked, ( std::vector< std::string >{ "doc.dtd <- " } ) );
    EXPECT_FALSE( doc.m_cursor.GetDtd().m_fExternalSubsetRead );
  }
  // The document written from the cursor is the document: its document type declaration as it was - the external subset is not part of that.
  {
    CursorDocument< TyChar > doc( s_szDocument, entities.Resolver< TyChar >() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
    EXPECT_EQ( StrNarrowExact( strWritten ), s_szDocument );
  }
  // And written without the declaration it is the same document for whoever reads it without any DTD.
  {
    CursorDocument< TyChar > doc( s_szDocument, entities.Resolver< TyChar >() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc, false ) );
    const std::string strWithout = StrNarrowExact( strWritten );
    EXPECT_EQ( strWithout, "\n<doc a=\"internal\" b=\"b\">internal e external x</doc>" );
    CursorDocument< TyChar > docWith( s_szDocument, entities.Resolver< TyChar >() );
    CursorDocument< TyChar, false > docWithout( strWithout.c_str() );
    ASSERT_TRUE( docWith.FOpened() && docWithout.FOpened() );
    std::string strDifference;
    EXPECT_TRUE( FCompareXmlDocuments( docWith.m_cursor, docWithout.m_cursor, xml_compare_options(), &strDifference ) ) << strDifference;
  }
}

// External parameter entities, and references to parameter entities within declarations - which may be in the external
//  subset and in the text of parameter entities, and not in the internal subset itself.
TYPED_TEST( DtdTest, ExternalParameterEntities )
{
  typedef TypeParam TyChar;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  // A reference between the declarations of the internal subset to an external parameter entity - which declares another:
  //  the system identifier of that is relative to the entity which declares it.
  {
    TestEntities entities;
    entities.m_mapEntities[ "decls.ent" ] =
      "<?xml encoding='utf-8'?>"
      "<!ELEMENT doc (item*)>\n"
      "<!ENTITY % more SYSTEM 'sub/more.ent'>\n"
      "%more;\n"
      "<!ATTLIST item b CDATA 'after'>\n";
    entities.m_mapEntities[ "sub/more.ent" ] = "<!ELEMENT item EMPTY><!ATTLIST item a CDATA 'from more'>";
    CursorDocument< TyChar, true, true > doc(
      "<!DOCTYPE doc [\n"
      "<!ENTITY % decls SYSTEM 'decls.ent'>\n"
      "%decls;\n"
      "]>\n"
      "<doc><item/></doc>", entities.Resolver< TyChar >(), "doc.xml" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    EXPECT_EQ( entities.m_rgstrAsked, ( std::vector< std::string >{ "decls.ent <- doc.xml", "sub/more.ent <- decls.ent" } ) );
    EXPECT_TRUE( doc.m_cursor.GetDtd().FAllDeclarationsRead() );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( doc.StrAttributes(), "a=\"from more\" b=\"after\"" );
    while ( doc.m_cursor.FMoveUp() )
      ;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    (void)doc.m_cursor.FNextTag();
    EXPECT_TRUE( doc.m_cursor.FIsValid() ) << ( doc.m_cursor.RgValidityErrors().empty() ? std::string() : doc.m_cursor.RgValidityErrors().front() );
  }
  // An external parameter entity which isn't had is not read: what it declares is not known and the declarations of
  //  entities and of attributes after it are not processed ( 5.1 ).
  {
    TestEntities entities;
    CursorDocument< TyChar > doc(
      "<!DOCTYPE doc [<!ENTITY % decls SYSTEM 'nosuch.ent'>%decls;<!ATTLIST doc a CDATA 'v'>]><doc/>", entities.Resolver< TyChar >() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    EXPECT_TRUE( doc.m_cursor.GetDtd().m_fStoppedProcessingDeclarations );
    EXPECT_FALSE( doc.m_cursor.GetDtd().FAllDeclarationsRead() );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( doc.StrAttributes(), "" );
  }
  // Within declarations - in the external subset: the name of an element type, a content model and a piece of one, the
  //  definitions of attributes, the keyword of a conditional section, a literal; and an entity which is external.
  {
    TestEntities entities;
    entities.m_mapEntities[ "doc.dtd" ] =
      "<!ENTITY % inline \"b | i\">\n"
      "<!ENTITY % text \"(#PCDATA | %inline;)*\">\n"
      "<!ELEMENT p %text;>\n"
      "<!ELEMENT b (#PCDATA)><!ELEMENT i (#PCDATA)>\n"
      "<!ENTITY % atts \"id ID #IMPLIED class CDATA 'c'\">\n"
      "<!ATTLIST p %atts;>\n"
      "<!ENTITY % draft 'INCLUDE'><!ENTITY % final 'IGNORE'>\n"
      "<![%draft;[ <!ELEMENT note (#PCDATA)> ]]>\n"
      "<![ %final; [ <!ELEMENT hidden (#PCDATA)> ]]>\n"
      "<!ENTITY % name \"doc\">\n"
      "<!ENTITY % items \"p | note\">\n"
      "<!ELEMENT %name; (head?, (%items;)*)>\n"
      "<!ENTITY % extatts SYSTEM 'atts.ent'>\n"
      "<!ATTLIST %name; %extatts;>\n"
      "<!ENTITY % headname 'head'><!ENTITY % headdecl '<!ELEMENT &#37;headname; EMPTY>'>\n"
      "%headdecl;\n"
      "<!ENTITY % system \"'pic.gif'\"><!NOTATION gif SYSTEM 'gif'>\n"
      "<!ENTITY pic SYSTEM %system; NDATA gif>\n"
      "<!ENTITY greeting \"hello %name;\">\n";
    entities.m_mapEntities[ "atts.ent" ] = "<?xml version='1.0' encoding='utf-8'?>lang NMTOKEN 'en'";
    CursorDocument< TyChar, true, true > doc(
      "<!DOCTYPE doc SYSTEM 'doc.dtd'>\n"
      "<doc><head/><p id='x'>&greeting; <b>bold</b></p><note>n</note></doc>", entities.Resolver< TyChar >(), "doc.xml" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    EXPECT_EQ( entities.m_rgstrAsked, ( std::vector< std::string >{ "doc.dtd <- doc.xml", "atts.ent <- doc.dtd" } ) );
    xml_dtd< TyChar > const & rdtd = doc.m_cursor.GetDtd();
    EXPECT_TRUE( rdtd.FAllDeclarationsRead() );
    EXPECT_TRUE( rdtd.m_rgstrValidityErrors.empty() ) << ( rdtd.m_rgstrValidityErrors.empty() ? std::string() : rdtd.m_rgstrValidityErrors.front() );
    ASSERT_TRUE( !!rdtd.PGetElement( W( "p" ) ) );
    EXPECT_EQ( rdtd.PGetElement( W( "p" ) )->m_eContent, exdcMixed );
    EXPECT_EQ( StrJoin( rdtd.PGetElement( W( "p" ) )->m_rgstrMixedNames ), "b i" );
    EXPECT_TRUE( !!rdtd.PGetElement( W( "note" ) ) );
    EXPECT_FALSE( !!rdtd.PGetElement( W( "hidden" ) ) );
    ASSERT_TRUE( !!rdtd.PGetElement( W( "doc" ) ) );
    EXPECT_EQ( StrContentParticle( rdtd.PGetElement( W( "doc" ) )->m_cpChildren ), "(head?,(p|note)*)" );
    ASSERT_TRUE( !!rdtd.PGetElement( W( "head" ) ) );
    EXPECT_EQ( rdtd.PGetElement( W( "head" ) )->m_eContent, exdcEmpty );
    ASSERT_TRUE( !!rdtd.PGetGeneralEntity( W( "pic" ) ) );
    EXPECT_EQ( StrNarrow( *rdtd.PGetGeneralEntity( W( "pic" ) )->m_externalId.m_optstrSystem ), "pic.gif" );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( doc.StrAttributes(), "lang=\"en\"" );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() ); // head
    ASSERT_TRUE( doc.m_cursor.FNextTag() ); // p
    EXPECT_EQ( doc.StrAttributes(), "id=\"x\" class=\"c\"" );
    EXPECT_EQ( StrNarrow( doc.StrContent() ), "hello doc " );
    while ( doc.m_cursor.FMoveUp() )
      ;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    (void)doc.m_cursor.FNextTag();
    EXPECT_TRUE( doc.m_cursor.FIsValid() ) << ( doc.m_cursor.RgValidityErrors().empty() ? std::string() : doc.m_cursor.RgValidityErrors().front() );
  }
  // WFC: PEs in Internal Subset - not within a declaration of the internal subset itself. But within the text of a
  //  parameter entity which is referenced there it may be.
  {
    CursorDocument< TyChar > doc( "<!DOCTYPE doc [<!ENTITY % c '(#PCDATA)'><!ELEMENT doc %c;>]><doc/>" );
    EXPECT_FALSE( doc.FOpened() );
  }
  {
    CursorDocument< TyChar > doc( "<!DOCTYPE doc [<!ENTITY % c '(#PCDATA)'><!ENTITY % d '<!ELEMENT doc &#37;c;>'>%d;]><doc/>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( !!doc.m_cursor.GetDtd().PGetElement( W( "doc" ) ) );
    EXPECT_EQ( doc.m_cursor.GetDtd().PGetElement( W( "doc" ) )->m_eContent, exdcMixed );
  }
  // Within a declaration there is no going on without the text of the entity: one which isn't declared, one which isn't
  //  had - and one which refers to itself ( WFC: No Recursion ).
  for ( const char * pszSubset : { "<!ELEMENT doc %nosuch;>", "<!ENTITY % e SYSTEM 'nosuch.ent'><!ELEMENT doc %e;>",
                                   "<!ENTITY % a '(&#37;b;)'><!ENTITY % b '&#37;a;'><!ELEMENT doc %a;>" } )
  {
    TestEntities entities;
    entities.m_mapEntities[ "doc.dtd" ] = pszSubset;
    CursorDocument< TyChar > doc( "<!DOCTYPE doc SYSTEM 'doc.dtd'><doc/>", entities.Resolver< TyChar >() );
    EXPECT_FALSE( doc.FOpened() ) << pszSubset;
  }
}

// The validity constraints which are about the external declarations: those of the external subset and of the texts of
//  parameter entities.
TYPED_TEST( DtdTest, ValidationOfExternalDeclarations )
{
  typedef TypeParam TyChar;
  // Read the document - with the entities - by a validating cursor. Returns what is not valid about it: see StrValidate().
  auto Validate = []( const char * _pszDocument, const char * _pszExternalSubset ) -> std::string
  {
    TestEntities entities;
    entities.m_mapEntities[ "doc.dtd" ] = _pszExternalSubset;
    CursorDocument< TyChar, true, true > doc( _pszDocument, entities.Resolver< TyChar >() );
    if ( !doc.FOpened() )
      return "not opened: " + doc.m_strError;
    try
    {
      if ( doc.m_cursor.FMoveDown() )
        (void)doc.m_cursor.FNextTag();
    }
    catch ( std::exception const & rexc )
    {
      return std::string( "exception: " ) + rexc.what();
    }
    std::string strErrors;
    for ( std::string const & rstrError : doc.m_cursor.GetDtd().m_rgstrValidityErrors )
      strErrors += rstrError + "\n";
    for ( std::string const & rstrError : doc.m_cursor.RgValidityErrors() )
      strErrors += rstrError + "\n";
    return strErrors;
  };
  static const char s_szDocument[] = "<!DOCTYPE doc SYSTEM 'doc.dtd'><doc/>";
  // The text of a parameter entity is properly nested with the declarations: what begins in it ends in it.
  EXPECT_EQ( Validate( s_szDocument, "<!ENTITY % e '(#PCDATA)'><!ELEMENT doc %e;>" ), "" );
  EXPECT_EQ( Validate( s_szDocument, "<!ENTITY % e '#PCDATA'><!ELEMENT doc (%e;)>" ), "" );
  // VC: Proper Declaration/PE Nesting.
  EXPECT_EQ( Validate( s_szDocument, "<!ENTITY % e '>'><!ELEMENT doc (#PCDATA) %e;" ),
    "A declaration begins in the text of one entity and ends in that of another: parameter entities are not properly nested with the declarations.\n" );
  // VC: Proper Group/PE Nesting.
  EXPECT_EQ( Validate( s_szDocument, "<!ENTITY % e '(#PCDATA'><!ELEMENT doc %e;)>" ),
    "A parenthesized group of a content model begins in the text of one entity and ends in that of another: parameter entities are not properly nested with the declarations.\n" );
  EXPECT_EQ( Validate( "<!DOCTYPE doc SYSTEM 'doc.dtd'><doc><a/></doc>", "<!ELEMENT a EMPTY><!ENTITY % e 'a)'><!ELEMENT doc (%e;>" ),
    "A parenthesized group of a content model begins in the text of one entity and ends in that of another: parameter entities are not properly nested with the declarations.\n" );
  // VC: Proper Conditional Section/PE Nesting.
  EXPECT_EQ( Validate( s_szDocument, "<!ELEMENT doc EMPTY><!ENTITY % e 'INCLUDE['><![ %e; <!ATTLIST doc a CDATA #IMPLIED> ]]>" ),
    "The \"<![\" and the '[' of a conditional section are in the texts of different entities: parameter entities are not properly nested with the declarations.\n" );
  EXPECT_EQ( Validate( s_szDocument, "<!ELEMENT doc EMPTY><!ENTITY % e '> ]]>'><![INCLUDE[ <!ATTLIST doc a CDATA #IMPLIED %e;" ),
    "A declaration begins in the text of one entity and ends in that of another: parameter entities are not properly nested with the declarations.\n"
    "A conditional section begins in the text of one entity and ends in that of another: parameter entities are not properly nested with the declarations.\n" );
  // A reference between declarations is another matter: the text of the entity must be whole declarations or the DTD is not well formed.
  EXPECT_EQ( Validate( s_szDocument, "<!ELEMENT doc EMPTY><!ENTITY % e '<!ATTLIST doc a CDATA #IMPLIED> ]]>'><![INCLUDE[ %e;" ).substr( 0, 11 ), "not opened:" );

  // VC: Standalone Document Declaration: a document which is declared standalone is not changed by external declarations.
  static const char s_szExternal[] =
    "<!ELEMENT doc (item*)>\n"
    "<!ELEMENT item (#PCDATA)>\n"
    "<!ATTLIST item kind NMTOKEN #IMPLIED lang CDATA 'en'>\n"
    "<!ENTITY ext 'external text'>\n";
  static const char s_szYes[] = "<?xml version='1.0' standalone='yes'?><!DOCTYPE doc SYSTEM 'doc.dtd' [<!ENTITY int 'internal text'>]>";
  static const char s_szNo[] = "<?xml version='1.0' standalone='no'?><!DOCTYPE doc SYSTEM 'doc.dtd' [<!ENTITY int 'internal text'>]>";
  static const char s_szNothing[] = "<!DOCTYPE doc SYSTEM 'doc.dtd' [<!ENTITY int 'internal text'>]>";
  auto ValidateStandalone = [&]( const char * _pszProlog, const char * _pszRoot )
  {
    return Validate( ( std::string( _pszProlog ) + _pszRoot ).c_str(), s_szExternal );
  };
  // What owes nothing to them.
  EXPECT_EQ( ValidateStandalone( s_szYes, "<doc><item kind='k' lang='fr'>&int; &lt;</item></doc>" ), "" );
  // An attribute by its default.
  static const char s_szDefault[] = "<doc><item/></doc>";
  EXPECT_EQ( ValidateStandalone( s_szYes, s_szDefault ),
    "The document is declared standalone and a tag <item> has the attribute [lang] by its default - which is declared externally.\n" );
  // A reference to an entity which is declared externally - in character data or in the value of an attribute: in a
  //  document which is declared standalone the entity is not declared ( 4.1 WFC: Entity Declared ) - that is an error
  //  of well-formedness, not of validity alone.
  static const char s_szEntity[] = "<doc><item lang='&ext;'>&ext;</item></doc>";
  EXPECT_NE( ValidateStandalone( s_szYes, s_szEntity ).find( "is declared standalone and the value of an attribute refers to the entity [ext]" ), std::string::npos );
  EXPECT_NE( ValidateStandalone( s_szYes, "<doc><item lang='fr'>&ext;</item></doc>" ).find( "exception:" ), std::string::npos );
  // The value of an attribute which its type makes another.
  static const char s_szNormalized[] = "<doc><item lang='x' kind=' k '/></doc>";
  EXPECT_EQ( ValidateStandalone( s_szYes, s_szNormalized ),
    "The document is declared standalone and the value of an attribute is normalized by the type of the attribute [kind] - which is declared externally.\n" );
  // White space within element content.
  static const char s_szWhiteSpace[] = "<doc> <item lang='x'/>\n</doc>";
  EXPECT_EQ( ValidateStandalone( s_szYes, s_szWhiteSpace ),
    "The document is declared standalone and has white space within the element <doc> - whose element content is declared externally.\n" );
  // None of which is anything if the document is not declared standalone - or says that it isn't.
  for ( const char * pszRoot : { s_szDefault, s_szEntity, s_szNormalized, s_szWhiteSpace } )
  {
    EXPECT_EQ( ValidateStandalone( s_szNo, pszRoot ), "" ) << pszRoot;
    EXPECT_EQ( ValidateStandalone( s_szNothing, pszRoot ), "" ) << pszRoot;
  }
}

// External entities which are files - FReadExternalEntityFile(): each in an encoding of its own.
TEST( DtdExternalFiles, FilesInTheirEncodings )
{
  namespace fs = std::filesystem;
  const fs::path pathDir = fs::temp_directory_path() / "xmlpgtest_dtd_external";
  std::error_code ec;
  fs::remove_all( pathDir, ec );
  ASSERT_TRUE( fs::create_directories( pathDir / "sub", ec ) ) << ec.message();
  auto WriteFile = []( fs::path const & _rpath, std::string const & _rstrBytes )
  {
    std::ofstream ofs( _rpath, std::ios::binary );
    ofs.write( _rstrBytes.data(), std::streamsize( _rstrBytes.size() ) );
  };
  auto Utf16 = []( const char * _psz, bool _fLittleEndian )
  {
    std::string strBytes( _fLittleEndian ? "\xff\xfe" : "\xfe\xff" );
    for ( ; !!*_psz; ++_psz )
    {
      strBytes += _fLittleEndian ? *_psz : '\0';
      strBytes += _fLittleEndian ? '\0' : *_psz;
    }
    return strBytes;
  };
  // The external subset in UTF-16 with a byte order mark; it refers to an entity beside it which is in ISO-8859-1 -
  //  as its text declaration says - and to one in a directory below, in UTF-16 of the other byte order.
  WriteFile( pathDir / "doc.dtd", Utf16(
    "<!ELEMENT doc (#PCDATA)>\n"
    "<!ENTITY % latin SYSTEM 'latin.ent'>%latin;\n"
    "<!ENTITY % big SYSTEM 'sub/big.ent'>%big;\n", true ) );
  WriteFile( pathDir / "latin.ent", "<?xml version='1.0' encoding='ISO-8859-1'?><!ENTITY e 'caf\xe9'>" );
  WriteFile( pathDir / "sub" / "big.ent", Utf16( "<!ENTITY % beside SYSTEM 'beside.ent'>%beside;<!ATTLIST doc a CDATA 'big'>", false ) );
  WriteFile( pathDir / "sub" / "beside.ent", "\xef\xbb\xbf<!ATTLIST doc b CDATA 'beside'>" ); // UTF-8 with a byte order mark.
  const std::string strDocumentPath = ( pathDir / "doc.xml" ).generic_string();
  static const char s_szDocument[] = "<!DOCTYPE doc SYSTEM 'doc.dtd'><doc>&e;</doc>";
  {
    typedef xml_traits< _l_transport_fixedmem< char16_t >, true, true > TyXmlTraits;
    std::u16string strText( 1, char16_t( 0xFEFF ) );
    strText.append( s_szDocument, s_szDocument + strlen( s_szDocument ) );
    xml_parser< TyXmlTraits > xmlParser;
    xmlParser.SetReadExternalEntityFiles();
    xmlParser.SetBaseLocation( std::u16string( strDocumentPath.begin(), strDocumentPath.end() ) );
    xml_read_cursor< TyXmlTraits > cursor;
    ASSERT_NO_THROW( cursor = xmlParser.OpenMemory( strText.data(), strText.length() * sizeof( char16_t ) ) );
    ASSERT_TRUE( cursor.FHasDtd() );
    EXPECT_TRUE( cursor.GetDtd().FAllDeclarationsRead() );
    ASSERT_TRUE( cursor.FMoveDown() );
    std::u16string strContent;
    cursor.GetContextCur().GetContentString( strContent );
    EXPECT_TRUE( strContent == u"caf\u00e9" );
    auto const & rltokTag = cursor.GetTagCur().GetLexToken();
    auto const & rrgAttrs = rltokTag.GetValue()[ vknAttributesIdx ];
    ASSERT_EQ( rrgAttrs.GetSize(), 2u );
    std::u16string strB, strA;
    rltokTag.GetString( strB, rrgAttrs[ 0 ][ vknAttr_ValueIdx ] );
    rltokTag.GetString( strA, rrgAttrs[ 1 ][ vknAttr_ValueIdx ] );
    EXPECT_TRUE( strB == u"beside" );
    EXPECT_TRUE( strA == u"big" );
    (void)cursor.FNextTag();
    EXPECT_TRUE( cursor.FIsValid() );
  }
  // Without being told to, nothing is read.
  {
    typedef xml_traits< _l_transport_fixedmem< char8_t >, true, false > TyXmlTraits;
    xml_parser< TyXmlTraits > xmlParser;
    xmlParser.SetBaseLocation( std::u8string( strDocumentPath.begin(), strDocumentPath.end() ) );
    xml_read_cursor< TyXmlTraits > cursor;
    ASSERT_NO_THROW( cursor = xmlParser.OpenMemory( s_szDocument, strlen( s_szDocument ) ) );
    ASSERT_TRUE( cursor.FHasDtd() );
    EXPECT_FALSE( cursor.GetDtd().m_fExternalSubsetRead );
  }
  // Only files: an entity which is elsewhere is not read.
  {
    xml_dtd_external_id< char8_t > externalId;
    externalId.m_optstrSystem = u8"http://example.com/doc.dtd";
    std::u8string strText, strLocation;
    EXPECT_FALSE( FReadExternalEntityFile< char8_t >( externalId, std::u8string_view(), strText, strLocation ) );
    externalId.m_optstrSystem = u8"nosuch.dtd";
    const std::u8string strBase( strDocumentPath.begin(), strDocumentPath.end() );
    EXPECT_FALSE( FReadExternalEntityFile< char8_t >( externalId, strBase, strText, strLocation ) );
    externalId.m_optstrSystem = u8"sub/beside.ent";
    ASSERT_TRUE( FReadExternalEntityFile< char8_t >( externalId, strBase, strText, strLocation ) );
    EXPECT_TRUE( strText == u8"<!ATTLIST doc b CDATA 'beside'>" );
    EXPECT_TRUE( strLocation.ends_with( u8"/sub/beside.ent" ) );
  }
  fs::remove_all( pathDir, ec );
}

// General entities whose replacement text is not text alone - 4.4.2: a reference in content to such an entity is replaced
//  by the text, which is content of the document: its tags are tags of the document, and the character data around
//  the reference and that of the entity are the character data of the element which they are in.
TYPED_TEST( DtdTest, EntitiesWithMarkup )
{
  typedef TypeParam TyChar;
  typedef std::conditional_t< sizeof( TyChar ) == sizeof( char8_t ), char32_t, char8_t > TyCharOther;
  // The document with its entities is the same document as _pszExpanded - which has no DTD - read without any.
  auto DifferenceFromExpanded = []( const char * _pszDocument, const char * _pszExpanded )
  {
    return StrDifference< TyChar, true, TyCharOther, false >( _pszDocument, _pszExpanded );
  };
  // What reading all of the document throws - empty if nothing.
  auto ErrorReading = []( const char * _pszDocument ) -> std::string
  {
    const std::string strDifference = StrDifference< TyChar, true, TyChar, true >( _pszDocument, _pszDocument );
    return strDifference;
  };
  static const char s_szDoctype[] =
    "<!DOCTYPE doc [\n"
    "<!ENTITY e 'in <b a=\"1\">bold &amp; <i/></b> out'>\n"
    "<!ENTITY t 'text'>\n"
    "<!ENTITY inner '<i>x&t;</i>'>\n"
    "<!ENTITY outer 'a &inner; b &inner;'>\n"
    "<!ENTITY built \"&#60;foo>&#38;amp;&#60;/foo>\">\n"
    "<!ENTITY empty '<!-- nothing -->'>\n"
    "]>\n";
  auto Doc = []( const char * _pszRoot ) { return std::string( s_szDoctype ) + _pszRoot; };
  EXPECT_EQ( DifferenceFromExpanded( Doc( "<doc>before &e; after &t; end</doc>" ).c_str(), "<doc>before in <b a='1'>bold &amp; <i/></b> out after text end</doc>" ), "" );
  // At the beginning and at the end of an element, one after another, and all that an element has.
  EXPECT_EQ( DifferenceFromExpanded( Doc( "<doc>&e;&e;<p>&inner;</p></doc>" ).c_str(),
    "<doc>in <b a='1'>bold &amp; <i/></b> outin <b a='1'>bold &amp; <i/></b> out<p><i>xtext</i></p></doc>" ), "" );
  // An entity whose text refers to another which has markup.
  EXPECT_EQ( DifferenceFromExpanded( Doc( "<doc>&outer;</doc>" ).c_str(), "<doc>a <i>xtext</i> b <i>xtext</i></doc>" ), "" );
  // Markup which the literal of the entity has as character references ( 4.5 ).
  EXPECT_EQ( DifferenceFromExpanded( Doc( "<doc>&built;</doc>" ).c_str(), "<doc><foo>&amp;</foo></doc>" ), "" );
  // The text may be anything which content may have.
  EXPECT_EQ( DifferenceFromExpanded( Doc( "<doc>x&empty;y</doc>" ).c_str(), "<doc>x<!-- nothing -->y</doc>" ), "" );

  // Written from the cursor with its document type declaration the document is as it was: the references are written
  //  and not what the entities have. Without the declaration what they have is what is written.
  {
    const std::string strDocument = Doc( "<doc>before &e; after &t; <p>&outer;</p>&built; end</doc>\n" );
    {
      CursorDocument< TyChar > doc( strDocument.c_str() );
      ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
      std::basic_string< TyChar > strWritten;
      ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
      EXPECT_EQ( StrNarrowExact( strWritten ), strDocument );
    }
    {
      CursorDocument< TyChar > doc( strDocument.c_str() );
      ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
      std::basic_string< TyChar > strWritten;
      ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc, false ) );
      const std::string strWithout = StrNarrowExact( strWritten );
      EXPECT_EQ( strWithout,
        "\n<doc>before in <b a=\"1\">bold &amp; <i/></b> out after text <p>a <i>xtext</i> b <i>xtext</i></p><foo>&amp;</foo> end</doc>\n" );
      EXPECT_EQ( DifferenceFromExpanded( strDocument.c_str(), strWithout.c_str() ), "" );
    }
  }
  // The tokens which mark what an entity has are among the content - and one may move within what an entity has and
  //  pass over the rest of it.
  {
    typedef typename CursorDocument< TyChar >::_TyXmlTraits::_TyXmlToken TyXmlToken;
    CursorDocument< TyChar > doc( Doc( "<doc><sec>s &e; t</sec><last/></doc>" ).c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() ); // doc
    ASSERT_TRUE( doc.m_cursor.FMoveDown() ); // sec
    std::string strContent;
    doc.m_cursor.ApplyAllContent(
      [&strContent]( const TyXmlToken * _pxtBegin, const TyXmlToken * _pxtEnd )
      {
        for ( ; _pxtEnd != _pxtBegin; ++_pxtBegin )
        {
          std::basic_string< TyChar > str;
          _pxtBegin->GetLexToken().GetString( str );
          strContent += ( _pxtBegin->FIsEntityContentBegin() ? "{" : _pxtBegin->FIsEntityContentEnd() ? "}" : "[" ) + StrNarrow( str ) + ( _pxtBegin->FIsEntityContentBegin() || _pxtBegin->FIsEntityContentEnd() ? "" : "]" );
        }
      } );
    EXPECT_EQ( strContent, "[s ]{e[in ]" );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() ); // b - of the entity.
    EXPECT_EQ( StrNarrow( doc.m_cursor.SvTagCur() ), "b" );
    EXPECT_EQ( doc.StrAttributes(), "a=\"1\"" );
    ASSERT_TRUE( doc.m_cursor.FMoveUp() ); // sec
    ASSERT_TRUE( doc.m_cursor.FNextTag() ); // Past the rest of sec - the rest of the entity and what follows it - to last.
    EXPECT_EQ( StrNarrow( doc.m_cursor.SvTagCur() ), "last" );
  }
  // Read into a tree.
  {
    typedef typename CursorDocument< TyChar >::_TyXmlTraits TyXmlTraits;
    CursorDocument< TyChar > doc( Doc( "<doc>before &e; after <p>&outer;</p></doc>" ).c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    auto prXmlDoc = xml_document< TyXmlTraits >::PrCreateXmlDocument();
    ASSERT_NO_THROW( prXmlDoc.first->FromXmlStream( doc.m_cursor, prXmlDoc.second ) );
  }

  // The text of an entity is to match content [43]: an element which begins in it ends in it, and it doesn't end one
  //  which begins outside of it.
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY e '<b>'>]><doc>&e;</b></doc>" ).find( "doesn't end in it" ), std::string::npos );
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY e '</b>'>]><doc><b>&e;</doc>" ).find( "doesn't begin in it" ), std::string::npos );
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY e '<b>x</b><c'>]><doc>&e;</doc>" ).find( "not well formed as content" ), std::string::npos );
  // WFC: No Recursion.
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY a '<x>&b;</x>'><!ENTITY b 'y &a;'>]><doc>&a;</doc>" ).find( "reference to itself" ), std::string::npos );
  // WFC: Parsed Entity.
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!NOTATION n SYSTEM 'n'><!ENTITY u SYSTEM 'u.gif' NDATA n>]><doc>&u;</doc>" ).find( "unparsed entity" ), std::string::npos );
  // Outside of the document's element there is no reference.
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY e '<b/>'>]>&e;<doc/>" ), "" );

  // Validation: what the entity has is what the element has.
  {
    static const char s_szValidate[] = "<!DOCTYPE doc [<!ELEMENT doc (b, c?)><!ELEMENT b EMPTY><!ELEMENT c (#PCDATA)><!ENTITY e '<b/>'><!ENTITY f '<c>x</c>'>]>";
    auto Validate = []( const char * _pszRoot )
    {
      return StrValidate< TyChar >( ( std::string( s_szValidate ) + _pszRoot ).c_str() );
    };
    EXPECT_EQ( Validate( "<doc>&e;</doc>" ), "" );
    EXPECT_EQ( Validate( "<doc>&e;&f;</doc>" ), "" );
    EXPECT_EQ( Validate( "<doc>&e;&f;</doc>" ), StrValidate< TyChar >( ( std::string( s_szValidate ) + "<doc>&e;&f;</doc>" ).c_str(), true ) );
    EXPECT_EQ( Validate( "<doc>&f;</doc>" ), "The element <c> may not be here within the element <doc>: its content model has <b>.\n" );
    EXPECT_EQ( Validate( "<doc>&e;&e;</doc>" ), "The element <b> may not be here within the element <doc>: its content model has <c> or the end of the element.\n" );
  }

  // An external parsed entity [78]: read if the parser has the means - its text declaration is not part of its text.
  {
    static const char s_szDocument[] = "<!DOCTYPE doc [<!ENTITY chap SYSTEM 'sub/chap.xml'><!ENTITY t 'text'>]><doc>one &chap; two</doc>";
    TestEntities entities;
    entities.m_mapEntities[ "sub/chap.xml" ] = "<?xml version='1.0' encoding='utf-8'?><chapter n='1'>&t; of it</chapter> and more";
    {
      CursorDocument< TyChar > doc( s_szDocument, entities.Resolver< TyChar >(), "doc.xml" );
      CursorDocument< TyCharOther, false > docExpanded( "<doc>one <chapter n='1'>text of it</chapter> and more two</doc>" );
      ASSERT_TRUE( doc.FOpened() && docExpanded.FOpened() ) << doc.m_strError;
      std::string strDifference;
      EXPECT_TRUE( FCompareXmlDocuments( doc.m_cursor, docExpanded.m_cursor, xml_compare_options(), &strDifference ) ) << strDifference;
      EXPECT_EQ( entities.m_rgstrAsked, ( std::vector< std::string >{ "sub/chap.xml <- doc.xml" } ) );
    }
    {
      CursorDocument< TyChar > doc( s_szDocument, entities.Resolver< TyChar >(), "doc.xml" );
      ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
      std::basic_string< TyChar > strWritten;
      ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
      EXPECT_EQ( StrNarrowExact( strWritten ), s_szDocument );
    }
    // Without the means it is not read: the reference is one to an entity which there is no text for, and a document
    //  which is being validated is not all validated.
    {
      CursorDocument< TyChar, true, true > doc( "<!DOCTYPE doc [<!ELEMENT doc ANY><!ENTITY chap SYSTEM 'sub/chap.xml'>]><doc>one &chap; two</doc>" );
      ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
      ASSERT_TRUE( doc.m_cursor.FMoveDown() );
      EXPECT_ANY_THROW( doc.StrContent() );
      EXPECT_TRUE( doc.m_cursor.FValidationIncomplete() );
    }
  }
}

// The strings of the content of the current tag - those which the cursor has read: "[string]" for each token but for
//  the processing instructions, whose value is not one string.
template < class t_TyChar, bool t_kfSupportDTD, bool t_kfValidating >
std::string StrContentTokens( CursorDocument< t_TyChar, t_kfSupportDTD, t_kfValidating > & _rdoc )
{
  typedef typename CursorDocument< t_TyChar, t_kfSupportDTD, t_kfValidating >::_TyXmlTraits::_TyXmlToken TyXmlToken;
  std::string strContent;
  _rdoc.m_cursor.ApplyAllContent(
    [&strContent]( const TyXmlToken * _pxtBegin, const TyXmlToken * _pxtEnd )
    {
      for ( ; _pxtEnd != _pxtBegin; ++_pxtBegin )
      {
        if ( _pxtBegin->FIsProcessingInstruction() )
        {
          strContent += "[?]";
          continue;
        }
        std::basic_string< t_TyChar > str;
        _pxtBegin->GetLexToken().GetString( str );
        strContent += "[" + StrNarrowExact( str ) + "]";
      }
    } );
  return strContent;
}

// The ends of lines - 2.11: #xD #xA and a #xD which is not before a #xA are each #xA in the strings which are had from
//  the document. A #xD which a character reference stands for is that character. The document is as it was: what is
//  written from it is what was read.
TYPED_TEST( DtdTest, LineEnds )
{
  typedef TypeParam TyChar;
  typedef std::conditional_t< sizeof( TyChar ) == sizeof( char8_t ), char32_t, char8_t > TyCharOther;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  static const char s_szDocument[] =
    "<!-- before\r\nthe element -->\r\n"
    "<doc>one\r\ntwo\rthree\nfour&#13;five\r&#10;six<!--c\r\nd\r--><?pi p\r\nq?><![CDATA[e\r\nf\r]]>\r\n"
    "<sub>\r</sub>\r\n"
    "</doc>\r\n";
  static const char s_szNormalized[] =
    "<!-- before\nthe element -->\n"
    "<doc>one\ntwo\nthree\nfour&#13;five\n&#10;six<!--c\nd\n--><?pi p\nq?><![CDATA[e\nf\n]]>\n"
    "<sub>\n</sub>\n"
    "</doc>\n";
  // They are the same document - whether or not the cursor supports the DTD.
  EXPECT_EQ( ( StrDifference< TyChar, false, TyCharOther, false >( s_szDocument, s_szNormalized ) ), "" );
  EXPECT_EQ( ( StrDifference< TyChar, true, TyCharOther, true >( s_szDocument, s_szNormalized ) ), "" );
  {
    CursorDocument< TyChar, false > doc( s_szDocument );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    EXPECT_EQ( StrContentTokens( doc ), "[ before\nthe element ][\n]" );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( StrContentTokens( doc ), "[one\ntwo\nthree\nfour\rfive\n\nsix][c\nd\n][?][e\nf\n][\n]" );
    EXPECT_TRUE( doc.StrContent() == W( "one\ntwo\nthree\nfour\rfive\n\nsixe\nf\n\n" ) ) << StrNarrowExact( doc.StrContent() );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( StrContentTokens( doc ), "[\n]" );
  }
  // Having the strings changed nothing of what the document has: it is written as it was read.
  {
    CursorDocument< TyChar > doc( s_szDocument );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
    EXPECT_EQ( StrNarrowExact( strWritten ), s_szDocument );
  }
  // The DTD: the literal of an entity and that of a default value are text of the document - their ends of lines
  //  are normalized. A #xD which the literal of an entity has as a character reference is in the replacement text
  //  as that character, and is that character where the entity is referenced - also where the replacement text
  //  is read as content since it has markup.
  {
    static const char s_szDocumentDtd[] =
      "<!DOCTYPE doc [\r\n"
      "<!ENTITY cr 'a&#13;b'>\r\n"
      "<!ENTITY nl 'a\r\nb\rc'>\r\n"
      "<!ENTITY m '<i>x&#13;y\r\nz</i><!--k&#13;l-->'>\r\n"
      "<!ATTLIST doc d CDATA 'p\r\nq'>\r\n"
      "]>\r\n"
      "<doc>&cr;|&nl;|&m;</doc>\r\n";
    {
      CursorDocument< TyChar > doc( s_szDocumentDtd );
      ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
      ASSERT_TRUE( doc.m_cursor.FMoveDown() );
      EXPECT_EQ( doc.StrAttributes(), "d=\"p q\"" );
      EXPECT_TRUE( doc.StrContent() == W( "a\rb|a\nb\nc|" ) ) << StrNarrowExact( doc.StrContent() );
      ASSERT_TRUE( doc.m_cursor.FMoveDown() ); // i - of the entity.
      EXPECT_TRUE( doc.StrContent() == W( "x\ry\nz" ) ) << StrNarrowExact( doc.StrContent() );
      ASSERT_FALSE( doc.m_cursor.FNextTag() );
      // After the element of the entity: its comment - and the token which ends what the entity has.
      EXPECT_EQ( StrContentTokens( doc ).substr( 0, 7 ), "[k\rl][m" );
    }
    {
      CursorDocument< TyChar > doc( s_szDocumentDtd );
      ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
      std::basic_string< TyChar > strWritten;
      ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
      EXPECT_EQ( StrNarrowExact( strWritten ), s_szDocumentDtd );
    }
    // Written without its document type declaration what the entities stand for is what is written - and a #xD of
    //  that is written as a reference to it: as itself it would be read as the end of a line.
    {
      CursorDocument< TyChar > doc( s_szDocumentDtd );
      ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
      std::basic_string< TyChar > strWritten;
      ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc, false ) );
      const std::string strWithout = StrNarrowExact( strWritten );
      EXPECT_EQ( strWithout, "\r\n<doc d=\"p q\">a&#13;b|a\nb\nc|<i>x&#13;y\nz</i><!--k\rl--></doc>\r\n" );
      xml_compare_options options;
      options.m_fCompareComments = false; // There is no writing a #xD in a comment so that it is read as one.
      EXPECT_EQ( ( StrDifference< TyChar, true, TyCharOther, false >( s_szDocumentDtd, strWithout.c_str(), options ) ), "" );
    }
  }
  // A #xD which a character reference put in the replacement text of a parameter entity is a character of what that
  //  text declares - here of the literal of an entity which is within it: that literal is not text of the document.
  {
    CursorDocument< TyChar > doc( "<!DOCTYPE doc [<!ENTITY % pe \"<!ENTITY g 'a&#13;b'>\">%pe;<!ENTITY h 'a\r\nb'>]><doc>&g;|&h;</doc>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_TRUE( doc.StrContent() == W( "a\rb|a\nb" ) ) << StrNarrowExact( doc.StrContent() );
  }
  // The text of an external entity is normalized as it is had.
  {
    TestEntities entities;
    entities.m_mapEntities[ "doc.dtd" ] = "<!ENTITY ext 'e1\r\ne2'>\r\n<!ATTLIST doc d CDATA 'p\r\nq'>\r\n";
    entities.m_mapEntities[ "chap.xml" ] = "<c>l1\r\nl2\r</c>";
    CursorDocument< TyChar > doc( "<!DOCTYPE doc SYSTEM 'doc.dtd' [<!ENTITY chap SYSTEM 'chap.xml'>]><doc>&ext;&chap;</doc>", entities.Resolver< TyChar >(), "doc.xml" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( doc.StrAttributes(), "d=\"p q\"" );
    EXPECT_TRUE( doc.StrContent() == W( "e1\ne2" ) ) << StrNarrowExact( doc.StrContent() );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() ); // c
    EXPECT_TRUE( doc.StrContent() == W( "l1\nl2\n" ) ) << StrNarrowExact( doc.StrContent() );
  }
}

// A reference to an entity within the value of an attribute - 3.3.3: the white space of the replacement text of the
//  entity is spaces in the value - but not a character which a reference within the replacement text stands for.
//  WFC: No External Entity References, WFC: No < in Attribute Values [41].
TYPED_TEST( DtdTest, EntitiesInAttributeValues )
{
  typedef TypeParam TyChar;
  typedef std::conditional_t< sizeof( TyChar ) == sizeof( char8_t ), char32_t, char8_t > TyCharOther;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  static const char s_szDocument[] =
    "<!DOCTYPE doc [\n"
    "<!ENTITY ws 'a\tb\nc'>\n"
    "<!ENTITY ref 'a&#9;b'>\n"
    "<!ENTITY dref 'a&#38;#9;b'>\n"
    "<!ENTITY outer 'x&ws;y'>\n"
    "<!ENTITY plain 'p q'>\n"
    "<!ATTLIST doc n NMTOKENS #IMPLIED>\n"
    "]>\n"
    "<doc v='&ws;' r='&ref;' d='&dref;' o='[&outer;]' p='&plain;' n=' &ws; '>&ws;|&ref;|&dref;|&outer;</doc>\n";
  {
    CursorDocument< TyChar > doc( s_szDocument );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    static const char * const s_rgpszValues[] = { "a b c", "a b", "a\tb", "[xa b cy]", "p q", "a b c" };
    for ( size_t nAttr = 0; nAttr < DimensionOf( s_rgpszValues ); ++nAttr )
      EXPECT_TRUE( StrAttrValue( doc, nAttr ) == W( s_rgpszValues[nAttr] ) ) << nAttr << ": " << StrNarrowExact( StrAttrValue( doc, nAttr ) );
    // In content the text of each is as it is.
    EXPECT_TRUE( doc.StrContent() == W( "a\tb\nc|a\tb|a\tb|xa\tb\ncy" ) ) << StrNarrowExact( doc.StrContent() );
  }
  // Written with its document type declaration the document is as it was. Without it what the entities stand for is
  //  written - in the value of an attribute what they stand for there - and it is the same document to who reads it.
  {
    CursorDocument< TyChar > doc( s_szDocument );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
    EXPECT_EQ( StrNarrowExact( strWritten ), s_szDocument );
  }
  {
    CursorDocument< TyChar > doc( s_szDocument );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc, false ) );
    const std::string strWithout = StrNarrowExact( strWritten );
    EXPECT_EQ( strWithout,
      "\n<doc v='a b c' r='a b' d='a&#9;b' o='[xa b cy]' p='p q' n='a b c'>a\tb\nc|a\tb|a\tb|xa\tb\ncy</doc>\n" );
    EXPECT_EQ( ( StrDifference< TyChar, true, TyCharOther, false >( s_szDocument, strWithout.c_str() ) ), "" );
  }
  // What reading all of the document throws - empty if nothing.
  auto ErrorReading = []( const char * _pszDocument ) -> std::string
  {
    return StrDifference< TyChar, true, TyChar, true >( _pszDocument, _pszDocument );
  };
  // WFC: No < in Attribute Values: not in the replacement text of the entity nor in that of one which it refers to.
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY e '<b/>'>]><doc a='&e;'/>" ).find( "'<' in its replacement text" ), std::string::npos );
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY e '&#60;'>]><doc a='&e;'/>" ).find( "'<' in its replacement text" ), std::string::npos );
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY e '<b/>'><!ENTITY i 'x&e;'>]><doc a='&i;'/>" ).find( "'<' in its replacement text" ), std::string::npos );
  // ( But a '<' which the replacement text has as a reference is a character of the value. )
  {
    CursorDocument< TyChar > doc( "<!DOCTYPE doc [<!ENTITY e '&#38;#60;'><!ENTITY f '&lt;'>]><doc a='&e;&f;'/>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_TRUE( StrAttrValue( doc, 0 ) == W( "<<" ) ) << StrNarrowExact( StrAttrValue( doc, 0 ) );
  }
  // WFC: No External Entity References - and not to an unparsed entity ( 4.4.4 ).
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY x SYSTEM 'x.xml'>]><doc a='&x;'/>" ).find( "external entity" ), std::string::npos );
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY x SYSTEM 'x.xml'><!ENTITY i '&x;'>]><doc a='&i;'/>" ).find( "external entity" ), std::string::npos );
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!NOTATION n SYSTEM 'n'><!ENTITY u SYSTEM 'u.gif' NDATA n>]><doc a='&u;'/>" ).find( "unparsed entity" ), std::string::npos );
  // WFC: No Recursion.
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY a '&b;'><!ENTITY b '&a;'>]><doc a='&a;'/>" ).find( "reference to itself" ), std::string::npos );
  // WFC: Entity Declared - told when the value is had.
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY a 'b'>]><doc a='&nosuch;'/>" ), "" );
  // These are found whether or not anyone asks for the value: the tag is read.
  {
    CursorDocument< TyChar > doc( "<!DOCTYPE doc [<!ENTITY e '<b/>'>]><doc><sub a='&e;'/></doc>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_ANY_THROW( (void)doc.m_cursor.FMoveDown() );
  }
}

// What the cursor passes over - FNextTag() from above - is read if the DTD declares entities whose text is content of
//  the document: a reference to one in what is passed over is replaced as it is anywhere, and what is wrong with the
//  text of the entity - or with the reference - is found.
TYPED_TEST( DtdTest, EntitiesInContentWhichIsPassedOver )
{
  typedef TypeParam TyChar;
  // Move to the document's element and then beyond it without looking at what it has.
  auto ErrorPassingOver = []( const char * _pszDocument ) -> std::string
  {
    CursorDocument< TyChar > doc( _pszDocument );
    if ( !doc.FOpened() )
      return "not opened: " + doc.m_strError;
    try
    {
      if ( !doc.m_cursor.FMoveDown() )
        return "no element";
      if ( doc.m_cursor.FNextTag() )
        return "another element";
    }
    catch ( std::exception const & rexc )
    {
      return std::string( "exception: " ) + rexc.what();
    }
    return std::string();
  };
  EXPECT_EQ( ErrorPassingOver( "<!DOCTYPE doc [<!ENTITY e '<b>t</b>'><!ENTITY t 'text'>]><doc><sec>x &e; y &t;</sec><sec>&e;</sec><last/></doc>" ), "" );
  EXPECT_NE( ErrorPassingOver( "<!DOCTYPE doc [<!ENTITY e '<b>'>]><doc><sec>x &e; y</sec><last/></doc>" ).find( "doesn't end in it" ), std::string::npos );
  EXPECT_NE( ErrorPassingOver( "<!DOCTYPE doc [<!ENTITY e '</sec><sec>'>]><doc><sec>x &e; y</sec><last/></doc>" ).find( "doesn't begin in it" ), std::string::npos );
  EXPECT_NE( ErrorPassingOver( "<!DOCTYPE doc [<!ENTITY e '<b>x</b><c'>]><doc><sec><p>&e;</p></sec></doc>" ).find( "not well formed as content" ), std::string::npos );
  EXPECT_NE( ErrorPassingOver( "<!DOCTYPE doc [<!ENTITY a '<x>&b;</x>'><!ENTITY b 'y &a;'>]><doc><sec>&a;</sec></doc>" ).find( "reference to itself" ), std::string::npos );
  EXPECT_NE( ErrorPassingOver( "<!DOCTYPE doc [<!NOTATION n SYSTEM 'n'><!ENTITY u SYSTEM 'u.gif' NDATA n>]><doc><sec>&u;</sec></doc>" ).find( "unparsed entity" ), std::string::npos );
  // An external entity is read as well - if the parser has the means.
  {
    static const char s_szDocument[] = "<!DOCTYPE doc [<!ENTITY chap SYSTEM 'chap.xml'>]><doc><sec>&chap;</sec></doc>";
    TestEntities entities;
    entities.m_mapEntities[ "chap.xml" ] = "<chapter>";
    CursorDocument< TyChar > doc( s_szDocument, entities.Resolver< TyChar >(), "doc.xml" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_ANY_THROW( (void)doc.m_cursor.FNextTag() );
    EXPECT_EQ( entities.m_rgstrAsked, ( std::vector< std::string >{ "chap.xml <- doc.xml" } ) );
    // Without the means it is not: nothing is known of what it has.
    EXPECT_EQ( ErrorPassingOver( s_szDocument ), "" );
  }
}

// A document is validated the same whatever the cursor is asked to pass over or to filter: a comment, a processing
//  instruction or white space is content which an element may not have - one which is declared EMPTY - and the
//  validator knows of it though the user is not given it.
TYPED_TEST( DtdTest, ValidationOfWhatIsFiltered )
{
  typedef TypeParam TyChar;
  static const char s_szDoctype[] = "<!DOCTYPE doc [<!ELEMENT doc (e, f)><!ELEMENT e EMPTY><!ELEMENT f (#PCDATA)>]>";
  // _fFilter: comments and processing instructions are passed over and white space is filtered.
  auto Validate = []( const char * _pszRoot, bool _fFilter ) -> std::string
  {
    CursorDocument< TyChar, true, true > doc( ( std::string( s_szDoctype ) + _pszRoot ).c_str() );
    if ( !doc.FOpened() )
      return "not opened: " + doc.m_strError;
    doc.m_cursor.SetSkipComments( _fFilter );
    doc.m_cursor.SetSkipProcessingInstructions( _fFilter );
    doc.m_cursor.SetFilterWhitespaceCharData( _fFilter );
    try
    {
      for ( size_t nDepth = 0; ; )
      {
        if ( doc.m_cursor.FMoveDown() )
        {
          ++nDepth;
          continue;
        }
        if ( !nDepth )
          break;
        if ( !doc.m_cursor.FNextTag() )
          --nDepth;
      }
    }
    catch ( std::exception const & rexc )
    {
      return std::string( "exception: " ) + rexc.what();
    }
    std::string strErrors;
    for ( std::string const & rstrError : doc.m_cursor.RgValidityErrors() )
      strErrors += rstrError + "\n";
    return strErrors;
  };
  static const char s_szValid[] = "<doc> <!--c--> <e/> <?p q?> <f>t</f> </doc>";
  static const char s_szComment[] = "<doc><e><!--c--></e><f/></doc>";
  static const char s_szInstruction[] = "<doc><e><?p q?></e><f/></doc>";
  static const char s_szWhiteSpace[] = "<doc><e> </e><f/></doc>";
  for ( const bool kfFilter : { false, true } )
  {
    EXPECT_EQ( Validate( s_szValid, kfFilter ), "" ) << kfFilter;
    EXPECT_EQ( Validate( s_szComment, kfFilter ), "The element <e> is declared EMPTY and has a comment or a processing instruction.\n" ) << kfFilter;
    EXPECT_EQ( Validate( s_szInstruction, kfFilter ), "The element <e> is declared EMPTY and has a comment or a processing instruction.\n" ) << kfFilter;
    EXPECT_EQ( Validate( s_szWhiteSpace, kfFilter ), "The element <e> is declared EMPTY and has white space.\n" ) << kfFilter;
  }
  // And the user is not given what was to be passed over.
  {
    CursorDocument< TyChar, true, true > doc( ( std::string( s_szDoctype ) + s_szValid ).c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    doc.m_cursor.SetSkipComments();
    doc.m_cursor.SetSkipProcessingInstructions();
    doc.m_cursor.SetFilterWhitespaceCharData();
    ASSERT_TRUE( doc.m_cursor.FMoveDown() ); // doc
    EXPECT_EQ( StrContentTokens( doc ), "" );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() ); // e
    ASSERT_TRUE( doc.m_cursor.FNextTag() ); // f
    EXPECT_EQ( StrContentTokens( doc ), "[t]" );
    ASSERT_FALSE( doc.m_cursor.FNextTag() );
    EXPECT_EQ( StrContentTokens( doc ), "" );
    EXPECT_TRUE( doc.m_cursor.FIsValid() );
  }
  {
    CursorDocument< TyChar, true, true > doc( ( std::string( s_szDoctype ) + s_szValid ).c_str() );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() ); // doc
    EXPECT_EQ( StrContentTokens( doc ), "[ ][c][ ]" );
  }
}

// The variants - with XML traits which support the DTD and validate:
//  - xml_parser<> whose transport is a variant of the transports of a type of character ( xml_var_get_var_transport_t<> ).
//  - xml_parser_var<>, xml_read_cursor_var<> and xml_document_var<>: variants of the parsers, cursors and documents of
//    each type of character - one type reads a document in whichever encoding it is.
static const char s_szVariantDocument[] =
  "<!DOCTYPE doc [\n"
  "<!ELEMENT doc (item+, note?)>\n"
  "<!ELEMENT item (#PCDATA|b)*>\n"
  "<!ELEMENT b (#PCDATA)>\n"
  "<!ELEMENT note EMPTY>\n"
  "<!ATTLIST item kind (a|b) 'a' id ID #IMPLIED>\n"
  "<!ENTITY t 'text'>\n"
  "<!ENTITY m 'with <b>bold &t;</b>'>\n"
  "]>\n"
  "<doc><item>one &t;</item><item kind='b' id=' i2 '>two &m; end</item><note/></doc>\n";
static const char s_szVariantExpanded[] =
  "\n<doc><item kind='a'>one text</item><item kind='b' id='i2'>two with <b>bold text</b> end</item><note/></doc>\n";
// The same and not valid: an element which is not declared, and <note> is not empty.
static const char s_szVariantNotValid[] =
  "<!DOCTYPE doc [\n"
  "<!ELEMENT doc (item+, note?)>\n"
  "<!ELEMENT item (#PCDATA)>\n"
  "<!ELEMENT note EMPTY>\n"
  "]>\n"
  "<doc><item>one <c/></item><note>x</note></doc>\n";

// The text of a document as it is in memory: with the byte order mark by which its encoding is known.
template < class t_TyChar >
std::basic_string< t_TyChar > StrDocumentText( const char * _pszText )
{
  std::basic_string< t_TyChar > strText;
  if ( sizeof( t_TyChar ) > 1 )
    strText += t_TyChar( 0xFEFF );
  strText.append( _pszText, _pszText + strlen( _pszText ) );
  return strText;
}
// The document as the writer writes it from a cursor of any kind which is at the beginning of the document: in the
//  type of character t_TyChar and without a byte order mark.
template < class t_TyChar, class t_TyCursor >
std::basic_string< t_TyChar > StrWriteFromAnyCursor( t_TyCursor & _rcursor, bool _fWriteDoctypeDecl = true )
{
  typedef TyTestXmlWriter< t_TyChar > _TyXmlWriter;
  _TyXmlWriter xwWriter;
  xwWriter.SetWriteBOM( false );
  xwWriter.SetWriteXMLDecl( !_rcursor.FPseudoXMLDecl() );
  xwWriter.SetWriteDoctypeDecl( _fWriteDoctypeDecl );
  xwWriter.SetUseNamespaces( _rcursor.FUseXMLNamespaces() );
  typename _TyXmlWriter::_TyXMLDeclProperties xdp;
  _rcursor.GetXMLDeclProperties( xdp );
  xwWriter.OpenMemFile( xdp, nullptr, true );
  xwWriter.NWriteFromReadCursor( _rcursor );
  return StrWritten< t_TyChar >( xwWriter );
}
// Whether what the cursor reads is the same document as _pszExpanded read without any DTD - and how it isn't.
template < class t_TyCursor >
std::string StrDifferenceFromExpanded( t_TyCursor & _rcursor, const char * _pszExpanded )
{
  CursorDocument< char8_t, false > docExpanded( _pszExpanded );
  if ( !docExpanded.FOpened() )
    return "the expanded document wasn't opened: " + docExpanded.m_strError;
  std::string strDifference;
  if ( FCompareXmlDocuments( _rcursor, docExpanded.m_cursor, xml_compare_options(), &strDifference ) )
    return std::string();
  return strDifference.empty() ? std::string( "different" ) : strDifference;
}

TYPED_TEST( DtdTest, VariantTransport )
{
  typedef TypeParam TyChar;
  typedef xml_traits< xml_var_get_var_transport_t< TyChar >, true, true > TyXmlTraits;
  typedef xml_parser< TyXmlTraits > TyXmlParser;
  typedef typename TyXmlParser::_TyReadCursor TyReadCursor;
  const std::basic_string< TyChar > strText = StrDocumentText< TyChar >( s_szVariantDocument );
  // What the DTD gives the document - the default of an attribute, the type of one, the text of its entities - and it is valid.
  {
    TyXmlParser xmlParser;
    TyReadCursor cursor = xmlParser.OpenMemoryVar( strText.data(), strText.length() * sizeof( TyChar ) );
    ASSERT_TRUE( cursor.FHasDtd() );
    EXPECT_EQ( StrDifferenceFromExpanded( cursor, s_szVariantExpanded ), "" );
    EXPECT_TRUE( cursor.FIsValid() ) << ( cursor.RgValidityErrors().empty() ? std::string() : cursor.RgValidityErrors().front() );
  }
  // It is written as it was read - and without its document type declaration as what that made it.
  {
    TyXmlParser xmlParser;
    TyReadCursor cursor = xmlParser.OpenMemoryVar( strText.data(), strText.length() * sizeof( TyChar ) );
    EXPECT_EQ( StrNarrowExact( StrWriteFromAnyCursor< TyChar >( cursor ) ), s_szVariantDocument );
  }
  {
    TyXmlParser xmlParser;
    TyReadCursor cursor = xmlParser.OpenMemoryVar( strText.data(), strText.length() * sizeof( TyChar ) );
    EXPECT_EQ( StrNarrowExact( StrWriteFromAnyCursor< TyChar >( cursor, false ) ),
      "\n<doc><item kind=\"a\">one text</item><item kind='b' id='i2'>two with <b>bold text</b> end</item><note/></doc>\n" );
  }
  // Read into a tree - and the tree is written when the parser and the cursor are gone.
  {
    auto prXmlDoc = xml_document< TyXmlTraits >::PrCreateXmlDocument();
    {
      TyXmlParser xmlParser;
      TyReadCursor cursor = xmlParser.OpenMemoryVar( strText.data(), strText.length() * sizeof( TyChar ) );
      ASSERT_NO_THROW( prXmlDoc.first->FromXmlStream( cursor, prXmlDoc.second ) );
    }
    typedef TyTestXmlWriter< TyChar > TyXmlWriter;
    TyXmlWriter xwWriter;
    xwWriter.SetWriteBOM( false );
    xwWriter.SetWriteXMLDecl( false );
    typename TyXmlWriter::_TyXMLDeclProperties xdp( prXmlDoc.first->GetXMLDeclProperties() );
    xwWriter.OpenMemFile( xdp, nullptr, true );
    ASSERT_NO_THROW( prXmlDoc.first->ToXmlStream( xwWriter ) );
    EXPECT_EQ( StrNarrowExact( StrWritten< TyChar >( xwWriter ) ), s_szVariantDocument );
  }
  // A document which is not valid.
  {
    const std::basic_string< TyChar > strNotValid = StrDocumentText< TyChar >( s_szVariantNotValid );
    TyXmlParser xmlParser;
    TyReadCursor cursor = xmlParser.OpenMemoryVar( strNotValid.data(), strNotValid.length() * sizeof( TyChar ) );
    ASSERT_TRUE( cursor.FMoveDown() );
    (void)cursor.FNextTag();
    EXPECT_FALSE( cursor.FIsValid() );
    EXPECT_EQ( cursor.RgValidityErrors().size(), 3u );
  }
}

typedef xml_parser_var< xml_var_get_var_transport_t, tuple< tuple< char8_t >, tuple< char16_t >, tuple< char32_t > >, true, true > TyXmlParserVarDtd;
typedef xml_document_var< TyXmlParserVarDtd::_TyTpTransports, true, true > TyXmlDocumentVarDtd;

TYPED_TEST( DtdTest, VariantParser )
{
  typedef TypeParam TyChar; // The encoding of the document - the parser is the same type for each.
  typedef TyXmlParserVarDtd::_TyReadCursorVar TyReadCursorVar;
  const std::basic_string< TyChar > strText = StrDocumentText< TyChar >( s_szVariantDocument );
  {
    TyXmlParserVarDtd xmlParser;
    TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strText.data(), strText.length() * sizeof( TyChar ) );
    ASSERT_TRUE( cursor.FHasDtd() );
    EXPECT_EQ( StrDifferenceFromExpanded( cursor, s_szVariantExpanded ), "" );
    EXPECT_TRUE( cursor.FIsValid() ) << ( cursor.RgValidityErrors().empty() ? std::string() : cursor.RgValidityErrors().front() );
    EXPECT_TRUE( cursor.RgValidityErrorsOfDtd().empty() );
    EXPECT_FALSE( cursor.FValidationIncomplete() );
  }
  // Written as it was read - in the encoding which it has and in another.
  {
    TyXmlParserVarDtd xmlParser;
    TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strText.data(), strText.length() * sizeof( TyChar ) );
    EXPECT_EQ( StrNarrowExact( StrWriteFromAnyCursor< TyChar >( cursor ) ), s_szVariantDocument );
  }
  {
    typedef std::conditional_t< sizeof( TyChar ) == sizeof( char8_t ), char32_t, char8_t > TyCharOther;
    TyXmlParserVarDtd xmlParser;
    TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strText.data(), strText.length() * sizeof( TyChar ) );
    EXPECT_EQ( StrNarrowExact( StrWriteFromAnyCursor< TyCharOther >( cursor ) ), s_szVariantDocument );
  }
  // Read into a tree - and the tree is written when the parser and the cursor are gone.
  {
    auto prXmlDoc = TyXmlDocumentVarDtd::PrCreateXmlDocument();
    {
      TyXmlParserVarDtd xmlParser;
      TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strText.data(), strText.length() * sizeof( TyChar ) );
      ASSERT_NO_THROW( prXmlDoc.first->FromXmlStream( cursor, prXmlDoc.second ) );
    }
    typedef TyTestXmlWriter< char8_t > TyXmlWriter;
    TyXmlWriter xwWriter;
    xwWriter.SetWriteBOM( false );
    xwWriter.SetWriteXMLDecl( false );
    typename TyXmlWriter::_TyXMLDeclProperties xdp;
    prXmlDoc.first->GetXMLDeclProperties( xdp );
    xwWriter.OpenMemFile( xdp, nullptr, true );
    ASSERT_NO_THROW( prXmlDoc.first->ToXmlStream( xwWriter ) );
    EXPECT_EQ( StrNarrowExact( StrWritten< char8_t >( xwWriter ) ), s_szVariantDocument );
  }
  // A document which is not valid.
  {
    const std::basic_string< TyChar > strNotValid = StrDocumentText< TyChar >( s_szVariantNotValid );
    TyXmlParserVarDtd xmlParser;
    TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strNotValid.data(), strNotValid.length() * sizeof( TyChar ) );
    ASSERT_TRUE( cursor.FMoveDown() );
    (void)cursor.FNextTag();
    EXPECT_FALSE( cursor.FIsValid() );
    EXPECT_EQ( cursor.RgValidityErrors().size(), 3u );
  }
  // External entities are read if the parser is told to - before the document is opened: here by a resolver for the
  //  type of character of the document.
  {
    static const char s_szDocument[] = "<!DOCTYPE doc SYSTEM 'doc.dtd' [<!ENTITY chap SYSTEM 'chap.xml'>]><doc>&chap;</doc>";
    const std::basic_string< TyChar > strExternal = StrDocumentText< TyChar >( s_szDocument );
    TestEntities entities;
    entities.m_mapEntities[ "doc.dtd" ] = "<!ELEMENT doc (c)><!ELEMENT c (#PCDATA)><!ATTLIST doc d CDATA 'dflt'>";
    entities.m_mapEntities[ "chap.xml" ] = "<c>chapter</c>";
    {
      TyXmlParserVarDtd xmlParser;
      xmlParser.SetExternalEntityResolver( entities.Resolver< TyChar >() );
      xmlParser.SetBaseLocation( "base/doc.xml" );
      TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strExternal.data(), strExternal.length() * sizeof( TyChar ) );
      EXPECT_EQ( StrDifferenceFromExpanded( cursor, "<doc d='dflt'><c>chapter</c></doc>" ), "" );
      EXPECT_TRUE( cursor.FIsValid() );
      EXPECT_EQ( entities.m_rgstrAsked, ( std::vector< std::string >{ "doc.dtd <- base/doc.xml", "chap.xml <- base/doc.xml" } ) );
    }
    // Not told to: they are not read.
    {
      TyXmlParserVarDtd xmlParser;
      TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strExternal.data(), strExternal.length() * sizeof( TyChar ) );
      ASSERT_TRUE( cursor.FMoveDown() );
      (void)cursor.FNextTag();
      EXPECT_TRUE( cursor.FValidationIncomplete() );
    }
  }
}

// A document which is a file, read through each of the transports of files with XML traits which support the DTD and
//  validate - by the parser of the transport and by the variant parser. Returns how what is read is not _pszExpanded -
//  or is not valid: empty if nothing.
template < class t_TyTransport >
std::string StrReadDocumentFile( std::string const & _rstrPath, const char * _pszExpanded )
{
  typedef xml_traits< t_TyTransport, true, true > TyXmlTraits;
  try
  {
    xml_parser< TyXmlTraits > xmlParser;
    xmlParser.SetReadExternalEntityFiles(); // They are relative to the file which is opened.
    xml_read_cursor< TyXmlTraits > cursor = xmlParser.OpenFile( _rstrPath.c_str() );
    const std::string strDifference = StrDifferenceFromExpanded( cursor, _pszExpanded );
    if ( !strDifference.empty() )
      return strDifference;
    return cursor.FIsValid() ? std::string() : ( "not valid: " + ( cursor.RgValidityErrors().empty() ? std::string( "the DTD" ) : cursor.RgValidityErrors().front() ) );
  }
  catch ( std::exception const & rexc )
  {
    return std::string( "exception: " ) + rexc.what();
  }
}
template < template < class ... > class t_tempTransport >
std::string StrReadDocumentFileVar( std::string const & _rstrPath, const char * _pszExpanded )
{
  try
  {
    TyXmlParserVarDtd xmlParser;
    xmlParser.SetReadExternalEntityFiles();
    TyXmlParserVarDtd::_TyReadCursorVar cursor = xmlParser.template OpenFileVar< t_tempTransport >( _rstrPath.c_str() );
    const std::string strDifference = StrDifferenceFromExpanded( cursor, _pszExpanded );
    if ( !strDifference.empty() )
      return strDifference;
    return cursor.FIsValid() ? std::string() : ( "not valid: " + ( cursor.RgValidityErrors().empty() ? std::string( "the DTD" ) : cursor.RgValidityErrors().front() ) );
  }
  catch ( std::exception const & rexc )
  {
    return std::string( "exception: " ) + rexc.what();
  }
}
TEST( DtdExternalFiles, DocumentsWhichAreFiles )
{
  namespace fs = std::filesystem;
  const fs::path pathDir = fs::temp_directory_path() / "xmlpgtest_dtd_documents";
  std::error_code ec;
  fs::remove_all( pathDir, ec );
  ASSERT_TRUE( fs::create_directories( pathDir, ec ) ) << ec.message();
  auto WriteFile = []( fs::path const & _rpath, std::string const & _rstrBytes )
  {
    std::ofstream ofs( _rpath, std::ios::binary );
    ofs.write( _rstrBytes.data(), std::streamsize( _rstrBytes.size() ) );
  };
  auto Utf16 = []( const char * _psz, bool _fLittleEndian )
  {
    std::string strBytes( _fLittleEndian ? "\xff\xfe" : "\xfe\xff" );
    for ( ; !!*_psz; ++_psz )
    {
      strBytes += _fLittleEndian ? *_psz : '\0';
      strBytes += _fLittleEndian ? '\0' : *_psz;
    }
    return strBytes;
  };
  // An external subset, an external entity whose text is content, an entity with markup and the default of an attribute.
  static const char s_szDocument[] =
    "<!DOCTYPE doc SYSTEM 'doc.dtd' [<!ENTITY chap SYSTEM 'chap.xml'><!ENTITY m '<b>bold</b>'>]>\r\n"
    "<doc>one &m; two &chap;</doc>\r\n";
  static const char s_szExpanded[] = "\n<doc d='dflt'>one <b>bold</b> two <c>chapter</c></doc>\n";
  WriteFile( pathDir / "doc.dtd", "<!ELEMENT doc (#PCDATA|b|c)*><!ELEMENT b (#PCDATA)><!ELEMENT c (#PCDATA)><!ATTLIST doc d CDATA 'dflt'>" );
  WriteFile( pathDir / "chap.xml", "<c>chapter</c>" );
  WriteFile( pathDir / "doc8.xml", s_szDocument );
  WriteFile( pathDir / "doc16le.xml", Utf16( s_szDocument, true ) );
  WriteFile( pathDir / "doc16be.xml", Utf16( s_szDocument, false ) );
  const std::string strPath8 = ( pathDir / "doc8.xml" ).generic_string();
  const std::string strPath16LE = ( pathDir / "doc16le.xml" ).generic_string();
  const std::string strPath16BE = ( pathDir / "doc16be.xml" ).generic_string();
  // A file which is in the byte order of this machine is read as it is and the other with its bytes switched.
  typedef std::integral_constant< bool, vkfIsBigEndian > TyFSwitchLE;
  typedef std::integral_constant< bool, vkfIsLittleEndian > TyFSwitchBE;
  // The parser of each transport.
  EXPECT_EQ( ( StrReadDocumentFile< _l_transport_file< char8_t, false_type > >( strPath8, s_szExpanded ) ), "" );
  EXPECT_EQ( ( StrReadDocumentFile< _l_transport_mapped< char8_t, false_type > >( strPath8, s_szExpanded ) ), "" );
  EXPECT_EQ( ( StrReadDocumentFile< _l_transport_file< char16_t, TyFSwitchLE > >( strPath16LE, s_szExpanded ) ), "" );
  EXPECT_EQ( ( StrReadDocumentFile< _l_transport_mapped< char16_t, TyFSwitchLE > >( strPath16LE, s_szExpanded ) ), "" );
  EXPECT_EQ( ( StrReadDocumentFile< _l_transport_file< char16_t, TyFSwitchBE > >( strPath16BE, s_szExpanded ) ), "" );
  EXPECT_EQ( ( StrReadDocumentFile< _l_transport_mapped< char16_t, TyFSwitchBE > >( strPath16BE, s_szExpanded ) ), "" );
  // The variant parser - whatever the encoding of the file.
  for ( std::string const & rstrPath : { strPath8, strPath16LE, strPath16BE } )
  {
    EXPECT_EQ( StrReadDocumentFileVar< _l_transport_file >( rstrPath, s_szExpanded ), "" ) << rstrPath;
    EXPECT_EQ( StrReadDocumentFileVar< _l_transport_mapped >( rstrPath, s_szExpanded ), "" ) << rstrPath;
  }
  fs::remove_all( pathDir, ec );
}

// What the writer does with strings which can't be written as they are - other than those of WriterStringValues.
TYPED_TEST( DtdTest, WriterStringsOfSectionsAndDeclarations )
{
  typedef TypeParam TyChar;
  typedef CursorDocument< TyChar, false > TyCursorDocument;
  typedef typename TyCursorDocument::_TyXmlTraits::_TyXmlToken TyXmlToken;
  auto W = []( const char * _psz ) { return StrWiden< TyChar >( _psz ); };
  // A CDATA section whose string has a #xD: as itself it would be read as the end of a line - #xA ( 2.11 ) - and there
  //  is no reference within a section: the section is ended, the reference is written and the section is begun again.
  auto WriteSection = [&W]( const char * _pszSection ) -> std::string
  {
    TyCursorDocument doc( "<doc/>" );
    VerifyThrowSz( doc.FOpened(), "%s", doc.m_strError.c_str() );
    TyTestXmlWriter< TyChar > xwWriter;
    xwWriter.SetWriteBOM( false );
    xwWriter.SetWriteXMLDecl( false );
    xwWriter.OpenMemFile();
    VerifyThrow( doc.m_cursor.FMoveDown() );
    {
      auto xwtDoc = xwWriter.StartTag( std::move( doc.m_cursor.GetTagCur() ), true );
      xwtDoc.Commit();
      TyXmlToken xtSection( doc.m_xmlParser.GetUserObj(), s_knTokenCDataSection );
      xtSection.GetValue().emplaceVal( W( _pszSection ) );
      xwWriter.WriteToken( xtSection );
    } // The tag is ended.
    return StrNarrowExact( StrWritten< TyChar >( xwWriter ) );
  };
  auto ContentReadBack = []( std::string const & _rstrWritten ) -> std::string
  {
    TyCursorDocument doc( _rstrWritten.c_str() );
    if ( !doc.FOpened() || !doc.m_cursor.FMoveDown() )
      return "not read: " + doc.m_strError;
    return StrNarrowExact( doc.StrContent() );
  };
  {
    static const char s_szSection[] = "a\rb\r\nc <&> \r";
    const std::string strWritten = WriteSection( s_szSection );
    EXPECT_EQ( strWritten, "<doc><![CDATA[a]]>&#13;<![CDATA[b]]>&#13;<![CDATA[\nc <&> ]]>&#13;<![CDATA[]]></doc>" );
    EXPECT_EQ( ContentReadBack( strWritten ), s_szSection );
  }
  // With the "]]>" which a section can't have either.
  {
    static const char s_szSection[] = "x]]>\r]]>y";
    const std::string strWritten = WriteSection( s_szSection );
    EXPECT_EQ( strWritten, "<doc><![CDATA[x]]]]><![CDATA[>]]>&#13;<![CDATA[]]]]><![CDATA[>y]]></doc>" );
    EXPECT_EQ( ContentReadBack( strWritten ), s_szSection );
  }
  EXPECT_EQ( WriteSection( "plain\ntext" ), "<doc><![CDATA[plain\ntext]]></doc>" );

  // The text of a document type declaration which is a string is parsed: it must be one - all of it and nothing more.
  auto ErrorWritingDoctype = [&W]( const char * _pszDoctype ) -> std::string
  {
    TyCursorDocument doc( "<doc/>" );
    VerifyThrowSz( doc.FOpened(), "%s", doc.m_strError.c_str() );
    TyXmlToken xtDoctype( doc.m_xmlParser.GetUserObj(), s_knTokenDoctypeDecl );
    xtDoctype.GetValue().emplaceVal( W( _pszDoctype ) );
    TyTestXmlWriter< TyChar > xwWriter;
    xwWriter.SetWriteBOM( false );
    xwWriter.SetWriteXMLDecl( false );
    xwWriter.OpenMemFile();
    try
    {
      xwWriter.WriteToken( xtDoctype );
    }
    catch ( std::exception const & rexc )
    {
      return std::string( "exception: " ) + rexc.what();
    }
    return ( StrNarrowExact( StrWritten< TyChar >( xwWriter ) ) == _pszDoctype ) ? std::string() : std::string( "not written as it is" );
  };
  EXPECT_EQ( ErrorWritingDoctype( "<!DOCTYPE doc>" ), "" );
  EXPECT_EQ( ErrorWritingDoctype( "<!DOCTYPE doc SYSTEM 'doc.dtd' [\n<!ELEMENT doc (a,(b|c)*)>\n<!ATTLIST doc a CDATA 'v'>\n<!-- c -->\n<!ENTITY e 'text'>\n]\n>" ), "" );
  // It begins and ends as one and isn't one.
  EXPECT_NE( ErrorWritingDoctype( "<!DOCTYPE doc [<!ELEMENT doc (a,(b|c)*>]>" ), "" ); // A parenthesis which isn't closed.
  EXPECT_NE( ErrorWritingDoctype( "<!DOCTYPE doc [<!ATTLIST doc a CDATA>]>" ), "" ); // No default.
  EXPECT_NE( ErrorWritingDoctype( "<!DOCTYPE doc [<!ENTITY e 'text'>]><doc>" ), "" ); // More than the declaration.
  EXPECT_NE( ErrorWritingDoctype( "<!DOCTYPE doc [<!ENTITY e 'text'>]> " ), "" );
  EXPECT_NE( ErrorWritingDoctype( " <!DOCTYPE doc>" ), "" );
  EXPECT_NE( ErrorWritingDoctype( "<!DOCTYPE doc [<!ENTITY % p '<!ELEMENT doc ANY>'><!ELEMENT a (%p;)>]>" ), "" ); // WFC: PEs in Internal Subset.
  EXPECT_NE( ErrorWritingDoctype( "" ), "" );
}

// Namespaces in XML - xml_parser<>::SetUseXMLNamespaces():
//  - With them - which is so unless the parser is told otherwise - the names of tags and of attributes are qualified
//    names, and a document which is not namespace-well-formed is an error.
//  - Without them the document is one of XML 1.0 alone: any Name is a name and "xmlns" is an attribute as any other.
TYPED_TEST( DtdTest, NamespacesOrNot )
{
  typedef TypeParam TyChar;
  // What reading all of the document throws - empty if nothing.
  auto ErrorReading = []( const char * _pszDocument, bool _fUseXMLNamespaces ) -> std::string
  {
    CursorDocument< TyChar > docLeft( _pszDocument, nullptr, nullptr, _fUseXMLNamespaces );
    CursorDocument< TyChar > docRight( _pszDocument, nullptr, nullptr, _fUseXMLNamespaces );
    if ( !docLeft.FOpened() || !docRight.FOpened() )
      return "not opened: " + docLeft.m_strError;
    try
    {
      std::string strDifference;
      if ( !FCompareXmlDocuments( docLeft.m_cursor, docRight.m_cursor, xml_compare_options(), &strDifference ) )
        return "not the same as itself: " + strDifference;
    }
    catch ( std::exception const & rexc )
    {
      return std::string( "exception: " ) + rexc.what();
    }
    return std::string();
  };
  // Names which are Names [5] of XML and not qualified names [7] of Namespaces in XML: a colon at the beginning or at
  //  the end, more than one colon, and a local part which doesn't begin as a name does.
  static const char * const s_rgpszNotQualified[] = {
    "<a:b:c xmlns:a='u'/>", "<:a/>", "<a: xmlns:a='u'/>", "<:/>", "<doc :='v'/>", "<doc a:='v' xmlns:a='u'/>", "<doc :a='v'/>",
    "<doc xmlns:a='u' a:b:c='v'/>", "<a:1b xmlns:a='u'/>", "<doc xmlns:a='u' a:-b='v'/>", "<doc xmlns:='u'/>", "<doc><a:b:c xmlns:a='u'></a:b:c></doc>",
    "<a::b xmlns:a='u'/>" };
  for ( const char * pszDocument : s_rgpszNotQualified )
  {
    EXPECT_NE( ErrorReading( pszDocument, true ).find( "is not a qualified name" ), std::string::npos ) << pszDocument << ": " << ErrorReading( pszDocument, true );
    EXPECT_EQ( ErrorReading( pszDocument, false ), "" ) << pszDocument;
  }
  // Qualified names - and those of a document which has no colon at all - are names either way.
  static const char * const s_rgpszQualified[] = {
    "<doc a='1' b-c.d='2'>text<e/></doc>", "<p:a xmlns:p='u' p:x='1' y='2'><p:b/></p:a>", "<doc xmlns='u' xml:lang='en'><_a.b-c x1='1'/></doc>" };
  for ( const char * pszDocument : s_rgpszQualified )
  {
    EXPECT_EQ( ErrorReading( pszDocument, true ), "" ) << pszDocument;
    EXPECT_EQ( ErrorReading( pszDocument, false ), "" ) << pszDocument;
  }
  // Without namespaces: the names are as they are written, a prefix is not one - nothing declares it - and a
  //  declaration of a namespace is an attribute as any other.
  {
    CursorDocument< TyChar > doc( "<a:b:c x:y='1' xmlns:q='u' :='2' z:='3'><:d/><e:></e:></a:b:c>", nullptr, nullptr, false );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    EXPECT_FALSE( doc.m_cursor.FUseXMLNamespaces() );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( StrNarrow( doc.m_cursor.SvTagCur() ), "a:b:c" );
    EXPECT_EQ( doc.StrAttributes(), "x:y=\"1\" xmlns:q=\"u\" :=\"2\" z:=\"3\"" );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( StrNarrow( doc.m_cursor.SvTagCur() ), ":d" );
    ASSERT_TRUE( doc.m_cursor.FNextTag() );
    EXPECT_EQ( StrNarrow( doc.m_cursor.SvTagCur() ), "e:" );
  }
  // The end tag is to be the name of the start tag - all of it.
  EXPECT_NE( ErrorReading( "<doc><e:></e></doc>", false ), "" );
  EXPECT_NE( ErrorReading( "<doc><e></e:></doc>", false ), "" );
  EXPECT_NE( ErrorReading( "<doc><a:b></a:c></doc>", false ), "" );
  // The same name twice is the same attribute twice.
  EXPECT_NE( ErrorReading( "<doc a:b='1' a:b='2'/>", false ), "" );
  // Without namespaces the document is written as it was read.
  {
    static const char s_szDocument[] = "<a:b:c x:y='1' xmlns:q=\"u\" :='2' z:='3'>text<:d/><e:>more</e:><?t:u v?></a:b:c>";
    CursorDocument< TyChar > doc( s_szDocument, nullptr, nullptr, false );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    std::basic_string< TyChar > strWritten;
    ASSERT_NO_THROW( strWritten = StrWriteFromCursor( doc ) );
    EXPECT_EQ( StrNarrowExact( strWritten ), s_szDocument );
  }
  // A writer which is not told that the document is one without namespaces - it uses them unless it is told - writes
  //  it as it was read all the same: the tokens of the cursor say of each name that it is in no namespace.
  {
    static const char s_szDocument[] = "<a:b:c x:y='1' xmlns:q=\"u\" :='2' z:='3'>text<:d/><e:>more</e:></a:b:c>";
    CursorDocument< TyChar > doc( s_szDocument, nullptr, nullptr, false );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    TyTestXmlWriter< TyChar > xwWriter;
    xwWriter.SetWriteBOM( false );
    xwWriter.SetWriteXMLDecl( false );
    xwWriter.OpenMemFile();
    std::string strResult;
    try
    {
      xwWriter.NWriteFromReadCursor( doc.m_cursor );
      strResult = StrNarrowExact( StrWritten< TyChar >( xwWriter ) );
    }
    catch ( std::exception const & rexc )
    {
      strResult = std::string( "exception: " ) + rexc.what();
    }
    EXPECT_EQ( strResult, s_szDocument );
  }
  // And compared by its names as they are: a prefix is part of a name.
  {
    CursorDocument< TyChar > docLeft( "<a:x p:y='1'/>", nullptr, nullptr, false );
    CursorDocument< TyChar > docRight( "<b:x p:y='1'/>", nullptr, nullptr, false );
    ASSERT_TRUE( docLeft.FOpened() && docRight.FOpened() );
    EXPECT_FALSE( FCompareXmlDocuments( docLeft.m_cursor, docRight.m_cursor ) );
  }
  {
    CursorDocument< TyChar > docLeft( "<a:x p:y='1'/>", nullptr, nullptr, false );
    CursorDocument< TyChar > docRight( "<a:x q:y='1'/>", nullptr, nullptr, false );
    ASSERT_TRUE( docLeft.FOpened() && docRight.FOpened() );
    EXPECT_FALSE( FCompareXmlDocuments( docLeft.m_cursor, docRight.m_cursor ) );
  }

  // The reserved prefixes and namespace names ( Namespaces in XML, 3 ) - with namespaces.
  static const char * const s_rgpszReserved[] = {
    "<doc xmlns:xmlns='u'/>",
    "<doc xmlns:p='http://www.w3.org/2000/xmlns/'/>",
    "<doc xmlns='http://www.w3.org/2000/xmlns/'/>",
    "<doc xmlns:xml='u'/>",
    "<doc xmlns:p='http://www.w3.org/XML/1998/namespace'/>",
    "<doc xmlns='http://www.w3.org/XML/1998/namespace'/>" };
  for ( const char * pszDocument : s_rgpszReserved )
  {
    EXPECT_NE( ErrorReading( pszDocument, true ).find( "not namespace-well-formed" ), std::string::npos ) << pszDocument << ": " << ErrorReading( pszDocument, true );
    EXPECT_EQ( ErrorReading( pszDocument, false ), "" ) << pszDocument;
  }
  EXPECT_EQ( ErrorReading( "<doc xmlns:xml='http://www.w3.org/XML/1998/namespace' xml:space='preserve'/>", true ), "" );
  // A prefix which is not declared, one which is declared twice by a tag, and one whose declaration is taken away.
  EXPECT_NE( ErrorReading( "<q:doc/>", true ), "" );
  EXPECT_NE( ErrorReading( "<doc q:a='1'/>", true ), "" );
  EXPECT_NE( ErrorReading( "<doc xmlns:p='u' xmlns:p='v'/>", true ), "" );
  EXPECT_NE( ErrorReading( "<doc xmlns:p='u'><e xmlns:p=''/></doc>", true ), "" );
  EXPECT_EQ( ErrorReading( "<doc xmlns='u'><e xmlns=''/></doc>", true ), "" );
  // The name of a namespace is the value of the attribute which declares it - what its references stand for - and two
  //  attributes of a tag are the same attribute if they have the same local name and namespace name.
  EXPECT_EQ( ErrorReading( "<doc xmlns:a='http://x&#47;y' xmlns:b='http://x/z'><e a:x='1' b:x='2'/></doc>", true ), "" );
  EXPECT_NE( ErrorReading( "<doc xmlns:a='http://x&#47;y' xmlns:b='http://x/y'><e a:x='1' b:x='2'/></doc>", true ), "" );
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY u 'http://x/y'>]><doc xmlns:a='&u;' xmlns:b='http://x/y'><e a:x='1' b:x='2'/></doc>", true ), "" );
  EXPECT_EQ( ErrorReading( "<doc xmlns:a='http://x&#47;y' xmlns:b='http://x/y'><e a:x='1' b:x='2'/></doc>", false ), "" );
  // No colon in the target of a processing instruction, in the name of an entity or in that of a notation.
  static const char * const s_rgpszColons[] = {
    "<doc><?a:b c?></doc>", "<?a:b c?><doc/>",
    "<!DOCTYPE doc [<!ENTITY a:b 'x'>]><doc/>", "<!DOCTYPE doc [<!ENTITY % a:b 'x'>]><doc/>",
    "<!DOCTYPE doc [<!NOTATION n:m SYSTEM 'x'>]><doc/>", "<!DOCTYPE doc [<?a:b c?>]><doc/>" };
  for ( const char * pszDocument : s_rgpszColons )
  {
    EXPECT_NE( ErrorReading( pszDocument, true ).find( "not namespace-well-formed" ), std::string::npos ) << pszDocument << ": " << ErrorReading( pszDocument, true );
    EXPECT_EQ( ErrorReading( pszDocument, false ), "" ) << pszDocument;
  }
  // The DTD knows names as they are written - with namespaces or without: the declarations are those of the names.
  {
    static const char s_szDocument[] =
      "<!DOCTYPE p:doc [<!ELEMENT p:doc (p:e)*><!ELEMENT p:e EMPTY><!ATTLIST p:doc xmlns:p CDATA #FIXED 'u'><!ATTLIST p:e p:k (a|b) 'a' id ID #IMPLIED r IDREF #IMPLIED>]>"
      "<p:doc><p:e id='i1'/><p:e p:k='b' r='i1'/></p:doc>";
    for ( const bool kfUseXMLNamespaces : { true, false } )
    {
      CursorDocument< TyChar, true, true > doc( s_szDocument, nullptr, nullptr, kfUseXMLNamespaces );
      ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
      ASSERT_TRUE( doc.m_cursor.FMoveDown() );
      EXPECT_EQ( doc.StrAttributes(), "xmlns:p=\"u\"" ) << kfUseXMLNamespaces; // By its default: with namespaces it declares the prefix of the document.
      ASSERT_TRUE( doc.m_cursor.FMoveDown() );
      EXPECT_EQ( doc.StrAttributes(), "id=\"i1\" p:k=\"a\"" ) << kfUseXMLNamespaces;
      ASSERT_TRUE( doc.m_cursor.FMoveUp() );
      (void)doc.m_cursor.FNextTag(); // To the end of the document.
      EXPECT_TRUE( doc.m_cursor.FIsValid() ) << kfUseXMLNamespaces << ( doc.m_cursor.RgValidityErrors().empty() ? std::string() : doc.m_cursor.RgValidityErrors().front() );
    }
  }
  // Namespace validity: the value of an attribute whose type is ID, IDREF(S), ENTITY(IES) or NOTATION has no colon.
  {
    static const char s_szDocument[] =
      "<!DOCTYPE doc [<!ELEMENT doc (e)*><!ELEMENT e EMPTY><!ATTLIST e id ID #IMPLIED r IDREFS #IMPLIED t NMTOKEN #IMPLIED>]>"
      "<doc><e id='a:b' t='x:y'/><e r='a:b'/></doc>";
    for ( const bool kfUseXMLNamespaces : { true, false } )
    {
      CursorDocument< TyChar, true, true > doc( s_szDocument, nullptr, nullptr, kfUseXMLNamespaces );
      ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
      ASSERT_TRUE( doc.m_cursor.FMoveDown() );
      (void)doc.m_cursor.FNextTag();
      EXPECT_EQ( doc.m_cursor.FIsValid(), !kfUseXMLNamespaces );
      EXPECT_EQ( doc.m_cursor.RgValidityErrors().size(), kfUseXMLNamespaces ? 2u : 0u );
    }
  }
}

// The variant parser is told of namespaces as the parser of one type of character is.
TYPED_TEST( DtdTest, NamespacesOrNotOfVariantParser )
{
  typedef TypeParam TyChar;
  typedef TyXmlParserVarDtd::_TyReadCursorVar TyReadCursorVar;
  static const char s_szDocument[] = "<a:b:c x:y='1' :='2'><:d/></a:b:c>";
  const std::basic_string< TyChar > strText = StrDocumentText< TyChar >( s_szDocument );
  {
    TyXmlParserVarDtd xmlParser;
    xmlParser.SetUseXMLNamespaces( false );
    TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strText.data(), strText.length() * sizeof( TyChar ) );
    EXPECT_FALSE( cursor.FUseXMLNamespaces() );
    EXPECT_EQ( StrNarrowExact( StrWriteFromAnyCursor< TyChar >( cursor ) ), s_szDocument );
  }
  {
    TyXmlParserVarDtd xmlParser;
    EXPECT_TRUE( xmlParser.FUseXMLNamespaces() );
    TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strText.data(), strText.length() * sizeof( TyChar ) );
    EXPECT_TRUE( cursor.FUseXMLNamespaces() );
    EXPECT_ANY_THROW( (void)cursor.FMoveDown() );
  }
}

// What the tests of the suite found - each as a small document of its own.
TYPED_TEST( DtdTest, FoundByTheConformanceSuite )
{
  typedef TypeParam TyChar;
  typedef std::conditional_t< sizeof( TyChar ) == sizeof( char8_t ), char32_t, char8_t > TyCharOther;
  // What reading all of the document throws - empty if nothing.
  auto ErrorReading = []( const char * _pszDocument ) -> std::string
  {
    return StrDifference< TyChar, true, TyChar, true >( _pszDocument, _pszDocument );
  };
  // [1] document ::= prolog element Misc*: one document element - and only comments, processing instructions and
  //  white space after it.
  EXPECT_EQ( ErrorReading( "<doc/><!-- c --><?p i?>\n" ), "" );
  EXPECT_NE( ErrorReading( "<doc></doc><doc></doc>" ).find( "second document element" ), std::string::npos );
  EXPECT_NE( ErrorReading( "<doc/><doc/>" ).find( "second document element" ), std::string::npos );
  EXPECT_NE( ErrorReading( "<doc/><!-- c --><bad/>" ), "" );
  EXPECT_NE( ErrorReading( "<doc/></doc>" ), "" );
  EXPECT_NE( ErrorReading( "<doc></doc>text" ), "" );
  EXPECT_NE( ErrorReading( "<doc/><!ELEMENT e EMPTY>" ), "" );
  EXPECT_NE( ErrorReading( "<doc/><![CDATA[x]]>" ), "" );
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ELEMENT doc EMPTY>]><doc/><!DOCTYPE doc>" ), "" );
  // And there is one: what follows the prolog is its start tag.
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ELEMENT doc (#PCDATA)>]></doc>" ), "" );
  EXPECT_NE( ErrorReading( "<!-- c --></doc><doc>" ), "" );
  // [17]: the target of a processing instruction is not "xml" - an XML declaration is only at the beginning. ( The
  //  lexical analyzer has no token for it: what was missing is that where there is no token a document doesn't just end. )
  EXPECT_NE( ErrorReading( "<doc/>\n<?xml version='1.0'?>" ), "" );
  EXPECT_NE( ErrorReading( "<doc><?XmL x?></doc>" ), "" );
  EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<?xml x?>]><doc/>" ), "" );
  EXPECT_EQ( ErrorReading( "<doc><?xml-stylesheet x?><?xmlx y?></doc>" ), "" );
  // 4.1 WFC: Entity Declared - a document which is declared standalone and an entity which is declared in the text
  //  of a parameter entity: it is not declared.
  EXPECT_NE( ErrorReading( "<?xml version='1.0' standalone='yes'?><!DOCTYPE doc [<!ENTITY % pe \"<!ENTITY e 'text'>\">%pe;]><doc>&e;</doc>" ).find( "declared standalone" ), std::string::npos );
  EXPECT_NE( ErrorReading( "<?xml version='1.0' standalone='yes'?><!DOCTYPE doc [<!ENTITY % pe \"<!ENTITY e 'text'>\">%pe;]><doc a='&e;'/>" ).find( "declared standalone" ), std::string::npos );
  EXPECT_EQ( ErrorReading( "<?xml version='1.0' standalone='no'?><!DOCTYPE doc [<!ENTITY % pe \"<!ENTITY e 'text'>\">%pe;]><doc a='&e;'>&e;</doc>" ), "" );
  EXPECT_EQ( ErrorReading( "<?xml version='1.0' standalone='yes'?><!DOCTYPE doc [<!ENTITY e 'text'>]><doc a='&e;'>&e;</doc>" ), "" );
  // The text declaration of an external entity [77]: it has an encoding, it has no "standalone", its version - if it
  //  has one - is first and is that of the document.
  auto ErrorOfExternalEntity = []( const char * _pszEntity ) -> std::string
  {
    TestEntities entities;
    entities.m_mapEntities[ "e.xml" ] = _pszEntity;
    CursorDocument< TyChar > docLeft( "<!DOCTYPE doc [<!ENTITY e SYSTEM 'e.xml'>]><doc>&e;</doc>", entities.Resolver< TyChar >(), "doc.xml" );
    CursorDocument< TyChar > docRight( "<doc><x/></doc>" );
    if ( !docLeft.FOpened() || !docRight.FOpened() )
      return "not opened: " + docLeft.m_strError;
    try
    {
      std::string strDifference;
      return FCompareXmlDocuments( docLeft.m_cursor, docRight.m_cursor, xml_compare_options(), &strDifference ) ? std::string() : ( "different: " + strDifference );
    }
    catch ( std::exception const & rexc )
    {
      return std::string( "exception: " ) + rexc.what();
    }
  };
  EXPECT_EQ( ErrorOfExternalEntity( "<x/>" ), "" );
  EXPECT_EQ( ErrorOfExternalEntity( "<?xml encoding='UTF-8'?><x/>" ), "" );
  EXPECT_EQ( ErrorOfExternalEntity( "<?xml version=\"1.0\" encoding = 'utf-8' ?><x/>" ), "" );
  EXPECT_NE( ErrorOfExternalEntity( "<?xml version='1.0'?><x/>" ).find( "text declaration" ), std::string::npos ); // No encoding.
  EXPECT_NE( ErrorOfExternalEntity( "<?xml encoding='UTF-8' version='1.0'?><x/>" ).find( "text declaration" ), std::string::npos );
  EXPECT_NE( ErrorOfExternalEntity( "<?xml version='1.0' encoding='UTF-8' standalone='yes'?><x/>" ).find( "text declaration" ), std::string::npos );
  EXPECT_NE( ErrorOfExternalEntity( "<?xml version='1.1' encoding='UTF-8'?><x/>" ).find( "text declaration" ), std::string::npos );
  EXPECT_NE( ErrorOfExternalEntity( "<?xml encoding='8bit!'?><x/>" ).find( "text declaration" ), std::string::npos );
  // Validity: a reference to an entity among the elements of element content is white space only if the replacement
  //  text of the entity is - a reference to a character in it is not, whatever the character.
  {
    static const char s_szDoctype[] = "<!DOCTYPE doc [<!ELEMENT doc (e*)><!ELEMENT e EMPTY><!ENTITY sp ' '><!ENTITY ref '&#38;#32;'>]>";
    EXPECT_EQ( StrValidate< TyChar >( ( std::string( s_szDoctype ) + "<doc><e/>&sp;<e/></doc>" ).c_str() ), "" );
    EXPECT_EQ( StrValidate< TyChar >( ( std::string( s_szDoctype ) + "<doc><e/>&ref;<e/></doc>" ).c_str() ),
      "The element <doc> is declared to have element content and has character data.\n" );
  }
  // A character which is not a Char [2] is no character of a document - #x0 among them: it is not where the document ends.
  for ( const char32_t chNot : { char32_t( 0 ), char32_t( 0x1 ), char32_t( 0xB ), char32_t( 0xC ), char32_t( 0x1F ), char32_t( 0xFFFE ), char32_t( 0xFFFF ) } )
  {
    for ( const char * pszWhere : { "<doc>a?b</doc>", "<doc a='x?y'/>", "<doc><!--a?b--></doc>", "<doc><?p a?b?></doc>", "<doc><![CDATA[a?b]]></doc>", "<doc/>?", "<doc/>?<!-- c -->" } )
    {
      // The text with the character where its '?' which is not of the markup of a processing instruction is.
      std::basic_string< TyChar > strText = StrDocumentText< TyChar >( pszWhere );
      const size_t knPos = strText.find( TyChar( '?' ), ( std::string( pszWhere ).find( "<?p" ) != std::string::npos ) ? strText.find( TyChar( 'a' ) ) : 0 );
      ASSERT_NE( knPos, std::basic_string< TyChar >::npos );
      std::basic_string< TyChar > strChar;
      ConvertString( strChar, &chNot, 1 );
      if ( !chNot )
        strChar.assign( 1, TyChar( 0 ) );
      strText.replace( knPos, 1, strChar );
      bool fThrew = false;
      try
      {
        typedef xml_traits< _l_transport_fixedmem< TyChar >, true, false > TyXmlTraits;
        xml_parser< TyXmlTraits > xmlParser;
        xml_read_cursor< TyXmlTraits > cursor = xmlParser.OpenMemory( strText.data(), strText.length() * sizeof( TyChar ) );
        for ( size_t nDepth = 0; ; )
        {
          if ( cursor.FMoveDown() )
          {
            ++nDepth;
            continue;
          }
          if ( !nDepth )
            break;
          if ( !cursor.FNextTag() )
            --nDepth;
        }
      }
      catch ( std::exception const & )
      {
        fThrew = true;
      }
      EXPECT_TRUE( fThrew ) << "The character " << uint32_t( chNot ) << " in " << pszWhere;
    }
  }
  // The writer: a tag which has an attribute by the default of the DTD - whose name is then a string - and other
  //  attributes, written in another type of character than the document's.
  {
    static const char s_szDocument[] =
      "<!DOCTYPE doc [<!ATTLIST doc xml:space (default|preserve) 'preserve' d CDATA 'dflt' xmlns:p CDATA 'u' p:e CDATA 'pe'>]>"
      "<doc a='1' b='2'><sub c='3'/></doc>";
    for ( const bool kfWriteDoctypeDecl : { true, false } )
    {
      CursorDocument< TyChar > doc( s_szDocument );
      ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
      std::basic_string< TyCharOther > strWritten;
      ASSERT_NO_THROW( strWritten = StrWriteFromAnyCursor< TyCharOther >( doc.m_cursor, kfWriteDoctypeDecl ) ) << kfWriteDoctypeDecl;
      EXPECT_EQ( ( StrDifference< TyChar, true, TyCharOther, true >( s_szDocument, StrNarrowExact( strWritten ).c_str() ) ), "" ) << StrNarrowExact( strWritten );
    }
  }
}

// What a document may have the parser do by its DTD - xml_dtd_limits: a document's entities are the document's own,
//  and a few lines of declarations stand for more text than there is memory if nothing holds them to less.
TYPED_TEST( DtdTest, LimitsOfEntities )
{
  typedef TypeParam TyChar;
  // What reading all of the document throws - empty if nothing. _plimits: the limits of the parser - null for its defaults.
  auto ErrorReading = []( std::string const & _rstrDocument, const xml_dtd_limits * _plimits = nullptr ) -> std::string
  {
    CursorDocument< TyChar > docLeft( _rstrDocument.c_str(), nullptr, nullptr, true, _plimits );
    CursorDocument< TyChar > docRight( _rstrDocument.c_str(), nullptr, nullptr, true, _plimits );
    if ( !docLeft.FOpened() || !docRight.FOpened() )
      return "not opened: " + docLeft.m_strError;
    try
    {
      std::string strDifference;
      if ( !FCompareXmlDocuments( docLeft.m_cursor, docRight.m_cursor, xml_compare_options(), &strDifference ) )
        return "not the same as itself: " + strDifference;
    }
    catch ( std::exception const & rexc )
    {
      return std::string( "exception: " ) + rexc.what();
    }
    return std::string();
  };
  // "A billion laughs": each entity is ten of the one before it - the last stands for 3 * 10^9 characters.
  std::string strLaughs = "<!DOCTYPE doc [<!ENTITY lol0 'lol'>";
  for ( int n = 1; n <= 9; ++n )
  {
    strLaughs += "<!ENTITY lol" + std::to_string( n ) + " '";
    for ( int nRef = 0; nRef < 10; ++nRef )
      strLaughs += "&lol" + std::to_string( n - 1 ) + ";";
    strLaughs += "'>";
  }
  strLaughs += "]>";
  // Declared and not referenced it costs nothing which matters: the document is read.
  EXPECT_EQ( ErrorReading( strLaughs + "<doc>&lol3;</doc>" ), "" );
  // Referenced - in content or in the value of an attribute - it is an error, and soon.
  EXPECT_NE( ErrorReading( strLaughs + "<doc>&lol9;</doc>" ).find( "more text than a document's entities may" ), std::string::npos );
  EXPECT_NE( ErrorReading( strLaughs + "<doc a='&lol9;'/>" ).find( "than is allowed" ), std::string::npos );
  EXPECT_NE( ErrorReading( strLaughs + "<doc><e a='x &lol8; y'/></doc>" ).find( "than is allowed" ), std::string::npos );
  // The same with markup in the text of the entities: they are read as content.
  {
    std::string strMarkup = "<!DOCTYPE doc [<!ENTITY m0 '<b>lol</b>'>";
    for ( int n = 1; n <= 9; ++n )
    {
      strMarkup += "<!ENTITY m" + std::to_string( n ) + " '";
      for ( int nRef = 0; nRef < 10; ++nRef )
        strMarkup += "&m" + std::to_string( n - 1 ) + ";";
      strMarkup += "'>";
    }
    strMarkup += "]>";
    EXPECT_EQ( ErrorReading( strMarkup + "<doc>&m3;</doc>" ), "" );
    EXPECT_NE( ErrorReading( strMarkup + "<doc>&m9;</doc>" ).find( "more text than a document's entities may" ), std::string::npos );
  }
  // One entity which is large, referenced often: all that the references stand for is what is limited.
  {
    const std::string strBig = "<!DOCTYPE doc [<!ENTITY big '" + std::string( 100000, 'x' ) + "'>]>";
    std::string strFew = strBig + "<doc>", strMany = strBig + "<doc>";
    for ( int n = 0; n < 20; ++n )
      strFew += "&big;";
    for ( int n = 0; n < 400; ++n )
      strMany += "&big;<e/>";
    strFew += "</doc>";
    strMany += "</doc>";
    EXPECT_EQ( ErrorReading( strFew ), "" );
    EXPECT_NE( ErrorReading( strMany ).find( "more text than a document's entities may" ), std::string::npos );
    // The limits are the parser's: whoever opens the document says what they are.
    xml_dtd_limits limits;
    limits.m_nMaxEntityExpansion = 0;
    EXPECT_EQ( ErrorReading( strMany, &limits ), "" );
    limits.m_nMaxEntityExpansion = 150000;
    EXPECT_NE( ErrorReading( strFew, &limits ).find( "more text than a document's entities may" ), std::string::npos );
    limits = xml_dtd_limits();
    limits.m_nMaxEntityText = 1000;
    EXPECT_NE( ErrorReading( strBig + "<doc a='&big;'/>", &limits ).find( "than is allowed" ), std::string::npos );
  }
  // Entities within entities: a chain which is longer than may be.
  {
    std::string strChain = "<!DOCTYPE doc [<!ENTITY c0 'end'>";
    for ( int n = 1; n <= 200; ++n )
      strChain += "<!ENTITY c" + std::to_string( n ) + " '&c" + std::to_string( n - 1 ) + ";'>";
    strChain += "]>";
    EXPECT_EQ( ErrorReading( strChain + "<doc a='&c50;'>&c50;</doc>" ), "" );
    EXPECT_NE( ErrorReading( strChain + "<doc>&c200;</doc>" ).find( "deep" ), std::string::npos );
    EXPECT_NE( ErrorReading( strChain + "<doc a='&c200;'/>" ).find( "than is allowed" ), std::string::npos );
  }
  // Parameter entities: each is ten of the one before it - in the literal of an entity, which is where a reference to
  //  one may be within a declaration when the declaration is itself in the text of a parameter entity.
  {
    std::string strParameters = "<!DOCTYPE doc [<!ENTITY % p0 '" + std::string( 100, 'x' ) + "'>";
    for ( int n = 1; n <= 9; ++n )
    {
      strParameters += "<!ENTITY % d" + std::to_string( n ) + " \"<!ENTITY &#37; p" + std::to_string( n ) + " '";
      for ( int nRef = 0; nRef < 10; ++nRef )
        strParameters += "&#37;p" + std::to_string( n - 1 ) + ";";
      strParameters += "'>\">%d" + std::to_string( n ) + ";";
    }
    strParameters += "]><doc/>";
    const std::string strError = ErrorReading( strParameters );
    EXPECT_TRUE( ( std::string::npos != strError.find( "more than" ) ) || ( std::string::npos != strError.find( "more text" ) ) ) << strError;
  }
  // No document type declaration at all - for documents from who is not trusted.
  {
    xml_dtd_limits limits;
    limits.m_fAllowDoctypeDecl = false;
    EXPECT_EQ( ErrorReading( "<doc a='1'>text &amp; more</doc>", &limits ), "" );
    EXPECT_NE( ErrorReading( "<!DOCTYPE doc><doc/>", &limits ).find( "told to accept none" ), std::string::npos );
    EXPECT_NE( ErrorReading( "<!DOCTYPE doc [<!ENTITY e 'x'>]><doc>&e;</doc>", &limits ).find( "told to accept none" ), std::string::npos );
    EXPECT_EQ( ErrorReading( "<!DOCTYPE doc [<!ENTITY e 'x'>]><doc>&e;</doc>" ), "" );
  }
  // An external entity which is larger than may be.
  {
    TestEntities entities;
    entities.m_mapEntities[ "big.xml" ] = "<c>" + std::string( 5000, 'x' ) + "</c>";
    xml_dtd_limits limits;
    limits.m_nMaxExternalEntity = 1000;
    for ( const bool kfLimited : { false, true } )
    {
      CursorDocument< TyChar > doc( "<!DOCTYPE doc [<!ENTITY e SYSTEM 'big.xml'>]><doc>&e;</doc>", entities.Resolver< TyChar >(), "doc.xml", true, kfLimited ? &limits : nullptr );
      ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
      if ( kfLimited )
        EXPECT_ANY_THROW( (void)doc.m_cursor.FMoveDown() );
      else
        EXPECT_NO_THROW( (void)doc.m_cursor.FMoveDown() );
    }
  }
}

// How deep DeepNesting nests a content model: 100000, or XMLPTEST_DEEP_NESTING_DEPTH - to try it smaller first.
size_t NDeepNestingDepth()
{
  const char * psz = getenv( "XMLPTEST_DEEP_NESTING_DEPTH" );
  const size_t kn = !psz ? 0 : size_t( strtoull( psz, nullptr, 10 ) );
  return !kn ? 100000 : kn;
}
// A content particle as it is written: "(a,(b|c)*)+" - made without calling a function for each level.
template < class t_TyChar >
std::string StrParticleTree( xml_dtd_content_particle< t_TyChar > const & _rcp )
{
  static const char * const s_rgpszOccurrence[] = { "", "?", "*", "+" };
  std::string str;
  // Each particle is begun when it is reached and ended once its children are: the stack has where we are in each.
  std::vector< std::pair< const xml_dtd_content_particle< t_TyChar > *, size_t > > rgStack{ { &_rcp, 0 } };
  for ( bool fBegun = false; !rgStack.empty(); )
  {
    auto & [ pcp, nChild ] = rgStack.back();
    if ( !fBegun )
    {
      if ( exdpName == pcp->m_eParticle )
      {
        str += StrNarrow( pcp->m_strName ) + s_rgpszOccurrence[ pcp->m_eOccurrence ];
        rgStack.pop_back();
        fBegun = true;
        continue;
      }
      str += "(";
      fBegun = true;
    }
    if ( nChild < pcp->m_rgChildren.size() )
    {
      if ( nChild )
        str += ( exdpChoice == pcp->m_eParticle ) ? "|" : ",";
      const xml_dtd_content_particle< t_TyChar > * pcpChild = &pcp->m_rgChildren[ nChild++ ];
      rgStack.push_back( { pcpChild, 0 } );
      fBegun = false;
      continue;
    }
    str += std::string( ")" ) + s_rgpszOccurrence[ pcp->m_eOccurrence ];
    rgStack.pop_back();
  }
  return str;
}

// What is nested deeply: nothing is done with it by calling a function for each level - the stack of a thread is
//  much smaller than a document may be.
TYPED_TEST( DtdTest, DeepNesting )
{
  typedef TypeParam TyChar;
  // Read all of a document: the count of its tags.
  auto NWalk = []( auto & _rcursor ) -> size_t
  {
    size_t nTags = 0;
    for ( size_t nDepth = 0; ; )
    {
      if ( _rcursor.FMoveDown() )
      {
        ++nDepth;
        ++nTags;
        continue;
      }
      if ( !nDepth )
        break;
      if ( _rcursor.FNextTag() )
        ++nTags;
      else
        --nDepth;
    }
    return nTags;
  };
  // A content model of groups within groups: "(((...(b)...)))" - read, validated against and freed; copied.
  {
    const size_t knDepth = NDeepNestingDepth();
    const std::string strDocument = "<!DOCTYPE doc [<!ELEMENT doc " + std::string( knDepth, '(' ) + "b" + std::string( knDepth, ')' ) + "><!ELEMENT b EMPTY>]>";
    for ( const bool kfValidating : { false, true } )
    {
      for ( const char * pszContent : { "<doc><b/></doc>", "<doc></doc>" } )
      {
        const std::string strText = strDocument + pszContent;
        size_t nTags = 0;
        bool fValid = false;
        std::string strError;
        try
        {
          if ( kfValidating )
          {
            CursorDocument< TyChar, true, true > doc( strText.c_str() );
            ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
            nTags = NWalk( doc.m_cursor );
            fValid = doc.m_cursor.FIsValid();
          }
          else
          {
            CursorDocument< TyChar > doc( strText.c_str() );
            ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
            nTags = NWalk( doc.m_cursor );
          }
        }
        catch ( std::exception const & rexc )
        {
          strError = rexc.what();
        }
        EXPECT_EQ( strError, "" );
        const bool kfHasB = ( std::string::npos != std::string( pszContent ).find( "<b/>" ) );
        EXPECT_EQ( nTags, kfHasB ? 2u : 1u );
        if ( kfValidating )
          EXPECT_EQ( fValid, kfHasB ) << pszContent;
      }
    }
    // A content particle made so - one name within knDepth groups: knDepth + 1 particles. Copied, assigned and freed.
    xml_dtd_content_particle< TyChar > cp;
    cp.m_strName = StrWiden< TyChar >( "b" );
    for ( size_t n = 0; n < knDepth; ++n )
    {
      xml_dtd_content_particle< TyChar > cpGroup;
      cpGroup.m_eParticle = ( n % 2 ) ? exdpChoice : exdpSeq;
      cpGroup.m_rgChildren.push_back( std::move( cp ) );
      cp = std::move( cpGroup );
    }
    xml_dtd_content_particle< TyChar > cpCopy( cp );
    EXPECT_EQ( StrParticleTree( cpCopy ), StrParticleTree( cp ) );
    cpCopy = xml_dtd_content_particle< TyChar >();
    cpCopy = cp;
    EXPECT_EQ( StrParticleTree( cpCopy ), StrParticleTree( cp ) );
    size_t nDepth = 0;
    for ( const xml_dtd_content_particle< TyChar > * pcp = &cpCopy; exdpName != pcp->m_eParticle; ++nDepth )
    {
      ASSERT_EQ( pcp->m_rgChildren.size(), 1u );
      pcp = &pcp->m_rgChildren.front();
    }
    EXPECT_EQ( nDepth, knDepth );
    cp = xml_dtd_content_particle< TyChar >();
    EXPECT_TRUE( cp.m_rgChildren.empty() );
  }
  // A copy is the same tree when groups have more than one child: "(a,(b|c)*,(d,(e|f))?)+" - 11 particles.
  {
    auto Name = []( const char * _psz )
    {
      xml_dtd_content_particle< TyChar > cpName;
      cpName.m_strName = StrWiden< TyChar >( _psz );
      return cpName;
    };
    auto Group = []( EXmlDtdParticle _eParticle, EXmlDtdOccurrence _eOccurrence, std::vector< xml_dtd_content_particle< TyChar > > _rgChildren )
    {
      xml_dtd_content_particle< TyChar > cpGroup;
      cpGroup.m_eParticle = _eParticle;
      cpGroup.m_eOccurrence = _eOccurrence;
      cpGroup.m_rgChildren = std::move( _rgChildren );
      return cpGroup;
    };
    const xml_dtd_content_particle< TyChar > cp = Group( exdpSeq, exdoOneOrMore, { Name( "a" ), Group( exdpChoice, exdoZeroOrMore, { Name( "b" ), Name( "c" ) } ),
      Group( exdpSeq, exdoOptional, { Name( "d" ), Group( exdpChoice, exdoOnce, { Name( "e" ), Name( "f" ) } ) } ) } );
    EXPECT_EQ( StrParticleTree( cp ), "(a,(b|c)*,(d,(e|f))?)+" );
    xml_dtd_content_particle< TyChar > cpCopy( cp );
    EXPECT_EQ( StrParticleTree( cpCopy ), "(a,(b|c)*,(d,(e|f))?)+" );
    xml_dtd_content_particle< TyChar > cpAssigned = Name( "z" );
    cpAssigned = cpCopy;
    EXPECT_EQ( StrParticleTree( cpAssigned ), "(a,(b|c)*,(d,(e|f))?)+" );
  }
  // Elements within elements: read and compared with themselves - one document after another. Each element which is
  //  open is a context of the cursor: so not as deep as the content model.
  {
    const size_t knDepth = (std::min)( NDeepNestingDepth(), size_t( 5000 ) );
    std::string strText = "<!DOCTYPE a [<!ELEMENT a (a?)>]>";
    for ( size_t n = 0; n < knDepth; ++n )
      strText += "<a>";
    for ( size_t n = 0; n < knDepth; ++n )
      strText += "</a>";
    {
      CursorDocument< TyChar > docLeft( strText.c_str() ), docRight( strText.c_str() );
      ASSERT_TRUE( docLeft.FOpened() && docRight.FOpened() );
      std::string strDifference;
      EXPECT_TRUE( FCompareXmlDocuments( docLeft.m_cursor, docRight.m_cursor, xml_compare_options(), &strDifference ) ) << strDifference;
    }
    CursorDocument< TyChar, true, true > docValidating( strText.c_str() );
    ASSERT_TRUE( docValidating.FOpened() ) << docValidating.m_strError;
    EXPECT_EQ( NWalk( docValidating.m_cursor ), knDepth );
    EXPECT_TRUE( docValidating.m_cursor.FIsValid() );
  }
}

// The files which a document may have us read - xml_file_entity_options: by default only those which are where the
//  document is.
TEST( DtdExternalFiles, FilesWhichADocumentMayName )
{
  namespace fs = std::filesystem;
  const fs::path pathTop = fs::temp_directory_path() / "xmlpgtest_dtd_confined";
  std::error_code ec;
  fs::remove_all( pathTop, ec );
  ASSERT_TRUE( fs::create_directories( pathTop / "docs" / "sub", ec ) ) << ec.message();
  auto WriteFile = []( fs::path const & _rpath, std::string const & _rstrBytes )
  {
    std::ofstream ofs( _rpath, std::ios::binary );
    ofs.write( _rstrBytes.data(), std::streamsize( _rstrBytes.size() ) );
  };
  WriteFile( pathTop / "secret.txt", "<s>secret</s>" ); // Above the directory of the document.
  WriteFile( pathTop / "docs" / "beside.xml", "<b>beside</b>" );
  WriteFile( pathTop / "docs" / "sub" / "below.xml", "<b>below</b>" );
  WriteFile( pathTop / "docs" / "large.xml", "<b>" + std::string( 4000, 'x' ) + "</b>" );
  const std::string strDocumentPath = ( pathTop / "docs" / "doc.xml" ).generic_string();
  // What the content of <doc> is when the entity is the file named - or what is thrown when it is had.
  auto Read = [&strDocumentPath]( std::string const & _rstrSystem, const xml_file_entity_options * _poptions = nullptr ) -> std::string
  {
    typedef xml_traits< _l_transport_fixedmem< char8_t >, true, false > TyXmlTraits;
    const std::string strDocument = "<!DOCTYPE doc [<!ENTITY e SYSTEM '" + _rstrSystem + "'>]><doc>&e;</doc>";
    try
    {
      xml_parser< TyXmlTraits > xmlParser;
      if ( !!_poptions )
        xmlParser.SetReadExternalEntityFiles( *_poptions );
      else
        xmlParser.SetReadExternalEntityFiles();
      xmlParser.SetBaseLocation( std::u8string( strDocumentPath.begin(), strDocumentPath.end() ) );
      xml_read_cursor< TyXmlTraits > cursor = xmlParser.OpenMemory( strDocument.data(), strDocument.size() );
      if ( !cursor.FMoveDown() || !cursor.FMoveDown() )
        return "not read"; // The reference stands for nothing which we have: there is no element within <doc>.
      std::u8string strContent;
      cursor.GetContextCur().GetContentString( strContent );
      return std::string( strContent.begin(), strContent.end() );
    }
    catch ( std::exception const & rexc )
    {
      return std::string( "exception: " ) + rexc.what();
    }
  };
  // Where the document is, and below.
  EXPECT_EQ( Read( "beside.xml" ), "beside" );
  EXPECT_EQ( Read( "sub/below.xml" ), "below" );
  EXPECT_EQ( Read( "sub/../beside.xml" ), "beside" );
  EXPECT_EQ( Read( ( pathTop / "docs" / "beside.xml" ).generic_string() ), "beside" ); // Named in full - and it is there.
  // Not above it - however the path is written.
  EXPECT_EQ( Read( "../secret.txt" ), "not read" );
  EXPECT_EQ( Read( "sub/../../secret.txt" ), "not read" );
  EXPECT_EQ( Read( ( pathTop / "secret.txt" ).generic_string() ), "not read" );
  EXPECT_EQ( Read( "file:///" + ( pathTop / "secret.txt" ).generic_string() ), "not read" );
  // Not on another machine - and nothing is looked at there to say so.
  EXPECT_EQ( Read( "//nosuchhost.invalid/share/x.xml" ), "not read" );
  EXPECT_EQ( Read( "\\\\nosuchhost.invalid\\share\\x.xml" ), "not read" );
  EXPECT_EQ( Read( "http://nosuchhost.invalid/x.xml" ), "not read" );
  // Unless whoever opens the document says where else - or that anywhere will do.
  {
    xml_file_entity_options options;
    options.m_pathRoot = pathTop;
    EXPECT_EQ( Read( "../secret.txt", &options ), "secret" );
    EXPECT_EQ( Read( "beside.xml", &options ), "beside" );
    options.m_pathRoot = pathTop / "docs" / "sub";
    EXPECT_EQ( Read( "beside.xml", &options ), "not read" );
    EXPECT_EQ( Read( "sub/below.xml", &options ), "below" );
    options = xml_file_entity_options();
    options.m_fConfineToDocument = false;
    EXPECT_EQ( Read( "../secret.txt", &options ), "secret" );
  }
  // A file which is larger than may be is not read.
  {
    EXPECT_EQ( Read( "large.xml" ), std::string( 4000, 'x' ) );
    xml_file_entity_options options;
    options.m_nbyMaxFile = 1000;
    EXPECT_EQ( Read( "large.xml", &options ), "not read" );
    EXPECT_EQ( Read( "beside.xml", &options ), "beside" );
  }
  fs::remove_all( pathTop, ec );
}

// Documents which are cut short and documents which are changed a character at a time: whatever the text, reading it
//  either gives a document or throws - it doesn't crash and it doesn't go on forever. And a document which is read is
//  one which the writer can write: what it writes is that document when it is read again.
// The documents which this starts from - each with much of what a document may have.
static const char * const s_rgpszRobustnessDocuments[] = {
  // Tags, attributes, references, comments, processing instructions, CDATA - and namespaces.
  "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone='yes'?>\n"
  "<!-- before --><?pi before?>\n"
  "<doc xmlns='urn:d' xmlns:p=\"urn:p\" a='1' p:b=\"two &amp; &#51; &#x34;\">text &lt;here&gt;<p:e/>\n"
  " <sub>more<![CDATA[ <raw> & ]]> text</sub><!-- c --><?t d?><e2 x='y'></e2>\n"
  "</doc>\n<!-- after -->\n",
  // A DTD: elements, attributes with defaults and types, entities - text, markup, within each other - and notations.
  "<!DOCTYPE doc [\n"
  "<!ELEMENT doc (item+, note?)>\n"
  "<!ELEMENT item (#PCDATA|b)*>\n"
  "<!ELEMENT b (#PCDATA)>\n"
  "<!ELEMENT note EMPTY>\n"
  "<!ATTLIST item kind (a|b) 'a' id ID #IMPLIED ref IDREF #IMPLIED n NMTOKENS #IMPLIED>\n"
  "<!ATTLIST note xml:space (default|preserve) #FIXED 'default'>\n"
  "<!NOTATION gif SYSTEM 'viewer'>\n"
  "<!ENTITY pic SYSTEM 'pic.gif' NDATA gif>\n"
  "<!ENTITY t 'text'>\n"
  "<!ENTITY ws 'a\tb'>\n"
  "<!ENTITY m 'with <b>bold &t;</b>'>\n"
  "<!ENTITY mm '&m; and &m;'>\n"
  "<?dtd-pi x?><!-- in the DTD -->\n"
  "]>\n"
  "<doc><item id='i1'>one &t;</item><item kind='b' id=' i2 ' ref='i1' n=' x  y '>two &m; &mm; end &ws;</item><note/></doc>\n",
  // Parameter entities - between declarations and in the literal of an entity - and what declares by them.
  "<!DOCTYPE doc [\n"
  "<!ENTITY % decls \"<!ELEMENT doc (#PCDATA|e)*><!ATTLIST doc a CDATA 'dflt'>\">\n"
  "%decls;\n"
  "<!ENTITY % inner \"<!ENTITY g 'general &#38;#60;e/&#38;#62;'>\">\n"
  "%inner;\n"
  "<!ELEMENT e EMPTY>\n"
  "]>\n"
  "<doc b='v'>x &g; y<e/></doc>",
};

// Read all of the document - every string of it is had - by a validating cursor. Returns whether it was read: false
//  if reading it threw. _rstrWritten: what the writer writes from it.
template < class t_TyChar >
bool FReadForRobustness( std::basic_string< t_TyChar > const & _rstrText, std::basic_string< t_TyChar > * _pstrWritten = nullptr )
{
  typedef xml_traits< _l_transport_fixedmem< t_TyChar >, true, true > TyXmlTraits;
  typedef xml_parser< TyXmlTraits > TyXmlParser;
  try
  {
    {
      TyXmlParser xmlParser;
      typename TyXmlParser::_TyReadCursor cursor = xmlParser.OpenMemory( _rstrText.data(), _rstrText.length() * sizeof( t_TyChar ) );
      xml_compare_options options;
      options.m_fCompareDoctypeDecl = true;
      _xml_compare_reader< typename TyXmlParser::_TyReadCursor > reader( cursor, options );
      _xml_compare_item item;
      do
      {
        reader.GetNext( item );
      }
      while ( excmpEndOfDocument != item.m_excmp );
    }
    if ( !!_pstrWritten )
    {
      TyXmlParser xmlParser;
      typename TyXmlParser::_TyReadCursor cursor = xmlParser.OpenMemory( _rstrText.data(), _rstrText.length() * sizeof( t_TyChar ) );
      *_pstrWritten = StrWriteFromAnyCursor< t_TyChar >( cursor );
    }
  }
  catch ( std::exception const & )
  {
    return false;
  }
  return true;
}
// Whether the two texts are the same document - false as well if reading either throws.
template < class t_TyChar >
bool FSameDocumentForRobustness( std::basic_string< t_TyChar > const & _rstrLeft, std::basic_string< t_TyChar > const & _rstrRight, std::string & _rstrDifference )
{
  typedef xml_traits< _l_transport_fixedmem< t_TyChar >, true, false > TyXmlTraits;
  typedef xml_parser< TyXmlTraits > TyXmlParser;
  try
  {
    TyXmlParser xmlParserLeft, xmlParserRight;
    typename TyXmlParser::_TyReadCursor cursorLeft = xmlParserLeft.OpenMemory( _rstrLeft.data(), _rstrLeft.length() * sizeof( t_TyChar ) );
    typename TyXmlParser::_TyReadCursor cursorRight = xmlParserRight.OpenMemory( _rstrRight.data(), _rstrRight.length() * sizeof( t_TyChar ) );
    xml_compare_options options;
    options.m_fCompareDoctypeDecl = true;
    return FCompareXmlDocuments( cursorLeft, cursorRight, options, &_rstrDifference );
  }
  catch ( std::exception const & rexc )
  {
    _rstrDifference = std::string( "exception: " ) + rexc.what();
    return false;
  }
}

TYPED_TEST( DtdTest, DocumentsWhichAreCutShort )
{
  typedef TypeParam TyChar;
  for ( const char * pszDocument : s_rgpszRobustnessDocuments )
  {
    const std::basic_string< TyChar > strText = StrDocumentText< TyChar >( pszDocument );
    ASSERT_TRUE( FReadForRobustness( strText ) ) << pszDocument;
    // Where the document's element ends: what is cut before that is no document.
    const std::string strDocument( pszDocument );
    const size_t knPosEndOfElement = strDocument.rfind( "</doc>" ) + 6;
    const size_t knBOM = strText.length() - strDocument.length();
    for ( size_t nLength = 0; nLength < strText.length(); ++nLength )
    {
      const bool kfRead = FReadForRobustness( strText.substr( 0, nLength ) );
      if ( nLength < knBOM + knPosEndOfElement )
        EXPECT_FALSE( kfRead ) << "Cut at " << nLength << ":\n" << strDocument.substr( 0, ( nLength > knBOM ) ? ( nLength - knBOM ) : 0 );
    }
  }
}

TYPED_TEST( DtdTest, DocumentsWhichAreChanged )
{
  typedef TypeParam TyChar;
  // The characters which are put into a document: those of markup, white space, one which is no character of XML
  //  and some which are not ASCII.
  static const char32_t s_rgchPut[] = { U'<', U'>', U'&', U';', U'\'', U'"', U'[', U']', U'%', U'!', U'?', U'/', U'-', U'=', U':', U'#', U' ', U'\n', U'\r', U'\t',
                                        U'x', U'0', char32_t( 0x1 ), char32_t( 0xE9 ), char32_t( 0x3A9 ), char32_t( 0xB7 ), char32_t( 0xFFFE ), char32_t( 0x10400 ) };
  uint32_t nRandom = 12345; // The same changes each time the test runs.
  auto NRandom = [&nRandom]( size_t _nBelow ) -> size_t
  {
    nRandom = nRandom * 1664525u + 1013904223u;
    return size_t( nRandom >> 8 ) % _nBelow;
  };
  size_t nRead = 0, nNotRead = 0;
  for ( const char * pszDocument : s_rgpszRobustnessDocuments )
  {
    const std::basic_string< TyChar > strText = StrDocumentText< TyChar >( pszDocument );
    const size_t knBOM = strText.length() - strlen( pszDocument );
    for ( size_t nChange = 0; nChange < 3000; ++nChange )
    {
      std::basic_string< TyChar > strChanged = strText;
      // One change, or a few of them.
      for ( size_t nOf = 1 + NRandom( 3 ); nOf--; )
      {
        if ( strChanged.length() <= knBOM + 1 )
          break;
        const size_t knPos = knBOM + NRandom( strChanged.length() - knBOM );
        switch( NRandom( 5 ) )
        {
          case 0: // A character is taken away.
            strChanged.erase( knPos, 1 );
          break;
          case 1: // One is there twice.
            strChanged.insert( knPos, 1, strChanged[knPos] );
          break;
          case 2: // Two are the other way around.
            if ( knPos + 1 < strChanged.length() )
              std::swap( strChanged[knPos], strChanged[knPos + 1] );
          break;
          default: // One is another - or another is put before it.
          {
            const char32_t chPut = s_rgchPut[ NRandom( DimensionOf( s_rgchPut ) ) ];
            std::basic_string< TyChar > strPut;
            ConvertString( strPut, &chPut, 1 );
            if ( !!NRandom( 2 ) )
              strChanged.replace( knPos, 1, strPut );
            else
              strChanged.insert( knPos, strPut );
          }
          break;
        }
      }
      std::basic_string< TyChar > strWritten;
      if ( !FReadForRobustness( strChanged, &strWritten ) )
      {
        ++nNotRead;
        continue;
      }
      ++nRead;
      // It is a document: then what the writer wrote of it is that document.
      std::string strDifference;
      std::basic_string< TyChar > strWrittenText;
      if ( sizeof( TyChar ) > 1 )
        strWrittenText += TyChar( 0xFEFF );
      strWrittenText += strWritten;
      EXPECT_TRUE( FSameDocumentForRobustness( strChanged, strWrittenText, strDifference ) )
        << "change " << nChange << ": " << strDifference << "\nthe document:\n" << StrNarrowExact( strChanged.substr( knBOM ) ) << "\nwritten:\n" << StrNarrowExact( strWritten );
      if ( ::testing::Test::HasFailure() )
        return; // One is enough to look at.
    }
  }
  // Most changes make a document which is none - and some don't: both are read here.
  EXPECT_GT( nRead, 100u );
  EXPECT_GT( nNotRead, 1000u );
}

// The W3C XML Conformance Test Suite - unittests/xmlconf: XMLP_XMLCONF_DIR is defined by the build.
// The suite tests whole documents and we only have the DTD as yet, so what is tested is what can be said of the DTD alone:
//  - WellFormedDocuments: the document type declaration of every document which the suite says is well formed - valid or
//    not - is accepted.
//  - NotWellFormedDeclarations: the document type declaration of every document which IBM's tests of the productions of
//    the declarations say is not well formed is rejected. ( For the other productions what is wrong is elsewhere - in the
//    external subset, in the document itself or in a constraint which is not grammar. )
//  - ReadWellFormedDocuments: the same documents are read into an xml_dtd by xml_dtd_parser without an error - and those
//    of IBM's tests which are not well formed for a reason which the reading finds are not.
//  - ExternalSubsets: the external subsets of well formed documents are accepted - but for those which have references
//    to parameter entities within declarations, which aren't substituted yet: those are listed and must be rejected at
//    the reference, so that the list is found to be out of date when they are. And those which the tests of the
//    productions of the conditional sections say are not well formed are rejected.
// A document is parsed from its "<!DOCTYPE" - found by skipping the XML declaration, comments, processing instructions
//  and white space before it - in UTF-8 or UTF-16 as the document is. Documents in other encodings are skipped.
#ifdef XMLP_XMLCONF_DIR
namespace
{
namespace fs = std::filesystem;
// The external entities of the documents of the suite are files of the suite - some of them in a directory beside
//  that of the document: what may be read is what is within the suite.
inline xml_file_entity_options SuiteFileEntityOptions()
{
  xml_file_entity_options options;
  options.m_pathRoot = fs::path( XMLP_XMLCONF_DIR );
  return options;
}

template < class t_TyChar >
bool FStartsWith( std::basic_string_view< t_TyChar > _sv, size_t _nPos, const char * _psz )
{
  for ( ; !!*_psz; ++_psz, ++_nPos )
    if ( ( _nPos >= _sv.size() ) || ( _sv[ _nPos ] != t_TyChar( *_psz ) ) )
      return false;
  return true;
}
template < class t_TyChar >
size_t NFind( std::basic_string_view< t_TyChar > _sv, size_t _nPos, const char * _psz )
{
  for ( ; _nPos < _sv.size(); ++_nPos )
    if ( FStartsWith( _sv, _nPos, _psz ) )
      return _nPos;
  return std::string::npos;
}
// The position of the "<!DOCTYPE" of the prolog of the document - npos if it has none.
template < class t_TyChar >
size_t NFindDoctype( std::basic_string_view< t_TyChar > _sv )
{
  size_t nPos = 0;
  for (;;)
  {
    while ( ( nPos < _sv.size() ) && ( ( _sv[ nPos ] == 0x20 ) || ( _sv[ nPos ] == 0x09 ) || ( _sv[ nPos ] == 0x0d ) || ( _sv[ nPos ] == 0x0a ) ) )
      ++nPos;
    const char * pszEnd;
    if ( FStartsWith( _sv, nPos, "<?" ) )
      pszEnd = "?>";
    else
    if ( FStartsWith( _sv, nPos, "<!--" ) )
      pszEnd = "-->";
    else
      return FStartsWith( _sv, nPos, "<!DOCTYPE" ) ? nPos : std::string::npos;
    nPos = NFind( _sv, nPos + 2, pszEnd );
    if ( std::string::npos == nPos )
      return std::string::npos;
    nPos += strlen( pszEnd );
  }
}
// Parse the document type declaration of the document in the file - or the file as an external subset.
// No value if the file isn't parsed: it is in an encoding we don't do here or the document has no document type declaration.
// _fRead: read it into an xml_dtd with xml_dtd_parser rather than parsing it to the parse tree.
std::optional< DtdParseResult > ConformanceParse( fs::path const & _rpath, bool _fExternalSubset, bool _fRead = false )
{
  std::ifstream ifs( _rpath, std::ios::binary );
  std::stringstream ss;
  ss << ifs.rdbuf();
  const std::string strBytes = ss.str();
  auto lambdaParse = [&]< class t_TyChar >( std::basic_string_view< t_TyChar > _sv ) -> std::optional< DtdParseResult >
  {
    size_t nPosDoctype = 0;
    if ( !_fExternalSubset )
    {
      nPosDoctype = NFindDoctype( _sv );
      if ( std::string::npos == nPosDoctype )
        return std::nullopt;
    }
    if ( !_fRead )
      return DtdTester< t_TyChar >::ParseText( _sv.substr( nPosDoctype ), _fExternalSubset ? s_knEntry_extSubset : s_knEntry_doctypedecl );
    DtdParseResult result;
    xml_dtd< t_TyChar > dtd;
    // Whether the document is declared to be standalone matters: it is in the XML declaration, before the "<!DOCTYPE".
    const std::basic_string_view< t_TyChar > svProlog = _sv.substr( 0, nPosDoctype );
    const bool fStandalone = ( std::string::npos != NFind( svProlog, 0, "standalone=\"yes\"" ) ) || ( std::string::npos != NFind( svProlog, 0, "standalone='yes'" ) );
    result.m_strError = StrReadDtdText< t_TyChar >( _sv.substr( nPosDoctype ), _fExternalSubset, dtd, fStandalone );
    result.m_fAccepted = result.m_strError.empty();
    return result;
  };
  auto lambdaByte = [&]( size_t _n ) -> unsigned char { return ( _n < strBytes.size() ) ? (unsigned char)strBytes[ _n ] : 0; };
  const bool fLittleEndian16 = ( lambdaByte( 0 ) == 0xff ) && ( lambdaByte( 1 ) == 0xfe ) && !( !lambdaByte( 2 ) && !lambdaByte( 3 ) );
  const bool fBigEndian16 = ( lambdaByte( 0 ) == 0xfe ) && ( lambdaByte( 1 ) == 0xff );
  if ( fLittleEndian16 || fBigEndian16 )
  {
    std::u16string str16;
    for ( size_t n = 2; n + 1 < strBytes.size(); n += 2 )
      str16 += char16_t( fLittleEndian16 ? ( lambdaByte( n ) | ( lambdaByte( n + 1 ) << 8 ) ) : ( lambdaByte( n + 1 ) | ( lambdaByte( n ) << 8 ) ) );
    return lambdaParse( std::u16string_view( str16 ) );
  }
  if ( std::string::npos != strBytes.find( '\0' ) )
    return std::nullopt; // UTF-16 without a byte order mark or UTF-32.
  // Bytes: UTF-8 unless the XML declaration or the text declaration says otherwise.
  size_t nBegin = ( ( lambdaByte( 0 ) == 0xef ) && ( lambdaByte( 1 ) == 0xbb ) && ( lambdaByte( 2 ) == 0xbf ) ) ? 3 : 0;
  if ( !strBytes.compare( nBegin, 5, "<?xml" ) )
  {
    const size_t nEnd = strBytes.find( "?>", nBegin );
    const size_t nEncoding = strBytes.find( "encoding", nBegin );
    if ( ( std::string::npos != nEncoding ) && ( nEncoding < nEnd ) )
    {
      std::string strDecl = strBytes.substr( nEncoding, nEnd - nEncoding );
      std::transform( strDecl.begin(), strDecl.end(), strDecl.begin(), []( unsigned char _c ) { return char( tolower( _c ) ); } );
      if ( ( std::string::npos == strDecl.find( "utf-8" ) ) && ( std::string::npos == strDecl.find( "ascii" ) ) && ( std::string::npos == strDecl.find( "iso-8859-1" ) ) )
        return std::nullopt;
    }
  }
  return lambdaParse( std::basic_string_view< char8_t >( (const char8_t *)strBytes.data() + nBegin, strBytes.size() - nBegin ) );
}
// The files with the extension below the directory of the suite - sorted, as paths relative to the suite with '/'.
std::vector< std::string > RgConformanceFiles( const char * _pszDirectory, const char * _pszExtension )
{
  std::vector< std::string > rgstr;
  const fs::path pathSuite( XMLP_XMLCONF_DIR );
  const fs::path pathDir = pathSuite / _pszDirectory;
  if ( !fs::is_directory( pathDir ) )
    return rgstr;
  for ( fs::directory_entry const & rde : fs::recursive_directory_iterator( pathDir ) )
    if ( rde.is_regular_file() && ( rde.path().extension() == _pszExtension ) )
      rgstr.push_back( fs::relative( rde.path(), pathSuite ).generic_string() );
  std::sort( rgstr.begin(), rgstr.end() );
  return rgstr;
}
fs::path PathConformanceFile( std::string const & _rstr )
{
  return fs::path( XMLP_XMLCONF_DIR ) / _rstr;
}
bool FHaveConformanceSuite()
{
  return fs::is_directory( fs::path( XMLP_XMLCONF_DIR ) / "ibm" );
}
} // namespace

TEST( DtdConformance, WellFormedDocuments )
{
  if ( !FHaveConformanceSuite() )
    GTEST_SKIP() << "No conformance suite at " << XMLP_XMLCONF_DIR;
  std::vector< std::string > rgstrFiles;
  for ( const char * pszDir : { "ibm/valid", "ibm/invalid", "sun/valid", "sun/invalid", "xmltest/valid", "xmltest/invalid" } )
    for ( std::string const & rstr : RgConformanceFiles( pszDir, ".xml" ) )
      rgstrFiles.push_back( rstr );
  // OASIS: the documents which pass are named so.
  for ( std::string const & rstr : RgConformanceFiles( "oasis", ".xml" ) )
    if ( std::string::npos != rstr.find( "pass" ) )
      rgstrFiles.push_back( rstr );
  size_t nParsed = 0;
  for ( std::string const & rstrFile : rgstrFiles )
  {
    const std::optional< DtdParseResult > optResult = ConformanceParse( PathConformanceFile( rstrFile ), false );
    if ( !optResult )
      continue;
    ++nParsed;
    EXPECT_TRUE( optResult->m_fAccepted ) << rstrFile << ": " << optResult->m_strError;
  }
  EXPECT_GE( nParsed, 500u ) << "of " << rgstrFiles.size();
}

TEST( DtdConformance, ReadWellFormedDocuments )
{
  if ( !FHaveConformanceSuite() )
    GTEST_SKIP() << "No conformance suite at " << XMLP_XMLCONF_DIR;
  size_t nRead = 0;
  for ( const char * pszDir : { "ibm/valid", "ibm/invalid", "sun/valid", "sun/invalid", "xmltest/valid", "xmltest/invalid" } )
  {
    for ( std::string const & rstrFile : RgConformanceFiles( pszDir, ".xml" ) )
    {
      const std::optional< DtdParseResult > optResult = ConformanceParse( PathConformanceFile( rstrFile ), false, true );
      if ( !optResult )
        continue;
      ++nRead;
      EXPECT_TRUE( optResult->m_fAccepted ) << rstrFile << ": " << optResult->m_strError;
    }
  }
  EXPECT_GE( nRead, 450u );
  // Not well formed for what the grammar doesn't say: a parameter entity reference within a declaration in the internal
  //  subset, a parameter entity referenced before it is declared in a standalone document.
  for ( const char * pszFile : { "ibm/not-wf/P29/ibm29n04.xml", "ibm/not-wf/P69/ibm69n05.xml", "ibm/not-wf/P69/ibm69n06.xml" } )
  {
    const std::optional< DtdParseResult > optResult = ConformanceParse( PathConformanceFile( pszFile ), false, true );
    ASSERT_TRUE( optResult.has_value() ) << pszFile;
    EXPECT_FALSE( optResult->m_fAccepted ) << pszFile;
    // The grammar alone accepts them.
    const std::optional< DtdParseResult > optResultGrammar = ConformanceParse( PathConformanceFile( pszFile ), false, false );
    EXPECT_TRUE( optResultGrammar.has_value() && optResultGrammar->m_fAccepted ) << pszFile;
  }
}

TEST( DtdConformance, NotWellFormedDeclarations )
{
  if ( !FHaveConformanceSuite() )
    GTEST_SKIP() << "No conformance suite at " << XMLP_XMLCONF_DIR;
  // IBM's tests are in a directory for each production: these are the productions of the document type declaration
  //  and of the element, attribute-list, entity and notation declarations.
  size_t nParsed = 0;
  for ( int nProduction : { 28, 45, 46, 47, 48, 49, 50, 51, 52, 53, 54, 55, 56, 57, 58, 59, 71, 72, 73, 74, 75, 76, 82, 83 } )
  {
    for ( std::string const & rstrFile : RgConformanceFiles( ( "ibm/not-wf/P" + std::to_string( nProduction ) ).c_str(), ".xml" ) )
    {
      const std::optional< DtdParseResult > optResult = ConformanceParse( PathConformanceFile( rstrFile ), false );
      if ( !optResult )
        continue;
      ++nParsed;
      EXPECT_FALSE( optResult->m_fAccepted ) << rstrFile;
    }
  }
  EXPECT_GE( nParsed, 140u ); // 148 when this was written.
}

TEST( DtdConformance, ExternalSubsets )
{
  if ( !FHaveConformanceSuite() )
    GTEST_SKIP() << "No conformance suite at " << XMLP_XMLCONF_DIR;
  // This is of the grammar alone - the tokens as they are in the file. These have references to parameter entities
  //  within declarations, which the grammar doesn't have: xml_dtd_parser puts the text of the entity in place of such a
  //  reference - see ExternalSubsetsAreRead below, where these are read.
  const std::set< std::string > setParameterEntitiesWithinDeclarations{
    "ibm/invalid/P49/ibm49i01.dtd", "ibm/invalid/P50/ibm50i01.dtd", "ibm/invalid/P51/ibm51i01.dtd",
    "ibm/valid/P49/ibm49v01.dtd", "ibm/valid/P50/ibm50v01.dtd", "ibm/valid/P51/ibm51v02.dtd", "sun/valid/dtdtest.dtd" };
  // This one has a text declaration without an encoding declaration, which [77] requires: it is rejected. ( No document of
  //  the suite uses it. )
  const std::set< std::string > setNotExternalSubsets{ "ibm/invalid/P51/ibm51i03.dtd" };
  size_t nAccepted = 0, nParameterEntities = 0;
  for ( const char * pszDir : { "ibm/valid", "ibm/invalid", "sun/valid", "sun/invalid", "xmltest/valid", "xmltest/invalid" } )
  {
    for ( std::string const & rstrFile : RgConformanceFiles( pszDir, ".dtd" ) )
    {
      const std::optional< DtdParseResult > optResult = ConformanceParse( PathConformanceFile( rstrFile ), true );
      if ( !optResult )
        continue;
      if ( setParameterEntitiesWithinDeclarations.contains( rstrFile ) )
      {
        ++nParameterEntities;
        EXPECT_FALSE( optResult->m_fAccepted ) << rstrFile << ": accepted - are parameter entities substituted now? Then it shouldn't be in the list.";
        EXPECT_NE( optResult->m_strError.find( "unexpected [PEReference]" ), std::string::npos ) << rstrFile << ": " << optResult->m_strError;
      }
      else
      if ( setNotExternalSubsets.contains( rstrFile ) )
        EXPECT_FALSE( optResult->m_fAccepted ) << rstrFile;
      else
      {
        EXPECT_TRUE( optResult->m_fAccepted ) << rstrFile << ": " << optResult->m_strError;
        nAccepted += optResult->m_fAccepted ? 1 : 0;
      }
    }
  }
  EXPECT_EQ( nParameterEntities, setParameterEntitiesWithinDeclarations.size() );
  EXPECT_GE( nAccepted, 40u ); // 43 when this was written.
  // The conditional sections [61] through [65]: external subsets which are not well formed.
  size_t nRejected = 0;
  std::vector< std::string > rgstrNotWellFormed;
  for ( int nProduction : { 61, 62, 63, 64, 65 } )
    for ( std::string const & rstr : RgConformanceFiles( ( "ibm/not-wf/P" + std::to_string( nProduction ) ).c_str(), ".dtd" ) )
      rgstrNotWellFormed.push_back( rstr );
  for ( std::string const & rstr : RgConformanceFiles( "oasis", ".dtd" ) )
    for ( const char * pszProduction : { "p61fail", "p62fail", "p63fail", "p64fail" } )
      if ( std::string::npos != rstr.find( pszProduction ) )
        rgstrNotWellFormed.push_back( rstr );
  for ( std::string const & rstrFile : rgstrNotWellFormed )
  {
    const std::optional< DtdParseResult > optResult = ConformanceParse( PathConformanceFile( rstrFile ), true );
    if ( !optResult )
      continue;
    EXPECT_FALSE( optResult->m_fAccepted ) << rstrFile;
    nRejected += optResult->m_fAccepted ? 0 : 1;
  }
  EXPECT_GE( nRejected, 25u );
}

// The verdict of a validating cursor on a document of the suite.
enum EConformanceValidity
{
  ecvNotJudged, // The document can't be judged: see ConformanceValidate().
  ecvValid,
  ecvNotValid
};
// The verdict of a validating cursor - which is at the beginning of its document - on the document.
// A document is not judged if nothing is found wrong with it and what it is depends on what we don't read: declarations
//  of it were not read, or it refers to an entity whose text is not read. Nor if it has no document type declaration -
//  or it can't be read at all, which is for what isn't supported since all of these documents are well formed.
// _rstrWhy: the first violation, or why it wasn't judged.
template < class t_TyCursor >
EConformanceValidity ConformanceVerdict( t_TyCursor & _rcursor, std::string & _rstrWhy )
{
  try
  {
    if ( _rcursor.FMoveDown() )
      (void)_rcursor.FNextTag();
  }
  catch ( std::exception const & rexc )
  {
    _rstrWhy = std::string( "not read: " ) + rexc.what();
    return ecvNotJudged;
  }
  if ( !_rcursor.FHasDtd() )
  {
    // There is nothing which it is valid against: the validator says so once it has seen the document's element.
    if ( _rcursor.RgValidityErrors().empty() )
    {
      _rstrWhy = "no document type declaration";
      return ecvNotJudged;
    }
    _rstrWhy = _rcursor.RgValidityErrors().front();
    return ecvNotValid;
  }
  auto const & rdtd = _rcursor.GetDtd();
  if ( !rdtd.m_rgstrValidityErrors.empty() || !_rcursor.RgValidityErrors().empty() )
  {
    // Not valid whatever else there is to know of it.
    _rstrWhy = !rdtd.m_rgstrValidityErrors.empty() ? rdtd.m_rgstrValidityErrors.front() : _rcursor.RgValidityErrors().front();
    return ecvNotValid;
  }
  if ( !rdtd.FAllDeclarationsRead() )
  {
    _rstrWhy = "declarations which are not read";
    return ecvNotJudged;
  }
  if ( _rcursor.FValidationIncomplete() )
  {
    _rstrWhy = "a reference to an entity whose text is not read";
    return ecvNotJudged;
  }
  if ( _rcursor.FIsValid() )
    return ecvValid;
  _rstrWhy = !rdtd.m_rgstrValidityErrors.empty() ? rdtd.m_rgstrValidityErrors.front() : _rcursor.RgValidityErrors().front();
  return ecvNotValid;
}
// Read a document of the suite with a validating cursor and say whether it is valid.
// The external subset and the external entities of the document are read: they are files of the suite.
// The document is read by the variant parser - in whichever encoding it is. One which is in UTF-8 is read as well by
//  the parser of UTF-8, as the other tests read: the two must say the same.
EConformanceValidity ConformanceValidate( fs::path const & _rpath, std::string & _rstrWhy, bool _fUseXMLNamespaces = true )
{
  _rstrWhy.clear();
  std::ifstream ifs( _rpath, std::ios::binary );
  std::stringstream ss;
  ss << ifs.rdbuf();
  const std::string strBytes = ss.str();
  if ( strBytes.empty() )
  {
    _rstrWhy = "empty";
    return ecvNotJudged;
  }
  EConformanceValidity ecvVariant = ecvNotJudged;
  try
  {
    TyXmlParserVarDtd xmlParser;
    xmlParser.SetReadExternalEntityFiles( SuiteFileEntityOptions() );
    xmlParser.SetBaseLocation( _rpath.generic_string() );
    xmlParser.SetUseXMLNamespaces( _fUseXMLNamespaces );
    TyXmlParserVarDtd::_TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strBytes.data(), strBytes.size() );
    ecvVariant = std::visit(
      [&_rstrWhy]( auto & _tCursor ) -> EConformanceValidity
      {
        return ConformanceVerdict( _tCursor, _rstrWhy );
      }, cursor.GetVariant() );
  }
  catch ( std::exception const & rexc )
  {
    _rstrWhy = std::string( "not opened: " ) + rexc.what();
  }
  if ( std::string::npos != strBytes.find( '\0' ) )
    return ecvVariant; // Not UTF-8.
  std::string strWhyUTF8;
  EConformanceValidity ecvUTF8 = ecvNotJudged;
  {
    CursorDocument< char8_t, true, true > doc( strBytes.c_str(), &FReadExternalEntityFile< char8_t >, _rpath.generic_string().c_str(), _fUseXMLNamespaces );
    if ( !doc.FOpened() )
      strWhyUTF8 = "not opened: " + doc.m_strError;
    else
      ecvUTF8 = ConformanceVerdict( doc.m_cursor, strWhyUTF8 );
  }
  EXPECT_EQ( ecvVariant, ecvUTF8 ) << _rpath.generic_string() << ": the variant parser says [" << _rstrWhy << "] and the parser of UTF-8 says [" << strWhyUTF8 << "]";
  _rstrWhy = strWhyUTF8;
  return ecvUTF8;
}

// The documents which the suite says are valid and those which it says are not valid - all of them are well formed -
//  are read by a validating cursor: it must find the first valid and the second not.
// Not all of them can be judged yet - see ConformanceValidate(): most of those which can't have an external subset.
TEST( DtdConformance, ValidAndInvalidDocuments )
{
  size_t nValidJudged = 0, nInvalidJudged = 0, nNotJudged = 0;
  for ( const char * pszDirectory : { "xmltest/valid/sa", "ibm/valid", "sun/valid", "xmltest/invalid", "ibm/invalid", "sun/invalid" } )
  {
    const bool kfValid = ( std::string::npos == std::string( pszDirectory ).find( "invalid" ) );
    for ( std::string const & rstrFile : RgConformanceFiles( pszDirectory, ".xml" ) )
    {
      if ( std::string::npos != rstrFile.find( "/out/" ) )
        continue; // The canonical forms of the documents - not documents of the tests.
      // The documents which the catalog of the suite says are not for a processor of namespaces ( NAMESPACE="no" ): they
      //  have names which are not qualified names. They are read without namespaces - and with them they are an error.
      static const std::set< std::string > s_setNotNamespaceWellFormed{ "xmltest/valid/sa/012.xml" };
      const bool kfUseXMLNamespaces = !s_setNotNamespaceWellFormed.contains( rstrFile );
      std::string strWhy;
      if ( !kfUseXMLNamespaces )
      {
        const EConformanceValidity ecvWith = ConformanceValidate( PathConformanceFile( rstrFile ), strWhy, true );
        EXPECT_EQ( ecvWith, ecvNotJudged ) << rstrFile << " is not namespace-well-formed and was read with namespaces.";
        EXPECT_NE( strWhy.find( "not namespace-well-formed" ), std::string::npos ) << rstrFile << ": " << strWhy;
      }
      const EConformanceValidity ecv = ConformanceValidate( PathConformanceFile( rstrFile ), strWhy, kfUseXMLNamespaces );
      if ( ecvNotJudged == ecv )
      {
        ++nNotJudged;
        std::cout << "[          ] not judged: " << rstrFile << ": " << strWhy << std::endl;
        continue;
      }
      ( kfValid ? nValidJudged : nInvalidJudged ) += 1;
      if ( kfValid )
        EXPECT_EQ( ecv, ecvValid ) << rstrFile << " is valid and was found not to be: " << strWhy;
      else
        EXPECT_EQ( ecv, ecvNotValid ) << rstrFile << " is not valid and nothing was found wrong with it.";
    }
  }
  if ( !nValidJudged && !nInvalidJudged && !nNotJudged )
    return; // The suite is not here.
  // Those which are judged - so that it is noticed if they no longer are.
  std::cout << "[          ] judged: " << nValidJudged << " valid, " << nInvalidJudged << " not valid; not judged: " << nNotJudged << "\n";
  EXPECT_GE( nValidJudged, 297u );
  EXPECT_GE( nInvalidJudged, 128u );
  EXPECT_EQ( nNotJudged, 0u ); // 297 and 128 when this was written: all of them.
}

// Richard Tobin's tests of Namespaces in XML 1.0 - eduni/namespaces/1.0 of the suite: its catalog says of each document
//  whether it is valid, well formed and not valid ( "invalid" ), not namespace-well-formed ( "not-wf" ) or has what a
//  processor may or may not report ( "error" ).
// Each document is a test of its own - named by its ID in the catalog - so that what fails says which document it is:
//  - Read with namespaces by a validating cursor it must be found to be what the catalog says it is.
//  - Read without namespaces it is a document of XML 1.0, and all but one of these are well formed ones: it is not an
//    error - and it is valid or isn't as it is with namespaces, but for what is only namespace validity.
//  - Either way, if it is read then what the writer writes from it is the same document when that is read.
struct NamespaceSuiteTest
{
  std::string m_strUri; // The file - beside the catalog.
  std::string m_strId;
  std::string m_strType;
};
void PrintTo( NamespaceSuiteTest const & _rtest, std::ostream * _pos )
{
  *_pos << _rtest.m_strId << " ( " << _rtest.m_strUri << ", " << _rtest.m_strType << " )";
}
fs::path PathNamespaceSuite()
{
  return fs::path( XMLP_XMLCONF_DIR ) / "eduni" / "namespaces" / "1.0";
}
// The tests of the catalog - none if the suite is not here.
std::vector< NamespaceSuiteTest > RgNamespaceSuiteTests()
{
  std::vector< NamespaceSuiteTest > rgTests;
  std::ifstream ifsCatalog( PathNamespaceSuite() / "rmt-ns10.xml", std::ios::binary );
  if ( !ifsCatalog )
    return rgTests;
  std::stringstream ssCatalog;
  ssCatalog << ifsCatalog.rdbuf();
  const std::string strCatalog = ssCatalog.str();
  // The value of an attribute of the <TEST> which begins at _nPos.
  auto StrAttribute = [&strCatalog]( size_t _nPos, const char * _pszName ) -> std::string
  {
    const size_t knPosEnd = strCatalog.find( '>', _nPos );
    const std::string strFind = std::string( " " ) + _pszName + "=\"";
    const size_t knPosAttr = strCatalog.find( strFind, _nPos );
    if ( ( std::string::npos == knPosAttr ) || ( knPosAttr > knPosEnd ) )
      return std::string();
    const size_t knPosValue = knPosAttr + strFind.length();
    return strCatalog.substr( knPosValue, strCatalog.find( '"', knPosValue ) - knPosValue );
  };
  for ( size_t nPos = strCatalog.find( "<TEST " ); std::string::npos != nPos; nPos = strCatalog.find( "<TEST ", nPos + 1 ) )
    rgTests.push_back( NamespaceSuiteTest{ StrAttribute( nPos, "URI" ), StrAttribute( nPos, "ID" ), StrAttribute( nPos, "TYPE" ) } );
  return rgTests;
}
// What reading a document of the suite found.
struct NamespaceSuiteRead
{
  EConformanceValidity m_ecv{ ecvNotJudged };
  bool m_fThrew{ false }; // Reading it threw: then it is not judged and m_strWhy is what was thrown.
  std::string m_strWhy;
  std::string m_strWritten; // What the writer wrote from it - in UTF-8 - if reading it didn't throw.
  std::string m_strRoundTrip; // How what was written is not the same document - or that writing it threw: empty if nothing.
};
NamespaceSuiteRead ReadNamespaceSuiteDocument( fs::path const & _rpath, bool _fUseXMLNamespaces )
{
  NamespaceSuiteRead read;
  std::ifstream ifs( _rpath, std::ios::binary );
  std::stringstream ss;
  ss << ifs.rdbuf();
  const std::string strBytes = ss.str();
  auto Open = [&]( TyXmlParserVarDtd & _rxmlParser )
  {
    _rxmlParser.SetReadExternalEntityFiles( SuiteFileEntityOptions() );
    _rxmlParser.SetBaseLocation( _rpath.generic_string() );
    _rxmlParser.SetUseXMLNamespaces( _fUseXMLNamespaces );
    return _rxmlParser.OpenMemoryVar( strBytes.data(), strBytes.size() );
  };
  try
  {
    TyXmlParserVarDtd xmlParser;
    TyXmlParserVarDtd::_TyReadCursorVar cursor = Open( xmlParser );
    read.m_ecv = std::visit(
      [&read]( auto & _tCursor ) -> EConformanceValidity
      {
        return ConformanceVerdict( _tCursor, read.m_strWhy );
      }, cursor.GetVariant() );
    read.m_fThrew = ( ecvNotJudged == read.m_ecv ) && ( 0 == read.m_strWhy.find( "not read:" ) );
  }
  catch ( std::exception const & rexc )
  {
    read.m_strWhy = std::string( "not opened: " ) + rexc.what();
    read.m_fThrew = true;
  }
  if ( read.m_fThrew )
    return read;
  // Written from a cursor - and what was written is read: it is to be the same document.
  try
  {
    {
      TyXmlParserVarDtd xmlParser;
      TyXmlParserVarDtd::_TyReadCursorVar cursor = Open( xmlParser );
      const std::u8string strWritten = StrWriteFromAnyCursor< char8_t >( cursor );
      read.m_strWritten.assign( strWritten.begin(), strWritten.end() );
    }
    TyXmlParserVarDtd xmlParser;
    TyXmlParserVarDtd::_TyReadCursorVar cursor = Open( xmlParser );
    CursorDocument< char8_t, true > docWritten( read.m_strWritten.c_str(), &FReadExternalEntityFile< char8_t >, _rpath.generic_string().c_str(), _fUseXMLNamespaces );
    if ( !docWritten.FOpened() )
      read.m_strRoundTrip = "what was written wasn't opened: " + docWritten.m_strError;
    else
    {
      xml_compare_options options;
      options.m_fCompareDoctypeDecl = true; // It is written as it was read.
      std::string strDifference;
      if ( !FCompareXmlDocuments( cursor, docWritten.m_cursor, options, &strDifference ) )
        read.m_strRoundTrip = strDifference.empty() ? std::string( "what was written is another document" ) : strDifference;
    }
  }
  catch ( std::exception const & rexc )
  {
    read.m_strRoundTrip = std::string( "exception: " ) + rexc.what();
  }
  return read;
}

class NamespacesInXml10 : public testing::TestWithParam< NamespaceSuiteTest >
{
};
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST( NamespacesInXml10 ); // The suite may not be here.

TEST_P( NamespacesInXml10, Document )
{
  NamespaceSuiteTest const & rtest = GetParam();
  ASSERT_FALSE( rtest.m_strUri.empty() || rtest.m_strType.empty() ) << "A test of the catalog which isn't understood.";
  const fs::path pathFile = PathNamespaceSuite() / rtest.m_strUri;
  const NamespaceSuiteRead read = ReadNamespaceSuiteDocument( pathFile, true );
  const NamespaceSuiteRead readWithout = ReadNamespaceSuiteDocument( pathFile, false );
  // Without namespaces it is a document of XML 1.0 - well formed, but for the one which has the same attribute twice
  //  and the one whose bytes - as they are in the suite - are not UTF-8: a byte of ISO-8859-1 in a namespace name.
  static const std::set< std::string > s_setNotWellFormedWithoutNamespaces{ "035.xml", "006.xml" };
  EXPECT_EQ( readWithout.m_fThrew, s_setNotWellFormedWithoutNamespaces.contains( rtest.m_strUri ) ) << "without namespaces: " << readWithout.m_strWhy;
  EXPECT_EQ( readWithout.m_strRoundTrip, "" ) << "without namespaces - written:\n" << readWithout.m_strWritten;
  if ( "valid" == rtest.m_strType )
  {
    EXPECT_EQ( read.m_ecv, ecvValid ) << "It is valid: " << read.m_strWhy;
    EXPECT_EQ( readWithout.m_ecv, ecvValid ) << "It is valid - without namespaces: " << readWithout.m_strWhy;
  }
  else
  if ( "invalid" == rtest.m_strType )
  {
    EXPECT_FALSE( read.m_fThrew ) << "It is namespace-well-formed: " << read.m_strWhy;
    EXPECT_EQ( read.m_ecv, ecvNotValid ) << "It is well formed and not valid: " << read.m_strWhy;
  }
  else
  if ( "not-wf" == rtest.m_strType )
    EXPECT_TRUE( read.m_fThrew ) << "It is not namespace-well-formed and was read.";
  else
    EXPECT_EQ( rtest.m_strType, "error" ); // What a processor may report or not: nothing is required.
  EXPECT_EQ( read.m_strRoundTrip, "" ) << "written:\n" << read.m_strWritten;
}
INSTANTIATE_TEST_SUITE_P( W3C, NamespacesInXml10, testing::ValuesIn( RgNamespaceSuiteTests() ),
  []( testing::TestParamInfo< NamespaceSuiteTest > const & _rinfo )
  {
    // The ID of the catalog as the name of the test: the characters which a name of a test may have.
    std::string strName = _rinfo.param.m_strId.empty() ? ( "test" + std::to_string( _rinfo.index ) ) : _rinfo.param.m_strId;
    for ( char & rch : strName )
      if ( !isalnum( (unsigned char)rch ) )
        rch = '_';
    return strName;
  } );
// All of the catalog is here.
TEST( DtdConformance, NamespacesInXml10Catalog )
{
  const std::vector< NamespaceSuiteTest > rgTests = RgNamespaceSuiteTests();
  if ( rgTests.empty() )
    GTEST_SKIP() << "No tests of namespaces at " << PathNamespaceSuite().generic_string();
  std::map< std::string, size_t > mapTypes;
  for ( NamespaceSuiteTest const & rtest : rgTests )
    ++mapTypes[ rtest.m_strType ];
  EXPECT_EQ( rgTests.size(), 48u );
  EXPECT_EQ( mapTypes[ "valid" ], 7u );
  EXPECT_EQ( mapTypes[ "invalid" ], 17u );
  EXPECT_EQ( mapTypes[ "not-wf" ], 21u );
  EXPECT_EQ( mapTypes[ "error" ], 3u );
}

// The W3C XML Conformance Test Suite by its catalog - xmlconf.xml: every test of it which is for what we read.
// The catalog is itself a document of the kind we are to read: it has an external subset - which gives its <TEST>s
//  attributes by default - and its test cases are in external general entities whose text is content. We read it with
//  the cursor.
struct SuiteTest
{
  std::string m_strId;
  std::string m_strPath; // The document - relative to the directory of the suite.
  std::string m_strType; // "valid", "invalid", "not-wf" or "error".
  std::string m_strEntities; // The external entities which are to be read for the document to be what it is: "none", "parameter", "general" or "both".
  std::string m_strRecommendation; // "XML1.0", "NS1.0", "XML1.1", ...
  std::string m_strEdition; // The editions of XML 1.0 which the test is for - empty: all of them.
  std::string m_strOutput; // The canonical form of the document - relative to the directory of the suite - if there is one.
  bool m_fNamespace{ true }; // The document is for a processor of namespaces.
};
void PrintTo( SuiteTest const & _rtest, std::ostream * _pos )
{
  *_pos << _rtest.m_strId << " ( " << _rtest.m_strPath << ", " << _rtest.m_strType << " )";
}
// The tests of the catalog - none if the suite is not here. _pstrError: why the catalog was not read, if it wasn't.
std::vector< SuiteTest > RgSuiteTestsRead( std::string * _pstrError )
{
  typedef xml_traits< _l_transport_fixedmem< char8_t >, true, true > TyXmlTraits;
  typedef xml_parser< TyXmlTraits > TyXmlParser;
  std::vector< SuiteTest > rgTests;
  const fs::path pathCatalog = fs::path( XMLP_XMLCONF_DIR ) / "xmlconf.xml";
  std::ifstream ifs( pathCatalog, std::ios::binary );
  if ( !ifs )
    return rgTests;
  std::stringstream ss;
  ss << ifs.rdbuf();
  const std::string strBytes = ss.str();
  try
  {
    TyXmlParser xmlParser;
    xmlParser.SetReadExternalEntityFiles( SuiteFileEntityOptions() );
    const std::string strBase = pathCatalog.generic_string();
    xmlParser.SetBaseLocation( std::u8string( strBase.begin(), strBase.end() ) );
    typename TyXmlParser::_TyReadCursor cursor = xmlParser.OpenMemory( strBytes.data(), strBytes.size() );
    // The attributes of the current tag.
    auto GetAttributes = [&cursor]() -> std::map< std::string, std::string >
    {
      std::map< std::string, std::string > mapAttrs;
      auto & rltokTag = cursor.GetTagCur().GetLexToken();
      auto & rrgAttrs = rltokTag.GetValue()[ vknAttributesIdx ];
      for ( size_t nAttr = 0; nAttr < rrgAttrs.GetSize(); ++nAttr )
      {
        std::u8string strName, strValue;
        rltokTag.GetString( strName, rrgAttrs[ nAttr ][ vknNameIdx ] );
        rltokTag.GetString( strValue, rrgAttrs[ nAttr ][ vknAttr_ValueIdx ] );
        mapAttrs[ std::string( strName.begin(), strName.end() ) ] = std::string( strValue.begin(), strValue.end() );
      }
      return mapAttrs;
    };
    std::vector< std::string > rgstrBase; // The base of each of the tags which we are within: what the URIs of its tests are relative to.
    auto Visit = [&]( size_t _nDepth )
    {
      rgstrBase.resize( _nDepth );
      std::string strBaseCur = ( _nDepth > 1 ) ? rgstrBase[ _nDepth - 2 ] : std::string();
      const std::u8string_view svTag = cursor.SvTagCur();
      const std::string strTag( svTag.begin(), svTag.end() );
      const std::map< std::string, std::string > mapAttrs = GetAttributes();
      auto StrAttr = [&mapAttrs]( const char * _pszName ) -> std::string
      {
        auto cit = mapAttrs.find( _pszName );
        return ( mapAttrs.end() == cit ) ? std::string() : cit->second;
      };
      if ( "TESTCASES" == strTag )
        strBaseCur += StrAttr( "xml:base" );
      else
      if ( "TEST" == strTag )
      {
        SuiteTest test;
        test.m_strId = StrAttr( "ID" );
        test.m_strPath = strBaseCur + StrAttr( "URI" );
        test.m_strType = StrAttr( "TYPE" );
        test.m_strEntities = StrAttr( "ENTITIES" );
        test.m_strRecommendation = StrAttr( "RECOMMENDATION" );
        test.m_strEdition = StrAttr( "EDITION" );
        if ( !StrAttr( "OUTPUT" ).empty() )
          test.m_strOutput = strBaseCur + StrAttr( "OUTPUT" );
        test.m_fNamespace = ( "no" != StrAttr( "NAMESPACE" ) );
        rgTests.push_back( std::move( test ) );
      }
      rgstrBase[ _nDepth - 1 ] = strBaseCur;
    };
    for ( size_t nDepth = 0; ; )
    {
      if ( cursor.FMoveDown() )
      {
        Visit( ++nDepth );
        continue;
      }
      if ( !nDepth )
        break;
      if ( cursor.FNextTag() )
        Visit( nDepth );
      else
        --nDepth;
    }
    if ( !!_pstrError && !cursor.FIsValid() )
      *_pstrError = "the catalog is not valid: " + ( cursor.RgValidityErrors().empty() ? ( cursor.GetDtd().m_rgstrValidityErrors.empty() ? std::string( "?" ) : cursor.GetDtd().m_rgstrValidityErrors.front() ) : cursor.RgValidityErrors().front() );
  }
  catch ( std::exception const & rexc )
  {
    if ( !!_pstrError )
      *_pstrError = std::string( "exception: " ) + rexc.what();
  }
  return rgTests;
}
std::vector< SuiteTest > const & RgSuiteTests()
{
  static const std::vector< SuiteTest > s_rgTests = RgSuiteTestsRead( nullptr );
  return s_rgTests;
}

// What reading all of a document found.
enum ESuiteOutcome
{
  esoThrew, // It is not well formed - or what it is can't be read.
  esoValid,
  esoNotValid,
  esoRead, // It was read and nothing says whether it is valid: it wasn't validated - or what it is depends on what wasn't read.
  esoESuiteOutcomeCount
};
inline const char * PszSuiteOutcome( ESuiteOutcome _eso )
{
  static const char * const s_rgpsz[] = { "threw", "valid", "not valid", "read" };
  return s_rgpsz[ _eso ];
}
// What an exception of ours says - "[file:line],function: what" - without where in the code it is said.
inline std::string StrWhatWasThrown( const char * _pszWhat )
{
  std::string strWhat( _pszWhat );
  const size_t knPosFunction = ( '[' == strWhat[0] ) ? strWhat.find( "]," ) : std::string::npos;
  const size_t knPosWhat = ( std::string::npos == knPosFunction ) ? std::string::npos : strWhat.find( ": ", knPosFunction );
  if ( std::string::npos != knPosWhat )
    strWhat.erase( 0, knPosWhat + 2 );
  return strWhat;
}
// Read all of the document of the cursor - every string of it is had - and say what it is.
template < class t_TyCursor >
ESuiteOutcome SuiteOutcomeOfCursor( t_TyCursor & _rcursor, bool _fValidating, std::string & _rstrWhy )
{
  try
  {
    xml_compare_options options;
    options.m_fCompareDoctypeDecl = true;
    _xml_compare_reader< t_TyCursor > reader( _rcursor, options );
    _xml_compare_item item;
    do
    {
      reader.GetNext( item );
    }
    while ( excmpEndOfDocument != item.m_excmp );
  }
  catch ( std::exception const & rexc )
  {
    _rstrWhy = StrWhatWasThrown( rexc.what() );
    return esoThrew;
  }
  if ( !_fValidating )
    return esoRead;
  if ( !_rcursor.FHasDtd() )
  {
    _rstrWhy = _rcursor.RgValidityErrors().empty() ? std::string( "no document type declaration" ) : _rcursor.RgValidityErrors().front();
    return _rcursor.RgValidityErrors().empty() ? esoRead : esoNotValid;
  }
  auto const & rdtd = _rcursor.GetDtd();
  if ( !rdtd.m_rgstrValidityErrors.empty() || !_rcursor.RgValidityErrors().empty() )
  {
    _rstrWhy = !rdtd.m_rgstrValidityErrors.empty() ? rdtd.m_rgstrValidityErrors.front() : _rcursor.RgValidityErrors().front();
    return esoNotValid;
  }
  if ( !rdtd.FAllDeclarationsRead() || _rcursor.FValidationIncomplete() )
  {
    _rstrWhy = "what it is depends on what is not read";
    return esoRead;
  }
  return esoValid;
}
struct SuiteRead
{
  ESuiteOutcome m_eso{ esoThrew };
  std::string m_strWhy;
};
// Read a document which is in UTF-8, UTF-16 or UTF-32 with the variant parser.
//  _fValidating: by a validating cursor - otherwise by one which only supports the DTD.
//  _fReadExternalEntities: the external entities of the document are read - they are files beside it.
template < class t_TyXmlParserVar >
SuiteRead SuiteReadDocument( std::string const & _rstrBytes, fs::path const & _rpath, bool _fUseXMLNamespaces, bool _fReadExternalEntities )
{
  SuiteRead read;
  try
  {
    t_TyXmlParserVar xmlParser;
    if ( _fReadExternalEntities )
      xmlParser.SetReadExternalEntityFiles( SuiteFileEntityOptions() );
    xmlParser.SetBaseLocation( _rpath.generic_string() );
    xmlParser.SetUseXMLNamespaces( _fUseXMLNamespaces );
    typename t_TyXmlParserVar::_TyReadCursorVar cursor = xmlParser.OpenMemoryVar( _rstrBytes.data(), _rstrBytes.size() );
    read.m_eso = std::visit(
      [&read]( auto & _tCursor ) -> ESuiteOutcome
      {
        return SuiteOutcomeOfCursor( _tCursor, t_TyXmlParserVar::s_kfValidating, read.m_strWhy );
      }, cursor.GetVariant() );
  }
  catch ( std::exception const & rexc )
  {
    read.m_strWhy = StrWhatWasThrown( rexc.what() );
    read.m_eso = esoThrew;
  }
  return read;
}
typedef xml_parser_var< xml_var_get_var_transport_t, tuple< tuple< char8_t >, tuple< char16_t >, tuple< char32_t > >, true, false > TyXmlParserVarDtdNotValidating;

// The encoding which the document declares - lower case - if its bytes are of an encoding of a byte for a character
//  of ASCII and it has an XML declaration which declares one: empty otherwise.
std::string StrDeclaredEncodingOfDocument( std::string const & _rstrBytes )
{
  if ( ( _rstrBytes.size() < 6 ) || ( 0 != _rstrBytes.compare( 0, 5, "<?xml" ) ) )
    return std::string();
  const size_t knPosEnd = _rstrBytes.find( "?>" );
  const size_t knPosEncoding = _rstrBytes.find( "encoding" );
  if ( ( std::string::npos == knPosEnd ) || ( std::string::npos == knPosEncoding ) || ( knPosEncoding > knPosEnd ) )
    return std::string();
  const size_t knPosQuote = _rstrBytes.find_first_of( "'\"", knPosEncoding );
  if ( ( std::string::npos == knPosQuote ) || ( knPosQuote > knPosEnd ) )
    return std::string();
  const size_t knPosQuoteEnd = _rstrBytes.find( _rstrBytes[ knPosQuote ], knPosQuote + 1 );
  if ( ( std::string::npos == knPosQuoteEnd ) || ( knPosQuoteEnd > knPosEnd ) )
    return std::string();
  std::string strEncoding = _rstrBytes.substr( knPosQuote + 1, knPosQuoteEnd - knPosQuote - 1 );
  for ( char & rch : strEncoding )
    rch = (char)tolower( (unsigned char)rch );
  return strEncoding;
}

// Whether the bytes are UTF-8: each character is encoded in as few bytes as it can be, none is a surrogate or beyond
//  #x10FFFF - and none is #x0, which no document has.
inline bool FIsUTF8( std::string const & _rstrBytes )
{
  for ( size_t n = 0; n < _rstrBytes.size(); )
  {
    const uint8_t by = uint8_t( _rstrBytes[n] );
    const size_t knBytes = ( by < 0x80 ) ? 1 : ( ( ( by & 0xE0 ) == 0xC0 ) ? 2 : ( ( ( by & 0xF0 ) == 0xE0 ) ? 3 : ( ( ( by & 0xF8 ) == 0xF0 ) ? 4 : 0 ) ) );
    if ( !knBytes || ( n + knBytes > _rstrBytes.size() ) )
      return false;
    uint32_t ch = ( 1 == knBytes ) ? by : ( by & ( 0xFF >> ( knBytes + 1 ) ) );
    for ( size_t nByte = 1; nByte < knBytes; ++nByte )
    {
      const uint8_t byNext = uint8_t( _rstrBytes[n + nByte] );
      if ( ( byNext & 0xC0 ) != 0x80 )
        return false;
      ch = ( ch << 6 ) | ( byNext & 0x3F );
    }
    static const uint32_t s_rgchFirstOfLength[] = { 0, 0, 0x80, 0x800, 0x10000 };
    if ( !ch || ( ch < s_rgchFirstOfLength[knBytes] ) || ( ch > 0x10FFFF ) || ( ( ch >= 0xD800 ) && ( ch <= 0xDFFF ) ) )
      return false;
    n += knBytes;
  }
  return true;
}
// What is found of one test of the suite - empty strings for what is as it should be.
struct SuiteResult
{
  std::string m_strSkipped; // Why the test is not for us - then nothing else is said.
  std::string m_strValidating; // Read by a validating cursor with its external entities the document is not what the catalog says.
  std::string m_strNotValidating; // Read by a cursor which doesn't validate and without its external entities it is not.
  std::string m_strCanonical; // What is read is not what the canonical form of the document is.
  std::string m_strRoundTrip; // What the writer writes from it is not the same document.
  std::string m_strUTF16; // The document in UTF-16 - if it is in UTF-8 and doesn't declare its encoding - is not what the catalog says, read as the first is.
  std::string m_strUTF32; // The same in UTF-32.
};
SuiteResult RunSuiteTest( SuiteTest const & _rtest )
{
  SuiteResult result;
  if ( ( std::string::npos != _rtest.m_strRecommendation.find( "1.1" ) ) )
  {
    result.m_strSkipped = "for XML 1.1";
    return result;
  }
  if ( !_rtest.m_strEdition.empty() && ( std::string::npos == _rtest.m_strEdition.find( '5' ) ) )
  {
    result.m_strSkipped = "for editions of XML 1.0 before the fifth: " + _rtest.m_strEdition;
    return result;
  }
  const fs::path pathFile = fs::path( XMLP_XMLCONF_DIR ) / _rtest.m_strPath;
  std::ifstream ifs( pathFile, std::ios::binary );
  if ( !ifs )
  {
    result.m_strSkipped = "no file";
    return result;
  }
  std::stringstream ss;
  ss << ifs.rdbuf();
  const std::string strBytes = ss.str();
  const std::string strEncoding = StrDeclaredEncodingOfDocument( strBytes );
  if ( !strEncoding.empty() && ( "utf-8" != strEncoding ) && ( "utf-16" != strEncoding ) && ( "us-ascii" != strEncoding ) )
  {
    result.m_strSkipped = "in an encoding which a document may not be in for us: " + strEncoding;
    return result;
  }
  // By a validating cursor, with its external entities.
  const SuiteRead readValidating = SuiteReadDocument< TyXmlParserVarDtd >( strBytes, pathFile, _rtest.m_fNamespace, true );
  auto Said = []( const char * _pszExpected, SuiteRead const & _rread )
  {
    return std::string( "expected " ) + _pszExpected + ", " + PszSuiteOutcome( _rread.m_eso ) + ( _rread.m_strWhy.empty() ? std::string() : ( ": " + _rread.m_strWhy ) );
  };
  if ( ( "valid" == _rtest.m_strType ) && ( esoValid != readValidating.m_eso ) )
    result.m_strValidating = Said( "valid", readValidating );
  else
  if ( ( "invalid" == _rtest.m_strType ) && ( esoNotValid != readValidating.m_eso ) )
    result.m_strValidating = Said( "not valid", readValidating );
  else
  if ( ( "not-wf" == _rtest.m_strType ) && ( esoThrew != readValidating.m_eso ) )
    result.m_strValidating = Said( "not well formed", readValidating );
  // The same document in the other encodings: their lexical analyzers are others. ( Only a document whose bytes are
  //  UTF-8: one which has bytes that are not the encoding of a character is no document in another encoding. )
  if ( strEncoding.empty() && FIsUTF8( strBytes ) )
  {
    auto Judge = [&]( SuiteRead const & _rread ) -> std::string
    {
      if ( ( "valid" == _rtest.m_strType ) && ( esoValid != _rread.m_eso ) )
        return Said( "valid", _rread );
      if ( ( "invalid" == _rtest.m_strType ) && ( esoNotValid != _rread.m_eso ) )
        return Said( "not valid", _rread );
      if ( ( "not-wf" == _rtest.m_strType ) && ( esoThrew != _rread.m_eso ) )
        return Said( "not well formed", _rread );
      return std::string();
    };
    try
    {
      const size_t knBOM = ( ( strBytes.size() >= 3 ) && ( 0 == strBytes.compare( 0, 3, "\xEF\xBB\xBF" ) ) ) ? 3 : 0;
      std::u16string str16( 1, char16_t( 0xFEFF ) );
      std::u32string str32( 1, char32_t( 0xFEFF ) );
      {
        std::u16string strConverted;
        ConvertString( strConverted, (const char8_t *)strBytes.data() + knBOM, strBytes.size() - knBOM );
        str16 += strConverted;
      }
      {
        std::u32string strConverted;
        ConvertString( strConverted, (const char8_t *)strBytes.data() + knBOM, strBytes.size() - knBOM );
        str32 += strConverted;
      }
      const std::string strBytes16( (const char *)str16.data(), str16.size() * sizeof( char16_t ) );
      const std::string strBytes32( (const char *)str32.data(), str32.size() * sizeof( char32_t ) );
      result.m_strUTF16 = Judge( SuiteReadDocument< TyXmlParserVarDtd >( strBytes16, pathFile, _rtest.m_fNamespace, true ) );
      result.m_strUTF32 = Judge( SuiteReadDocument< TyXmlParserVarDtd >( strBytes32, pathFile, _rtest.m_fNamespace, true ) );
    }
    catch ( std::exception const & )
    {
      // Not UTF-8 which can be converted: there is no such document in the other encodings.
    }
  }
  // By one which doesn't validate, without them: then what is wrong with a document may be where it isn't read.
  const SuiteRead readNotValidating = SuiteReadDocument< TyXmlParserVarDtdNotValidating >( strBytes, pathFile, _rtest.m_fNamespace, false );
  // ( A reference to an entity which is declared where we didn't read - or whose text is there - is not an error, and
  //  there is no text to give for it: asking for the string which has it throws. That is how it is: not a failure. )
  const bool kfEntityNotRead = ( "none" != _rtest.m_strEntities ) && ( std::string::npos != readNotValidating.m_strWhy.find( "Can't find entity" ) );
  if ( ( ( "valid" == _rtest.m_strType ) || ( "invalid" == _rtest.m_strType ) ) && ( esoThrew == readNotValidating.m_eso ) && !kfEntityNotRead )
    result.m_strNotValidating = Said( "well formed", readNotValidating );
  else
  if ( ( "not-wf" == _rtest.m_strType ) && ( "none" == _rtest.m_strEntities ) && ( esoThrew != readNotValidating.m_eso ) )
    result.m_strNotValidating = Said( "not well formed", readNotValidating );
  if ( esoThrew == readValidating.m_eso )
    return result;
  // The canonical form: the document as it is read is that document - which has no DTD and no comments.
  if ( !_rtest.m_strOutput.empty() )
  {
    try
    {
      std::ifstream ifsOutput( fs::path( XMLP_XMLCONF_DIR ) / _rtest.m_strOutput, std::ios::binary );
      std::stringstream ssOutput;
      ssOutput << ifsOutput.rdbuf();
      const std::string strOutput = ssOutput.str();
      TyXmlParserVarDtd xmlParser;
      xmlParser.SetReadExternalEntityFiles( SuiteFileEntityOptions() );
      xmlParser.SetBaseLocation( pathFile.generic_string() );
      xmlParser.SetUseXMLNamespaces( _rtest.m_fNamespace );
      TyXmlParserVarDtd::_TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strBytes.data(), strBytes.size() );
      TyXmlParserVarDtdNotValidating xmlParserOutput;
      xmlParserOutput.SetUseXMLNamespaces( _rtest.m_fNamespace );
      TyXmlParserVarDtdNotValidating::_TyReadCursorVar cursorOutput = xmlParserOutput.OpenMemoryVar( strOutput.data(), strOutput.size() );
      xml_compare_options options;
      options.m_fCompareComments = false;
      std::string strDifference;
      if ( !FCompareXmlDocuments( cursor, cursorOutput, options, &strDifference ) )
        result.m_strCanonical = strDifference.empty() ? std::string( "another document" ) : strDifference;
    }
    catch ( std::exception const & rexc )
    {
      result.m_strCanonical = std::string( "exception: " ) + rexc.what();
    }
  }
  // Written from a cursor - and what was written is read: it is to be the same document.
  try
  {
    std::string strWritten;
    {
      TyXmlParserVarDtd xmlParser;
      xmlParser.SetReadExternalEntityFiles( SuiteFileEntityOptions() );
      xmlParser.SetBaseLocation( pathFile.generic_string() );
      xmlParser.SetUseXMLNamespaces( _rtest.m_fNamespace );
      TyXmlParserVarDtd::_TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strBytes.data(), strBytes.size() );
      const std::u8string strWritten8 = StrWriteFromAnyCursor< char8_t >( cursor );
      strWritten.assign( strWritten8.begin(), strWritten8.end() );
    }
    TyXmlParserVarDtd xmlParser;
    xmlParser.SetReadExternalEntityFiles( SuiteFileEntityOptions() );
    xmlParser.SetBaseLocation( pathFile.generic_string() );
    xmlParser.SetUseXMLNamespaces( _rtest.m_fNamespace );
    TyXmlParserVarDtd::_TyReadCursorVar cursor = xmlParser.OpenMemoryVar( strBytes.data(), strBytes.size() );
    TyXmlParserVarDtd xmlParserWritten;
    xmlParserWritten.SetReadExternalEntityFiles( SuiteFileEntityOptions() );
    xmlParserWritten.SetBaseLocation( pathFile.generic_string() );
    xmlParserWritten.SetUseXMLNamespaces( _rtest.m_fNamespace );
    TyXmlParserVarDtd::_TyReadCursorVar cursorWritten = xmlParserWritten.OpenMemoryVar( strWritten.data(), strWritten.size() );
    xml_compare_options options;
    options.m_fCompareDoctypeDecl = true; // It is written as it was read.
    std::string strDifference;
    if ( !FCompareXmlDocuments( cursor, cursorWritten, options, &strDifference ) )
      result.m_strRoundTrip = strDifference.empty() ? std::string( "another document" ) : strDifference;
  }
  catch ( std::exception const & rexc )
  {
    result.m_strRoundTrip = std::string( "exception: " ) + rexc.what();
  }
  return result;
}

// The tests of the suite which are known not to pass: unittests/xmlconf_expected_failures.txt - a line is how the
//  document was read, the ID of the test and why: see the file. Here: how it was read, a tab and the ID.
std::set< std::string > const & SetSuiteExpectedFailures()
{
  static const std::set< std::string > s_set = []()
  {
    std::set< std::string > set;
    std::ifstream ifs( fs::path( XMLP_XMLCONF_DIR ).parent_path() / "xmlconf_expected_failures.txt" );
    for ( std::string strLine; std::getline( ifs, strLine ); )
    {
      if ( strLine.empty() || ( '#' == strLine[0] ) )
        continue;
      const size_t knPosTab = strLine.find( '\t' );
      const size_t knPosTabSecond = ( std::string::npos == knPosTab ) ? std::string::npos : strLine.find( '\t', knPosTab + 1 );
      if ( std::string::npos != knPosTab )
        set.insert( strLine.substr( 0, knPosTabSecond ) );
    }
    return set;
  }();
  return s_set;
}
// The tests are run in groups - by where their documents are: "xmltest/not-wf", "ibm/valid", "oasis", ...
std::string StrSuiteGroup( SuiteTest const & _rtest )
{
  const size_t knPosFirst = _rtest.m_strPath.find( '/' );
  if ( std::string::npos == knPosFirst )
    return "suite";
  const size_t knPosSecond = _rtest.m_strPath.find( '/', knPosFirst + 1 );
  const size_t knPosThird = ( std::string::npos == knPosSecond ) ? std::string::npos : _rtest.m_strPath.find( '/', knPosSecond + 1 );
  // The directory of the document - no more than two of its names.
  return _rtest.m_strPath.substr( 0, ( std::string::npos == knPosSecond ) ? knPosFirst : ( ( std::string::npos == knPosThird ) && ( 0 != _rtest.m_strPath.compare( 0, 5, "eduni" ) ) ? knPosFirst : knPosSecond ) );
}
std::vector< std::string > RgSuiteGroups()
{
  std::vector< std::string > rgGroups;
  for ( SuiteTest const & rtest : RgSuiteTests() )
  {
    const std::string strGroup = StrSuiteGroup( rtest );
    if ( rgGroups.end() == std::find( rgGroups.begin(), rgGroups.end(), strGroup ) )
      rgGroups.push_back( strGroup );
  }
  return rgGroups;
}

// Every test of the catalog which is for what we read - a test of ours for each group of them. Each document is read
//  as RunSuiteTest() reads it, and what it is found to be is to be what the catalog says - but for the documents which
//  are listed as known not to be: those are to fail as they are listed, so that the list is what is so.
class W3CSuite : public testing::TestWithParam< std::string >
{
};
GTEST_ALLOW_UNINSTANTIATED_PARAMETERIZED_TEST( W3CSuite ); // The suite may not be here.

TEST_P( W3CSuite, Documents )
{
  const std::string & rstrGroup = GetParam();
  std::set< std::string > const & rsetExpectedFailures = SetSuiteExpectedFailures();
  size_t nRun = 0, nSkipped = 0, nKnownFailures = 0;
  for ( SuiteTest const & rtest : RgSuiteTests() )
  {
    if ( StrSuiteGroup( rtest ) != rstrGroup )
      continue;
    const SuiteResult result = RunSuiteTest( rtest );
    if ( !result.m_strSkipped.empty() )
    {
      ++nSkipped;
      continue;
    }
    ++nRun;
    auto Check = [&]( const char * _pszHow, const char * _pszHowRead, std::string const & _rstrSaid )
    {
      const bool kfExpected = rsetExpectedFailures.contains( std::string( _pszHow ) + "\t" + rtest.m_strId );
      nKnownFailures += ( kfExpected && !_rstrSaid.empty() ) ? 1 : 0;
      if ( !_rstrSaid.empty() && !kfExpected )
        ADD_FAILURE() << rtest.m_strId << " ( " << rtest.m_strPath << ", " << rtest.m_strType << " ) " << _pszHowRead << ": "
                      << ( ( _rstrSaid.length() > 500 ) ? ( _rstrSaid.substr( 0, 150 ) + " ... " + _rstrSaid.substr( _rstrSaid.length() - 300 ) ) : _rstrSaid );
      else
      if ( _rstrSaid.empty() && kfExpected )
        ADD_FAILURE() << rtest.m_strId << " ( " << rtest.m_strPath << " ) is listed in xmlconf_expected_failures.txt as failing [" << _pszHow
                      << "] and no longer does: its line is to be taken away.";
    };
    Check( "V", "read by a validating cursor with its external entities", result.m_strValidating );
    Check( "N", "read without validating and without its external entities", result.m_strNotValidating );
    Check( "C", "compared with its canonical form", result.m_strCanonical );
    Check( "R", "written and read again", result.m_strRoundTrip );
    Check( "16", "converted to UTF-16", result.m_strUTF16 );
    Check( "32", "converted to UTF-32", result.m_strUTF32 );
  }
  std::cout << "[          ] " << rstrGroup << ": " << nRun << " documents, " << nSkipped << " not for us, " << nKnownFailures << " known failures" << std::endl;
}
INSTANTIATE_TEST_SUITE_P( W3C, W3CSuite, testing::ValuesIn( RgSuiteGroups() ),
  []( testing::TestParamInfo< std::string > const & _rinfo )
  {
    std::string strName = _rinfo.param;
    for ( char & rch : strName )
      if ( !isalnum( (unsigned char)rch ) )
        rch = '_';
    return strName;
  } );
// The catalog of the suite is read by the cursor: it is valid - and all of it is here.
TEST( DtdConformance, SuiteCatalog )
{
  std::string strError;
  const std::vector< SuiteTest > rgTests = RgSuiteTestsRead( &strError );
  if ( rgTests.empty() && strError.empty() )
    GTEST_SKIP() << "No suite at " << XMLP_XMLCONF_DIR;
  EXPECT_EQ( strError, "" );
  EXPECT_EQ( rgTests.size(), 2585u );
  size_t nForUs = 0, nWithCanonicalForm = 0;
  for ( SuiteTest const & rtest : rgTests )
  {
    const bool kfForUs = ( std::string::npos == rtest.m_strRecommendation.find( "1.1" ) ) && ( rtest.m_strEdition.empty() || ( std::string::npos != rtest.m_strEdition.find( '5' ) ) );
    nForUs += kfForUs ? 1 : 0;
    nWithCanonicalForm += ( kfForUs && !rtest.m_strOutput.empty() ) ? 1 : 0;
  }
  EXPECT_EQ( nForUs, 2002u ); // The others are for XML 1.1 or for the editions of XML 1.0 before the fifth.
  EXPECT_GE( nWithCanonicalForm, 387u );
}

// The external subsets of the suite are read by xml_dtd_parser - with the external parameter entities which they refer
//  to, which are files beside them, and with the references to parameter entities within their declarations.
TEST( DtdConformance, ExternalSubsetsAreRead )
{
  if ( !FHaveConformanceSuite() )
    GTEST_SKIP() << "No conformance suite at " << XMLP_XMLCONF_DIR;
  // A text declaration without an encoding declaration, which [77] requires.
  const std::set< std::string > setNotExternalSubsets{ "ibm/invalid/P51/ibm51i03.dtd" };
  size_t nRead = 0;
  for ( const char * pszDir : { "ibm/valid", "ibm/invalid", "sun/valid", "sun/invalid", "xmltest/valid", "xmltest/invalid" } )
  {
    for ( std::string const & rstrFile : RgConformanceFiles( pszDir, ".dtd" ) )
    {
      const fs::path pathFile = PathConformanceFile( rstrFile );
      std::ifstream ifs( pathFile, std::ios::binary );
      const std::string strBytes( ( std::istreambuf_iterator< char >( ifs ) ), std::istreambuf_iterator< char >() );
      std::string strError;
      try
      {
        std::u8string strText;
        DecodeExternalEntity( (const uint8_t *)strBytes.data(), strBytes.size(), strText );
        const std::string strBase = pathFile.generic_string();
        xml_dtd< char8_t > dtd;
        xml_dtd_parser< char8_t > dtdParser( dtd );
        dtdParser.SetExternalEntityResolver( &FReadExternalEntityFile< char8_t >, std::u8string( strBase.begin(), strBase.end() ) );
        dtdParser.ParseExternalSubset( strText );
      }
      catch ( std::exception const & rexc )
      {
        strError = rexc.what();
        if ( strError.empty() )
          strError = "error";
      }
      if ( setNotExternalSubsets.contains( rstrFile ) )
        EXPECT_FALSE( strError.empty() ) << rstrFile;
      else
      {
        EXPECT_EQ( strError, "" ) << rstrFile;
        nRead += strError.empty() ? 1 : 0;
      }
    }
  }
  EXPECT_GE( nRead, 45u );
}
#endif //XMLP_XMLCONF_DIR

} // namespace ns_XMLGTestDtd

int main( int argc, char ** argv )
{
  g_strProgramName = argv[0];
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}
