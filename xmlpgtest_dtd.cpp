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
#include <optional>
#include <set>
#include <sstream>
#include "syslogmgr.inl"
#include "_compat.inl"
#include "xml_inc.h"
#include "xml_inc.h"
#include "xml_tag.h"
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
    "Notation [g] is declared more than once." } ) );
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
template < class t_TyChar, bool t_kfSupportDTD = true >
struct CursorDocument
{
  typedef xml_traits< _l_transport_fixedmem< t_TyChar >, t_kfSupportDTD, false > _TyXmlTraits;
  typedef xml_parser< _TyXmlTraits > _TyXmlParser;
  typedef xml_read_cursor< _TyXmlTraits > _TyCursor;
  std::basic_string< t_TyChar > m_strText; // The cursor reads this in place.
  _TyXmlParser m_xmlParser;
  _TyCursor m_cursor;
  std::string m_strError; // If the document couldn't be opened.

  explicit CursorDocument( const char * _pszText )
  {
    if ( sizeof( t_TyChar ) > 1 )
      m_strText += t_TyChar( 0xFEFF ); // The byte order mark: how the encoding of what is in memory is known.
    m_strText.append( _pszText, _pszText + strlen( _pszText ) );
    try
    {
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
  // An entity which has markup is not one of the document's entities yet: a reference to it is to an unknown entity.
  {
    CursorDocument< TyChar > doc( "<!DOCTYPE a [<!ENTITY m '<b/>'>]><a>x &m; y</a>" );
    ASSERT_TRUE( doc.FOpened() ) << doc.m_strError;
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_ANY_THROW( doc.StrContent() );
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
  // An entity must be declared before the default value which refers to it: well-formedness if the document is standalone.
  EXPECT_NE( lambdaError( "<!ATTLIST a x CDATA '&later;'><!ENTITY later 'v'>", true ).find( "refers to an entity which is not declared" ), std::string::npos );
  {
    xml_dtd< TyChar > dtdInvalid;
    ASSERT_EQ( StrReadDtd< TyChar >( "<!DOCTYPE a [<!ATTLIST a x CDATA '&later;' y CDATA 'ok'><!ENTITY later 'v'>]>", dtdInvalid ), "" );
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
  // These have references to parameter entities within declarations: they are accepted when those are substituted - then
  //  take them out of this list.
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
#endif //XMLP_XMLCONF_DIR

} // namespace ns_XMLGTestDtd

int main( int argc, char ** argv )
{
  g_strProgramName = argv[0];
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}
