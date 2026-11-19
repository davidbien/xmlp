
// xmlpgen_dtd.cpp
// Generate the parser of the DTD: the document type declaration and the internal and external subsets.
// 19NOV2026
// The tags of an XML document are each a single token of the lexical analyzer and the cursor ( xml_cursor.h ) puts them
//  together by hand - that is unchanged. The DTD is different: its content models nest and its declarations are many
//  small things in sequence, so it has a grammar - and the specification writes that grammar down: productions [28]
//  through [83]. Here they are given to the parser generator ( pgen ) as they are written in the specification.
// The tokens are those of the DTD DFAs of the lexical analyzers ( xmlpgen_utf8.cpp, xmlpgen_utf16.cpp, xmlpgen_utf32.cpp ):
//  the identifiers of the tokens are the same in each so the one parser - _xmlpdtd.h - serves all three.
//
// Where a production isn't written as the specification writes it the specification's text is in the comment. There
//  is one reason for every such place: optional white space. As written the parser would need two tokens to know whether
//  an S belongs to something optional or comes after it - "(S ExternalID)? S?" - and it has one. The productions are
//  regrouped so that the S is read first and the decision made after it. The language is the same.
//
// Three things are asked of the parser generator:
//  - Entry non-terminals: the one parser parses a document type declaration [28], an external subset [30] or the
//    replacement text of a parameter entity which is referenced between declarations [31].
//  - Lazy lookahead: the parser doesn't read a token where the only thing possible is one reduction. So the parse of
//    "<!DOCTYPE ... >" ends at its '>' without the lexical analyzer being asked for a token of the DTD from the
//    document which follows it. And the modes below are then chosen in states which know where they are.
//  - The modes of the lexical analyzer: a keyword is also a Name and an Nmtoken, and the four kinds of quoted literal
//    are the same characters. Each of those is in a DFA of its own and the parser says, for each token, which DFA(s)
//    can produce a token that is possible where it is. The modes are the DFAs in the order given by the generators of
//    the lexical analyzers - the order in which more than one is tried.

#include "xmlpgen.h"

// The single letter macros of the generators of the lexical analyzer are not for here.
#undef l
#undef ls
#undef lr
#undef lanyof
#undef u
#undef t
#undef a

__REGEXP_USING_NAMESPACE
__XMLP_USING_NAMESPACE

