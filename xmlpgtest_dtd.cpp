// xmlpgtest_dtd.cpp
// Google test stuff for XML Parser.
// The DTD: the tokens of the DTD DFAs of the lexical analyzers and the parser generated for the DTD ( xmlpgen_dtd.cpp ).
// The text of a document type declaration, an external subset or the replacement text of a parameter entity is parsed
//  in UTF-8, UTF-16 and UTF-32 - the one parser with each of the three lexical analyzers - into the parse tree, which is
//  all there is as yet: nothing is done with the declarations.
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
#include "syslogmgr.inl"
#include "_compat.inl"
#include "xml_inc.h"
#include "_p_lexang.h"
#include "_xmlpdtd.h"
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
    DtdParseResult result;
    const std::basic_string< _TyChar > strText( _pszText, _pszText + strlen( _pszText ) );
    try
    {
      _TyXmlParser xmlParser;
      xmlParser.emplaceTransport( strText.c_str(), strText.length() );
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

} // namespace ns_XMLGTestDtd

int main( int argc, char ** argv )
{
  g_strProgramName = argv[0];
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}