void
GenerateDtdParser( ns_pg::LexerModes< vtyTokenIdent > const & _rmodes )
{
  using namespace ns_pg;
  // The types of the tokens are those of any of the lexical analyzers: only the identifiers of the tokens are used -
  //  and that they are tokens, and not triggers, is checked when this is compiled.
  typedef _l_transport_fixedmem< char8_t > _TyTransport;
  typedef xml_traits< _TyTransport, false, false > _TyXmlTraits;
  typedef _TyXmlTraits::_TyLexTraits _TyLexT;
  typedef Grammar< vtyDefaultSymbolType, vtyTokenIdent, _TyDefaultAllocator > _TyGrammar;

  _TyGrammar g;
// The terminals - named as the specification writes them.
  auto S = g.NewTerminal( "S", TyGetTokenDtdS<_TyLexT>() ); // [3]
  auto Name = g.NewTerminal( "Name", TyGetTokenDtdName<_TyLexT>() ); // [5]
  auto Nmtoken = g.NewTerminal( "Nmtoken", TyGetTokenDtdNmtoken<_TyLexT>() ); // [7]
  auto EntityValue = g.NewTerminal( "EntityValue", TyGetTokenDtdEntityValue<_TyLexT>() ); // [9]
  auto AttValue = g.NewTerminal( "AttValue", TyGetTokenDtdAttValue<_TyLexT>() ); // [10]
  auto SystemLiteral = g.NewTerminal( "SystemLiteral", TyGetTokenDtdSystemLiteral<_TyLexT>() ); // [11]
  auto PubidLiteral = g.NewTerminal( "PubidLiteral", TyGetTokenDtdPubidLiteral<_TyLexT>() ); // [12]
  auto Comment = g.NewTerminal( "Comment", TyGetTokenComment<_TyLexT>() ); // [15]
  auto PI = g.NewTerminal( "PI", TyGetTokenProcessingInstruction<_TyLexT>() ); // [16]
  auto PEReference = g.NewTerminal( "PEReference", TyGetTokenDtdPEReference<_TyLexT>() ); // [69]
  auto TextDecl = g.NewTerminal( "TextDecl", TyGetTokenDtdTextDecl<_TyLexT>() ); // [77]
  auto Ignore = g.NewTerminal( "Ignore", TyGetTokenDtdIgnore<_TyLexT>() ); // [65]
  auto kwDOCTYPE = g.NewTerminal( "'<!DOCTYPE'", TyGetTokenDtdDoctypeBegin<_TyLexT>() );
  auto kwELEMENT = g.NewTerminal( "'<!ELEMENT'", TyGetTokenDtdElementBegin<_TyLexT>() );
  auto kwATTLIST = g.NewTerminal( "'<!ATTLIST'", TyGetTokenDtdAttlistBegin<_TyLexT>() );
  auto kwENTITY = g.NewTerminal( "'<!ENTITY'", TyGetTokenDtdEntityBegin<_TyLexT>() );
  auto kwNOTATION = g.NewTerminal( "'<!NOTATION'", TyGetTokenDtdNotationBegin<_TyLexT>() );
  auto condStart = g.NewTerminal( "'<!['", TyGetTokenDtdCondSectBegin<_TyLexT>() );
  auto condEnd = g.NewTerminal( "']]>'", TyGetTokenDtdCondSectEnd<_TyLexT>() );
  auto lbracket = g.NewTerminal( "'['", TyGetTokenDtdLBracket<_TyLexT>() );
  auto rbracket = g.NewTerminal( "']'", TyGetTokenDtdRBracket<_TyLexT>() );
  auto gt = g.NewTerminal( "'>'", TyGetTokenDtdGreater<_TyLexT>() );
  auto lparen = g.NewTerminal( "'('", TyGetTokenDtdLParen<_TyLexT>() );
  auto rparen = g.NewTerminal( "')'", TyGetTokenDtdRParen<_TyLexT>() );
  auto bar = g.NewTerminal( "'|'", TyGetTokenDtdBar<_TyLexT>() );
  auto comma = g.NewTerminal( "','", TyGetTokenDtdComma<_TyLexT>() );
  auto qmark = g.NewTerminal( "'?'", TyGetTokenDtdQuestion<_TyLexT>() );
  auto star = g.NewTerminal( "'*'", TyGetTokenDtdStar<_TyLexT>() );
  auto plus = g.NewTerminal( "'+'", TyGetTokenDtdPlus<_TyLexT>() );
  auto percent = g.NewTerminal( "'%'", TyGetTokenDtdPercent<_TyLexT>() );
  auto kwPCDATA = g.NewTerminal( "'#PCDATA'", TyGetTokenDtdPCDATA<_TyLexT>() );
  auto kwREQUIRED = g.NewTerminal( "'#REQUIRED'", TyGetTokenDtdREQUIRED<_TyLexT>() );
  auto kwIMPLIED = g.NewTerminal( "'#IMPLIED'", TyGetTokenDtdIMPLIED<_TyLexT>() );
  auto kwFIXED = g.NewTerminal( "'#FIXED'", TyGetTokenDtdFIXED<_TyLexT>() );
  auto kwEMPTY = g.NewTerminal( "'EMPTY'", TyGetTokenDtdEMPTY<_TyLexT>() );
  auto kwANY = g.NewTerminal( "'ANY'", TyGetTokenDtdANY<_TyLexT>() );
  auto kwCDATA = g.NewTerminal( "'CDATA'", TyGetTokenDtdCDATA<_TyLexT>() );
  auto kwID = g.NewTerminal( "'ID'", TyGetTokenDtdID<_TyLexT>() );
  auto kwIDREF = g.NewTerminal( "'IDREF'", TyGetTokenDtdIDREF<_TyLexT>() );
  auto kwIDREFS = g.NewTerminal( "'IDREFS'", TyGetTokenDtdIDREFS<_TyLexT>() );
  auto kwENTITYType = g.NewTerminal( "'ENTITY'", TyGetTokenDtdENTITY<_TyLexT>() );
  auto kwENTITIES = g.NewTerminal( "'ENTITIES'", TyGetTokenDtdENTITIES<_TyLexT>() );
  auto kwNMTOKEN = g.NewTerminal( "'NMTOKEN'", TyGetTokenDtdNMTOKEN<_TyLexT>() );
  auto kwNMTOKENS = g.NewTerminal( "'NMTOKENS'", TyGetTokenDtdNMTOKENS<_TyLexT>() );
  auto kwNOTATIONType = g.NewTerminal( "'NOTATION'", TyGetTokenDtdNOTATION<_TyLexT>() );
  auto kwSYSTEM = g.NewTerminal( "'SYSTEM'", TyGetTokenDtdSYSTEM<_TyLexT>() );
  auto kwPUBLIC = g.NewTerminal( "'PUBLIC'", TyGetTokenDtdPUBLIC<_TyLexT>() );
  auto kwNDATA = g.NewTerminal( "'NDATA'", TyGetTokenDtdNDATA<_TyLexT>() );
  auto kwINCLUDE = g.NewTerminal( "'INCLUDE'", TyGetTokenDtdINCLUDE<_TyLexT>() );
  auto kwIGNORE = g.NewTerminal( "'IGNORE'", TyGetTokenDtdIGNORE<_TyLexT>() );

// The non-terminals - named as the specification names them.
  auto doctypedecl = g.NewNonTerminal( "doctypedecl" ), DeclSep = g.NewNonTerminal( "DeclSep" ), intSubset = g.NewNonTerminal( "intSubset" ),
    markupdecl = g.NewNonTerminal( "markupdecl" ), extSubset = g.NewNonTerminal( "extSubset" ), extSubsetDecl = g.NewNonTerminal( "extSubsetDecl" ),
    elementdecl = g.NewNonTerminal( "elementdecl" ), contentspec = g.NewNonTerminal( "contentspec" ), children = g.NewNonTerminal( "children" ),
    cp = g.NewNonTerminal( "cp" ), choice = g.NewNonTerminal( "choice" ), seq = g.NewNonTerminal( "seq" ), Mixed = g.NewNonTerminal( "Mixed" ),
    AttlistDecl = g.NewNonTerminal( "AttlistDecl" ), AttDef = g.NewNonTerminal( "AttDef" ), AttType = g.NewNonTerminal( "AttType" ),
    StringType = g.NewNonTerminal( "StringType" ), TokenizedType = g.NewNonTerminal( "TokenizedType" ), EnumeratedType = g.NewNonTerminal( "EnumeratedType" ),
    NotationType = g.NewNonTerminal( "NotationType" ), Enumeration = g.NewNonTerminal( "Enumeration" ), DefaultDecl = g.NewNonTerminal( "DefaultDecl" ),
    conditionalSect = g.NewNonTerminal( "conditionalSect" ), includeSect = g.NewNonTerminal( "includeSect" ), ignoreSect = g.NewNonTerminal( "ignoreSect" ),
    ignoreSectContents = g.NewNonTerminal( "ignoreSectContents" ), EntityDecl = g.NewNonTerminal( "EntityDecl" ), GEDecl = g.NewNonTerminal( "GEDecl" ),
    PEDecl = g.NewNonTerminal( "PEDecl" ), EntityDef = g.NewNonTerminal( "EntityDef" ), PEDef = g.NewNonTerminal( "PEDef" ),
    ExternalID = g.NewNonTerminal( "ExternalID" ), NDataDecl = g.NewNonTerminal( "NDataDecl" ), NotationDecl = g.NewNonTerminal( "NotationDecl" ),
    PublicID = g.NewNonTerminal( "PublicID" );

// The entries: what the parser can be asked to parse. s_knEntry_<non-terminal> in the generated header.
  g.AddEntryNonTerminal( doctypedecl ); // The document type declaration within a document.
  g.AddEntryNonTerminal( extSubset ); // An external subset: a file. ( And an external parameter entity [79] - the same thing. )
  g.AddEntryNonTerminal( extSubsetDecl ); // The replacement text of a parameter entity referenced between declarations - [28a], [31].

// The productions. EBNF as lexang's: x * y: y follows x, x | y: x or y, ~x: zero or more, ++x: one or more, --x: optional.
  doctypedecl = kwDOCTYPE * S * Name * ( S * ExternalID * --S | --S ) * --( lbracket * intSubset * rbracket * --S ) * gt; // [28] As written: '<!DOCTYPE' S Name (S ExternalID)? S? ('[' intSubset ']' S?)? '>'
  DeclSep = PEReference | S; // [28a]
  intSubset = ~( markupdecl | DeclSep ); // [28b]
  markupdecl = elementdecl | AttlistDecl | EntityDecl | NotationDecl | PI | Comment; // [29]
  extSubset = --TextDecl * extSubsetDecl; // [30]
  extSubsetDecl = ~( markupdecl | conditionalSect | DeclSep ); // [31]
  elementdecl = kwELEMENT * S * Name * S * contentspec * --S * gt; // [45]
  contentspec = kwEMPTY | kwANY | Mixed | children; // [46]
  children = ( choice | seq ) * --( qmark | star | plus ); // [47]
  cp = ( Name | choice | seq ) * --( qmark | star | plus ); // [48]
  choice = lparen * --S * cp * ++( --S * bar * --S * cp ) * --S * rparen; // [49]
  seq = lparen * --S * cp * ++( --S * comma * --S * cp ) * --S * rparen
      | lparen * --S * cp * --S * rparen; // [50] As written: '(' S? cp ( S? ',' S? cp )* S? ')'
  Mixed = lparen * --S * kwPCDATA * ++( --S * bar * --S * Name ) * --S * rparen * star
        | lparen * --S * kwPCDATA * --S * rparen * --star; // [51] As written: '(' S? '#PCDATA' (S? '|' S? Name)* S? ')*' | '(' S? '#PCDATA' S? ')'
                                                           // ')*' is the two tokens ')' and '*': there can be nothing between them since S is a token.
  AttlistDecl = kwATTLIST * S * Name * ~AttDef * --S * gt; // [52]
  AttDef = S * Name * S * AttType * S * DefaultDecl; // [53]
  AttType = StringType | TokenizedType | EnumeratedType; // [54]
  StringType = kwCDATA; // [55]
  TokenizedType = kwID | kwIDREF | kwIDREFS | kwENTITYType | kwENTITIES | kwNMTOKEN | kwNMTOKENS; // [56]
  EnumeratedType = NotationType | Enumeration; // [57]
  NotationType = kwNOTATIONType * S * lparen * --S * Name * ~( --S * bar * --S * Name ) * --S * rparen; // [58]
  Enumeration = lparen * --S * Nmtoken * ~( --S * bar * --S * Nmtoken ) * --S * rparen; // [59]
  DefaultDecl = kwREQUIRED | kwIMPLIED | --( kwFIXED * S ) * AttValue; // [60]
  conditionalSect = includeSect | ignoreSect; // [61]
  includeSect = condStart * --S * kwINCLUDE * --S * lbracket * extSubsetDecl * condEnd; // [62]
  ignoreSect = condStart * --S * kwIGNORE * --S * lbracket * ignoreSectContents * condEnd; // [63] As written: ... '[' ignoreSectContents* ']]>'
  ignoreSectContents = ~Ignore * ~( condStart * ignoreSectContents * condEnd * ~Ignore ); // [64] As written: Ignore ('<![' ignoreSectContents ']]>' Ignore)*
    // [65] Ignore ::= Char* - (Char* ('<![' | ']]>') Char*) - here the token Ignore is a piece of that and never empty, so
    //  Ignore is Ignore*. And [63]'s repetition of something which may be empty is ambiguous: it is one ignoreSectContents here.
  EntityDecl = GEDecl | PEDecl; // [70]
  GEDecl = kwENTITY * S * Name * S * EntityDef * gt; // [71] As written: '<!ENTITY' S Name S EntityDef S? '>' - the S? is in EntityDef.
  PEDecl = kwENTITY * S * percent * S * Name * S * PEDef * --S * gt; // [72]
  EntityDef = EntityValue * --S | ExternalID * --S | ExternalID * NDataDecl * --S; // [73] As written: EntityValue | (ExternalID NDataDecl?) - with [71]'s S?.
  PEDef = EntityValue | ExternalID; // [74]
  ExternalID = kwSYSTEM * S * SystemLiteral | PublicID * S * SystemLiteral; // [75] As written: 'SYSTEM' S SystemLiteral | 'PUBLIC' S PubidLiteral S SystemLiteral
  NDataDecl = S * kwNDATA * S * Name; // [76]
  NotationDecl = kwNOTATION * S * Name * S * ( ExternalID * --S | PublicID * --S ) * gt; // [82] As written: '<!NOTATION' S Name S (ExternalID | PublicID) S? '>'
  PublicID = kwPUBLIC * S * PubidLiteral; // [83]

  const bool fFinalized = g.FFinalize();
  for ( auto const & rgd : g.RgDiagnostics() )
    std::cout << "DTD grammar: " << rgd.m_strMessage << "\n";
  VerifyThrowSz( fFinalized && g.RgDiagnostics().empty(), "The grammar of the DTD has errors or warnings." );

  ParserGenerator< _TyGrammar > gen( g, ELRMethod::elrmLALR1 );
  gen.ReportConflicts( std::cout );
  gen.SetLazyLookahead();
  const bool fModesOk = gen.FSetLexerModes( _rmodes );
  gen.ReportLexerModes( std::cout );
  gen.ReportTableSizes( std::cout );
  VerifyThrowSz( fModesOk && !gen.NLexerDiagnostics( ELexerModeDiagnostic::elmdWarning ), "The grammar of the DTD doesn't agree with the lexical analyzer." );
  gen.GenerateFile( "_xmlpdtd.h", "XMLPDTD", "ns_xmlpdtd" ); // The identifier of a token is an int32_t - as vtyTokenIdent is.
  static_assert( std::is_same_v< vtyTokenIdent, int32_t > );
}
