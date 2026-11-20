
// xmlpgen_utf8.cpp
// Generate the UTF8 XML lexicographical analyzer.
// dbien
// 18FEB2021
// Pulling this stuff out so I can put each generator in a different file merely so I can easily diff the generators.

#include "xmlpgen.h"

__REGEXP_USING_NAMESPACE
__XMLP_USING_NAMESPACE

void
GenerateUTF8XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp, ns_pg::LexerModes< vtyTokenIdent > & _rmodesDtd )
{
  __REGEXP_OP_USING_NAMESPACE

  typedef _l_transport_fixedmem< char8_t > _TyTransport;
  typedef xml_traits< _TyTransport, false, false > _TyXmlTraits;
  typedef typename _TyXmlTraits::_TyLexTraits _TyLexTraits;
  typedef _TyLexTraits _TyLexT; // shorthand for below.

  typedef char8_t _TyCTok;
  typedef _TyDefaultAllocator  _TyAllocator;
  typedef _regexp_final< _TyCTok, _TyAllocator > _TyFinal;
  typedef _regexp_trigger< _TyCTok, _TyAllocator > _TyTrigger;
  typedef _regexp_action< _TyCTok, _TyAllocator > _TyFreeAction;

// Define some simple macros to make the productions more readable.
#define lnot(x) litnotset<_TyCTok>(x)

// Utility and multiple use non-triggered productions:
  _TyFinal	S = ++( l(0x20) | l(0x09) | l(0x0d) | l(0x0a) ); // [3]
  _TyFinal	Eq = --S * l(u8'=') * --S; // [25].
  _TyFinal	Char =	l(0x09) | l(0x0a) | l(0x0d) | lr(0x20,0xFF); // [2].

  _TyFinal NameStartChar = l(u8':') | lr(u8'A',u8'Z') | l(u8'_') | lr(u8'a',u8'z') | lr(0xC0,0xFF); // [4]


  _TyFinal NameChar = NameStartChar | l(u8'-') | l(u8'.') | lr(u8'0',u8'9') | lr(0x80,0xBF);	// [4a]
  _TyFinal Name = NameStartChar * ~NameChar; // [5]
  _TyFinal Names = Name * ~( S * Name ); // [6]
  _TyFinal Nmtoken = ++NameChar;
  _TyFinal Nmtokens = Nmtoken * ~( S * Nmtoken );

  _TyFinal NameStartCharNoColon = lr(u8'A',u8'Z') | l(u8'_') | lr(u8'a',u8'z') | lr(0xC0,0xFF); // [4]


  _TyFinal NameCharNoColon = NameStartCharNoColon | l(u8'-') | l(u8'.') | lr(u8'0',u8'9') | lr(0x80,0xBF);	// [4a]
  _TyFinal NCName = NameStartCharNoColon * ~NameCharNoColon;
  _TyFinal Prefix = NCName;
  _TyFinal LocalPart = NCName;

// Qualified name:
  // The name of a tag or of an attribute: any Name [5] of XML, in two pieces for Namespaces in XML. The first piece is the
  //  name up to its first colon which is not its first character; the second is what is after that colon - which may
  //  be nothing: then there is no second piece, as there is none for a name which has no such colon.
  // Whether the name is a QName [7] of Namespaces in XML - the first piece doesn't begin with a colon, and if the name has
  //  that colon then the second piece is an NCName [4] - is for the cursor to say when namespaces are in use: without
  //  them any Name is a name. ( This was the QName alone: a Name which is not one was then no token at all. )
  // Both triggers of the second piece are reached whenever the name has the colon - also when nothing follows it: a
  //  trigger which begins a piece and is left without its end is data which no token takes.
  _TyFinal _NameFirstPiece = NameStartChar * ~NameCharNoColon;
  _TyFinal QName = t(TyGetTriggerPrefixBegin<_TyLexT>()) * _NameFirstPiece * t(TyGetTriggerPrefixEnd<_TyLexT>()) *
                    --( l(u8':') * t( TyGetTriggerLocalPartBegin<_TyLexT>() ) * ~NameChar * t( TyGetTriggerLocalPartEnd<_TyLexT>() ) );
#if 0 // We have to rid these because they match the QName production and therefore they cause trigger confusion. We'll have to check in post-processing for xmlns as the prefix.
  _TyFinal	DefaultAttName = ls(u8"xmlns") /* * t( TyGetTriggerDefaultAttName<_TyLexT>() ) */;
  _TyFinal PrefixedAttName = ls(u8"xmlns:") * t( TyGetTriggerPrefixedAttNameBegin<_TyLexT>() ) * NCName * t( TyGetTriggerPrefixedAttNameEnd<_TyLexT>() );
  _TyFinal NSAttName = PrefixedAttName | DefaultAttName;
#endif //0

// Attribute value and character data productions:
  _TyFinal EntityRef = l(u8'&') * t( TyGetTriggerEntityRefBegin<_TyLexT>() ) 	// [49]
                                * Name * t( TyGetTriggerEntityRefEnd<_TyLexT>() ) * l(u8';');
  _TyFinal CharRefDecData = ++lr(u8'0',u8'9');
  _TyFinal CharRefHexData = ++( lr(u8'0',u8'9') | lr(u8'A',u8'F') | lr(u8'a',u8'f') );
  _TyFinal CharRefDec = ls(u8"&#") * t( TyGetTriggerCharDecRefBegin<_TyLexT>() ) * CharRefDecData * t( TyGetTriggerCharDecRefEnd<_TyLexT>()  ) * l(u8';');
  _TyFinal CharRefHex = ls(u8"&#x") * t( TyGetTriggerCharHexRefBegin<_TyLexT>() ) * CharRefHexData * t( TyGetTriggerCharHexRefEnd<_TyLexT>() ) * l(u8';');
  _TyFinal CharRef = CharRefDec | CharRefHex; // [66]
  _TyFinal Reference = EntityRef | CharRef;	// [67]
  
  // A run of tabs and of ends of lines within the value is a piece of the value of its own: each of them is a space in
  //  the value of the attribute ( 3.3.3 ) and the characters themselves are what the document has - see s_kdtAttrWhitespace.
  // A state may have but one trigger leaving it so both kinds of run begin with the same trigger.
  _TyFinal _AVCharRangeNoAmperLessDouble =	t(TyGetTriggerCharDataBegin<_TyLexT>()) * ( ++lnot(u8"&<\"\t\r\n") * t(TyGetTriggerCharDataEnd<_TyLexT>()) | ++( l(0x9) | l(0xA) | l(0xD) ) * t(TyGetTriggerAttWhitespaceEnd<_TyLexT>()) );
  _TyFinal _AVCharRangeNoAmperLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyLexT>()) * ( ++lnot(u8"&<\'\t\r\n") * t(TyGetTriggerCharDataSingleQuoteEnd<_TyLexT>()) | ++( l(0x9) | l(0xA) | l(0xD) ) * t(TyGetTriggerAttWhitespaceSingleQuoteEnd<_TyLexT>()) );
  _TyFinal AttCharDataNoDoubleQuoteOutputValidate = ~lnot(u8"&<\"");
  _TyFinal AttCharDataNoSingleQuoteOutputValidate = ~lnot(u8"&<\'");
  // We need only record whether single or double quotes were used as a convenience to any reader/writer system that may want to duplicate the current manner of quoting.
  _TyFinal AttValue =	l(u8'\"') * t( TyGetTriggerAttValueDoubleQuote<_TyLexT>() ) * ~( _AVCharRangeNoAmperLessDouble | Reference )  * l(u8'\"') |	// [10]
                      l(u8'\'') * ~( _AVCharRangeNoAmperLessSingle | Reference ) * l(u8'\''); // No need to record a single quote trigger as the lack of double quote is adequate.
  _TyFinal Attribute = /* NSAttName * Eq * AttValue | */ // [41]
                        QName * Eq * AttValue;

// The various types of tags:
  _TyFinal STag = l(u8'<') * QName * t(TyGetTriggerSaveTagName<_TyLexT>()) * ~( S * Attribute * t(TyGetTriggerSaveAttributes<_TyLexT>()) ) * --S * l(u8'>'); // [12]
  _TyFinal ETag = ls(u8"</") * QName * --S * l(u8'>');// [13]
  _TyFinal EmptyElemTag = l(u8'<') * QName * t(TyGetTriggerSaveTagName<_TyLexT>()) * ~( S * Attribute * t(TyGetTriggerSaveAttributes<_TyLexT>()) ) * --S * ls(u8"/>");// [14]								

// Processsing instruction:
  _TyFinal PITarget = Name - ( ( ( l(u8'x') | l(u8'X') ) * ( l(u8'm') | l(u8'M') ) * ( l(u8'l') | l(u8'L') ) ) ); // This produces an anti-accepting state for "xml" or "XML" (or "xMl", ...).
  _TyFinal PITargetMeat = ~Char;
  _TyFinal PI = ls(u8"<?")	* t( TyGetTriggerPITargetStart<_TyLexT>() ) * PITarget * t( TyGetTriggerPITargetEnd<_TyLexT>() )
      * ( ls(u8"?>") | ( S * t( TyGetTriggerPITargetMeatBegin<_TyLexT>() ) * ( PITargetMeat + ls(u8"?>") ) ) );
  // For output validation we need to ensure that we don't encounter a "?>" in the midst of the PITargetMeat:
  _TyFinal PITargetMeatOutputValidate = ~Char - ( ~Char * ls(u8"?>") * ~Char );

// Comment:
  _TyFinal _CharNoMinus =	l(0x09) | l(0x0a) | l(0x0d) | lr(0x020,0x02c) | lr(0x2e,0xff); // [2].
  // For a comment the end trigger was messing with the production since it kept the second '-'
  _TyFinal CommentChars = ~(_CharNoMinus | (l(u8'-') * _CharNoMinus));
  _TyFinal Comment = ls(u8"<!--") * CommentChars * ls(u8"-->");

// CDataSection:
  // As with Comments we cannot use triggers in this production since the characters in "]]>" are also present in the production Char.
  // There is no transition from character classes where the trigger could possibly fire.
  _TyFinal CDStart = ls(u8"<![CDATA["); //[18]
  _TyFinal CDEnd = ls(u8"]]>"); //[21]
  _TyFinal CDSect = CDStart * ~Char + CDEnd;
  // We need a separate production that finds any contained "]]>" so we can escape it appropriately.
  _TyFinal CDCharsOutputValidate = ~Char - ( ~Char * ls(u8"]]>") * ~Char );

// CharData and/or references:
  // We must have at least one character for the triggers to fire correctly.
  // We can only use an ending position on this CharData production. Triggers at the beginning of a production do not work because they would fire everytime.
  // This requires fixing up after the fact - easy to do - but also specific to the XML parser as we must know how to fix it up correctly and that will be specific to the XML parser.
  _TyFinal CharData = ++lnot(u8"<&") - ( ~lnot(u8"<&") * ls(u8"]]>") * ~lnot(u8"<&") );	
  _TyFinal CharDataWithTrigger = CharData * t(TyGetTriggerCharDataEndpoint<_TyLexT>());	
  _TyFinal CharDataAndReferences = ++( CharDataWithTrigger | Reference );

// prolog stuff:
// XMLDecl and TextDecl(external DTDs):
  // This makes things default to "standalone='no'" when no standalone statement is present.
  // This is the statement of the specification that reflects this:
  // "If there are external markup declarations but there is no standalone document declaration, the value 'no' is assumed."
  // So we assume no and then the value is meaningless if there are no external declarations.
  _TyFinal _YesSDDecl = ls(u8"yes") * t(TyGetTriggerStandaloneYes<_TyLexT>());
  _TyFinal _NoSDDecl = ls(u8"no") * t(TyGetTriggerStandaloneNo<_TyLexT>()); // Need to record a "no" to know when the standalone declaration is not present at all.
  _TyFinal SDDecl = S * ls(u8"standalone") * Eq *		// [32]
    ( ( l(u8'\"') * t(TyGetTriggerStandaloneDoubleQuote<_TyLexT>()) * ( _YesSDDecl | _NoSDDecl ) * l(u8'\"') ) |
      ( l(u8'\'') * ( _YesSDDecl | _NoSDDecl ) * l(u8'\'') ) );
  _TyFinal EncName = ( lr(u8'A',u8'Z') | lr(u8'a',u8'z') ) * // [81]
                      ~(  lr(u8'A',u8'Z') | lr(u8'a',u8'z') | 
                          lr(u8'0',u8'9') | l(u8'.') | l(u8'_') | l(u8'-') );
  _TyFinal _TrEncName =	t(TyGetTriggerEncodingNameBegin<_TyLexT>()) *  EncName * t(TyGetTriggerEncodingNameEnd<_TyLexT>());
  _TyFinal EncodingDecl = S * ls(u8"encoding") * Eq *			// [80].
      ( l(u8'\"') * t( TyGetTriggerEncDeclDoubleQuote<_TyLexT>()) * _TrEncName * l(u8'\"') | 
        l(u8'\'') * t( TyGetTriggerEncDeclSingleQuote<_TyLexT>()) * _TrEncName * l(u8'\'') ); // We need to record a trigger for the single quote to know whether the encoding decl is thaere at all.

  _TyFinal VersionNum = ls(u8"1.") * t(TyGetTriggerVersionNumBegin<_TyLexT>()) * ++lr(u8'0',u8'9') * t(TyGetTriggerVersionNumEnd<_TyLexT>());
  _TyFinal VersionInfo = S * ls(u8"version") * Eq * // [24]
                          (	l(u8'\"') * t(TyGetTriggerVersionNumDoubleQuote<_TyLexT>()) * VersionNum * l(u8'\"') |
                            l(u8'\'') * VersionNum * l(u8'\'') );

  _TyFinal XMLDecl = ls(u8"<?xml") * VersionInfo * --EncodingDecl * --SDDecl * --S * ls(u8"?>"); // [23]
   _TyFinal TextDecl = ls(u8"<?xml") * --VersionInfo * EncodingDecl * --S * ls(u8"?>"); //[77]
  // In order to have both the PI production (processing instruction) and the XMLDecl production in the same DFA you have to match them up initially - including the triggers - the same triggers.
  // However I am just going to move XMLDecl and TextDecl to their own DFAs and only use them at the start of a file. This allows better validation at the same time.
  //_TyFinal	XMLDecl = ls(u8"<?") * t( TyGetTriggerPITargetStart<_TyLexT>() ) * ls(u8"xml") * t( TyGetTriggerPITargetEnd<_TyLexT>() ) * VersionInfo * --EncodingDecl * --SDDecl * --S * ls(u8"?>"); // [23]
 //	_TyFinal	TextDecl = ls(u8"<?") * t( TyGetTriggerPITargetStart<_TyLexT>() ) * ls(u8"xml") * t( TyGetTriggerPITargetEnd<_TyLexT>() ) *  --VersionInfo * EncodingDecl * --S * ls(u8"?>"); //[77]

// The DTD: [28] through [83].
// The document type declaration and the internal and external subsets are parsed by a parser generated by pgen from the
//  productions of the specification - see xmlpgen_dtd.cpp. So here there are only the tokens of those productions: small
//  ones - a name, a keyword, a bracket - where a tag is one token. White space is a token since the grammar says where it is
//  required and where it may not be.
// Tokens which are the same characters - a keyword is a Name and an Nmtoken, the four kinds of quoted literal - are in
//  different DFAs: the parser chooses the DFA(s) for each token from where it is in the grammar.
  _TyFinal DtdS = ++( l(0x20) | l(0x09) | l(0x0d) | l(0x0a) ); // [3]
  _TyFinal DtdName = NameStartChar * ~NameChar; // [5]
  _TyFinal DtdNmtoken = ++NameChar; // [7]
  _TyFinal PEReference = l(u8'%') * Name * l(u8';'); // [69] No triggers: a '%' alone is a token of the same DFA.
// The literals of the productions:
  _TyFinal DtdDoctypeBegin = ls(u8"<!DOCTYPE"); // [28]
  _TyFinal DtdElementBegin = ls(u8"<!ELEMENT"); // [45]
  _TyFinal DtdAttlistBegin = ls(u8"<!ATTLIST"); // [52]
  _TyFinal DtdEntityBegin = ls(u8"<!ENTITY"); // [71], [72]
  _TyFinal DtdNotationBegin = ls(u8"<!NOTATION"); // [82]
  _TyFinal DtdCondSectBegin = ls(u8"<!["); // [62], [63]
  _TyFinal DtdCondSectEnd = ls(u8"]]>");
  _TyFinal DtdLBracket = l(u8'[');
  _TyFinal DtdRBracket = l(u8']');
  _TyFinal DtdGreater = l(u8'>');
  _TyFinal DtdLParen = l(u8'(');
  _TyFinal DtdRParen = l(u8')');
  _TyFinal DtdBar = l(u8'|');
  _TyFinal DtdComma = l(u8',');
  _TyFinal DtdQuestion = l(u8'?');
  _TyFinal DtdStar = l(u8'*');
  _TyFinal DtdPlus = l(u8'+');
  _TyFinal DtdPercent = l(u8'%'); // [72]
  _TyFinal DtdPCDATA = ls(u8"#PCDATA"); // [51]
  _TyFinal DtdREQUIRED = ls(u8"#REQUIRED"); // [60]
  _TyFinal DtdIMPLIED = ls(u8"#IMPLIED");
  _TyFinal DtdFIXED = ls(u8"#FIXED");
// The keywords: each of these is also a Name and an Nmtoken - they are in a DFA of their own.
  _TyFinal DtdEMPTY = ls(u8"EMPTY"); // [46]
  _TyFinal DtdANY = ls(u8"ANY");
  _TyFinal DtdCDATA = ls(u8"CDATA"); // [55]
  _TyFinal DtdID = ls(u8"ID"); // [56]
  _TyFinal DtdIDREF = ls(u8"IDREF");
  _TyFinal DtdIDREFS = ls(u8"IDREFS");
  _TyFinal DtdENTITY = ls(u8"ENTITY");
  _TyFinal DtdENTITIES = ls(u8"ENTITIES");
  _TyFinal DtdNMTOKEN = ls(u8"NMTOKEN");
  _TyFinal DtdNMTOKENS = ls(u8"NMTOKENS");
  _TyFinal DtdNOTATION = ls(u8"NOTATION"); // [58]
  _TyFinal DtdSYSTEM = ls(u8"SYSTEM"); // [75]
  _TyFinal DtdPUBLIC = ls(u8"PUBLIC");
  _TyFinal DtdNDATA = ls(u8"NDATA"); // [76]
  _TyFinal DtdINCLUDE = ls(u8"INCLUDE"); // [62]
  _TyFinal DtdIGNORE = ls(u8"IGNORE"); // [63]
// The quoted literals - each in a DFA of its own:
  _TyFinal SystemLiteral =	l(u8'\"') * t(TyGetTriggerSystemLiteralBegin<_TyLexT>()) * ~lnot(u8"\"") * t(TyGetTriggerSystemLiteralDoubleQuoteEnd<_TyLexT>()) * l(u8'\"') | //[11]
                            l(u8'\'') * t(TyGetTriggerSystemLiteralBegin<_TyLexT>()) * ~lnot(u8"\'") * t(TyGetTriggerSystemLiteralSingleQuoteEnd<_TyLexT>()) * l(u8'\'');
  _TyFinal PubidCharLessSingleQuote = l(0x20) | l(0xD) | l(0xA) | lr(u8'a',u8'z') | lr(u8'A',u8'Z') | lr(u8'0',u8'9') | lanyof(u8"-()+,./:=?;!*#@$_%");
  _TyFinal PubidChar = PubidCharLessSingleQuote | l('\''); //[13]
  _TyFinal PubidLiteral = l(u8'\"') * t(TyGetTriggerPubidLiteralBegin<_TyLexT>()) * ~PubidChar * t(TyGetTriggerPubidLiteralDoubleQuoteEnd<_TyLexT>()) * l(u8'\"') | //[12]
                            l(u8'\'') * t(TyGetTriggerPubidLiteralBegin<_TyLexT>()) * ~PubidCharLessSingleQuote * t(TyGetTriggerPubidLiteralSingleQuoteEnd<_TyLexT>()) * l(u8'\'');
  // EntityValue [9] is recorded as AttValue [10] is - they share those triggers but they are never in the same DFA - and the
  //  name of a parameter entity reference within it is recorded with the rest of the value.
  _TyFinal _EVPEReference = l(u8'%') * t(TyGetTriggerPEReferenceValueBegin<_TyLexT>()) * Name * t(TyGetTriggerPEReferenceValueEnd<_TyLexT>()) * l(u8';');
  _TyFinal _EVCharRangeNoPercentLessDouble =	t(TyGetTriggerCharDataBegin<_TyLexT>()) * ++lnot(u8"%&\"") * t(TyGetTriggerCharDataEnd<_TyLexT>());
  _TyFinal _EVCharRangeNoPercentLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyLexT>()) * ++lnot(u8"%&\'") * t(TyGetTriggerCharDataSingleQuoteEnd<_TyLexT>());
  _TyFinal EntityValue =	l(u8'\"') * t( TyGetTriggerAttValueDoubleQuote<_TyLexT>() ) * ~( _EVCharRangeNoPercentLessDouble | _EVPEReference | Reference )  * l(u8'\"') |	// [9]
                          l(u8'\'') * ~( _EVCharRangeNoPercentLessSingle | _EVPEReference | Reference ) * l(u8'\''); // No need to record a single quote trigger as the lack of double quote is adequate.
  // The AttValue [10] of the default of an attribute [60] is AttValue above.
// The contents of an ignored conditional section, [65]: Ignore ::= Char* - (Char* ('<![' | ']]>') Char*).
// Sections nest [64], so the '<![' and the ']]>' within the contents are tokens for the parser to count. What is between them
//  is this token: a run of characters without a '<' or a ']', or a single '<' or ']' - which is the token only when it
//  doesn't begin a '<![' or a ']]>' since those are longer. So Ignore is one or more of these tokens.
  _TyFinal DtdIgnore = ++lnot(u8"<]") | l(u8'<') | l(u8']');

#ifndef REGEXP_NO_TRIGGERS
  // _TyFinal Test = ++l('a') * t(TyGetTriggerStandaloneYes<_TyLexT>()) * ++l('b');
  // static const vtyActionIdent s_knTokenTest = 2000;
  // typedef _l_action_token< _l_action_save_data_single< s_knTokenTest, true, TyGetTriggerStandaloneYes > > TyGetTokenTest;
  // Test.SetAction( TyGetTokenTest<_TyLexT>() );
  XMLDecl.SetAction( TyGetTokenXMLDecl<_TyLexT>() );
  STag.SetAction( TyGetTokenSTag<_TyLexT>() );
  ETag.SetAction( TyGetTokenETag<_TyLexT>() );
  EmptyElemTag.SetAction( TyGetTokenEmptyElemTag<_TyLexT>() );
  Comment.SetAction( TyGetTokenComment<_TyLexT>() );
  CDSect.SetAction( TyGetTokenCDataSection<_TyLexT>() );
  CharDataAndReferences.SetAction( TyGetTokenCharData<_TyLexT>() );
  PI.SetAction( TyGetTokenProcessingInstruction<_TyLexT>() );
  // The tokens of the DTD:
  DtdS.SetAction( TyGetTokenDtdS<_TyLexT>() );
  DtdName.SetAction( TyGetTokenDtdName<_TyLexT>() );
  DtdNmtoken.SetAction( TyGetTokenDtdNmtoken<_TyLexT>() );
  PEReference.SetAction( TyGetTokenDtdPEReference<_TyLexT>() );
  DtdDoctypeBegin.SetAction( TyGetTokenDtdDoctypeBegin<_TyLexT>() );
  DtdElementBegin.SetAction( TyGetTokenDtdElementBegin<_TyLexT>() );
  DtdAttlistBegin.SetAction( TyGetTokenDtdAttlistBegin<_TyLexT>() );
  DtdEntityBegin.SetAction( TyGetTokenDtdEntityBegin<_TyLexT>() );
  DtdNotationBegin.SetAction( TyGetTokenDtdNotationBegin<_TyLexT>() );
  DtdLBracket.SetAction( TyGetTokenDtdLBracket<_TyLexT>() );
  DtdRBracket.SetAction( TyGetTokenDtdRBracket<_TyLexT>() );
  DtdGreater.SetAction( TyGetTokenDtdGreater<_TyLexT>() );
  DtdLParen.SetAction( TyGetTokenDtdLParen<_TyLexT>() );
  DtdRParen.SetAction( TyGetTokenDtdRParen<_TyLexT>() );
  DtdBar.SetAction( TyGetTokenDtdBar<_TyLexT>() );
  DtdComma.SetAction( TyGetTokenDtdComma<_TyLexT>() );
  DtdQuestion.SetAction( TyGetTokenDtdQuestion<_TyLexT>() );
  DtdStar.SetAction( TyGetTokenDtdStar<_TyLexT>() );
  DtdPlus.SetAction( TyGetTokenDtdPlus<_TyLexT>() );
  DtdPercent.SetAction( TyGetTokenDtdPercent<_TyLexT>() );
  DtdPCDATA.SetAction( TyGetTokenDtdPCDATA<_TyLexT>() );
  DtdEMPTY.SetAction( TyGetTokenDtdEMPTY<_TyLexT>() );
  DtdANY.SetAction( TyGetTokenDtdANY<_TyLexT>() );
  DtdCDATA.SetAction( TyGetTokenDtdCDATA<_TyLexT>() );
  DtdID.SetAction( TyGetTokenDtdID<_TyLexT>() );
  DtdIDREF.SetAction( TyGetTokenDtdIDREF<_TyLexT>() );
  DtdIDREFS.SetAction( TyGetTokenDtdIDREFS<_TyLexT>() );
  DtdENTITY.SetAction( TyGetTokenDtdENTITY<_TyLexT>() );
  DtdENTITIES.SetAction( TyGetTokenDtdENTITIES<_TyLexT>() );
  DtdNMTOKEN.SetAction( TyGetTokenDtdNMTOKEN<_TyLexT>() );
  DtdNMTOKENS.SetAction( TyGetTokenDtdNMTOKENS<_TyLexT>() );
  DtdNOTATION.SetAction( TyGetTokenDtdNOTATION<_TyLexT>() );
  DtdREQUIRED.SetAction( TyGetTokenDtdREQUIRED<_TyLexT>() );
  DtdIMPLIED.SetAction( TyGetTokenDtdIMPLIED<_TyLexT>() );
  DtdFIXED.SetAction( TyGetTokenDtdFIXED<_TyLexT>() );
  DtdSYSTEM.SetAction( TyGetTokenDtdSYSTEM<_TyLexT>() );
  DtdPUBLIC.SetAction( TyGetTokenDtdPUBLIC<_TyLexT>() );
  DtdNDATA.SetAction( TyGetTokenDtdNDATA<_TyLexT>() );
  DtdCondSectBegin.SetAction( TyGetTokenDtdCondSectBegin<_TyLexT>() );
  DtdCondSectEnd.SetAction( TyGetTokenDtdCondSectEnd<_TyLexT>() );
  DtdINCLUDE.SetAction( TyGetTokenDtdINCLUDE<_TyLexT>() );
  DtdIGNORE.SetAction( TyGetTokenDtdIGNORE<_TyLexT>() );
  SystemLiteral.SetAction( TyGetTokenDtdSystemLiteral<_TyLexT>() );
  PubidLiteral.SetAction( TyGetTokenDtdPubidLiteral<_TyLexT>() );
  EntityValue.SetAction( TyGetTokenDtdEntityValue<_TyLexT>() );
  AttValue.SetAction( TyGetTokenDtdAttValue<_TyLexT>() );
  TextDecl.SetAction( TyGetTokenDtdTextDecl<_TyLexT>() );
  DtdIgnore.SetAction( TyGetTokenDtdIgnore<_TyLexT>() );
#else //!REGEXP_NO_TRIGGERS
  STag.SetAction( _l_action_token< _TyCTok, 1 >() );
  ETag.SetAction( _l_action_token< _TyCTok, 2 >() );
  EmptyElemTag.SetAction( _l_action_token< _TyCTok, 3 >() );
  Comment.SetAction( _l_action_token< _TyCTok, 4 >() );
  XMLDecl.SetAction( _l_action_token< _TyCTok, 5 >() );
#endif //!REGEXP_NO_TRIGGERS

  _TyFinal AllXmlTokens( STag );
  AllXmlTokens.AddRule( ETag );
  AllXmlTokens.AddRule( EmptyElemTag );
  AllXmlTokens.AddRule( Comment );
  AllXmlTokens.AddRule( PI );
  AllXmlTokens.AddRule( CharDataAndReferences );
  AllXmlTokens.AddRule( CDSect );

  typedef _dfa< _TyCTok, _TyAllocator >	_TyDfa;
  typedef _TyDfa::_TyDfaCtxt _TyDfaCtxt;

  // Separate XMLDecl out into its own DFA since it clashes with processing instruction (PI).
  _TyDfa dfaXMLDecl;
  _TyDfaCtxt dctxtXMLDecl(dfaXMLDecl);
  gen_dfa(XMLDecl, dfaXMLDecl, dctxtXMLDecl);

  _TyDfa dfaAll;
  _TyDfaCtxt	dctxtAll(dfaAll);
  gen_dfa(AllXmlTokens, dfaAll, dctxtAll);

// Write-validation tokens. These are used during writing to:
// 1) Validate written tokens of various types.
// 2) Determine the production of CharRefs withing Attribute Values and CharData and CDataSections.
//    We will automatically substitute CharRefs for disallowed characters in these scenarios. Clearly
//    CDataSections involve a different mechanism than AttrValues and CharData - we will correctly
//    nest CDataSections to allow for the existences of "]]>" strings in the output.
// For these we needn't actually set an action but we do so anyway but they only annotate the accept state
//  with their token ids.
// The output validator won't execute any triggers/actions - it just stores the last token match point.
// We don't conglomerate these because they are used individually.
  S.SetAction( TyGetTokenValidSpaces<_TyLexT>() );
  CommentChars.SetAction( TyGetTokenValidCommentChars<_TyLexT>() );
  NCName.SetAction( TyGetTokenValidNCName<_TyLexT>() );
  CharData.SetAction( TyGetTokenValidCharData<_TyLexT>() );
  CDCharsOutputValidate.SetAction( TyGetTokenValidCDataSection<_TyLexT>() );
  AttCharDataNoSingleQuoteOutputValidate.SetAction( TyGetTokenValidAttCharDataNoSingleQuote<_TyLexT>() );
  AttCharDataNoDoubleQuoteOutputValidate.SetAction( TyGetTokenValidAttCharDataNoDoubleQuote<_TyLexT>() );
  Name.SetAction( TyGetTokenValidName<_TyLexT>() );
  CharRefDecData.SetAction( TyGetTokenValidCharRefDec<_TyLexT>() );
  CharRefHexData.SetAction( TyGetTokenValidCharRefHex<_TyLexT>() );
  EncName.SetAction( TyGetTokenValidEncName<_TyLexT>() );
  PITarget.SetAction( TyGetTokenValidPITarget<_TyLexT>() );
  PITargetMeatOutputValidate.SetAction( TyGetTokenValidPITargetMeat<_TyLexT>() );

  _TyDfa dfaSpaces;
  _TyDfaCtxt dctxtSpaces(dfaSpaces);
  gen_dfa(S, dfaSpaces, dctxtSpaces);
  
  _TyDfa dfaCommentChars;
  _TyDfaCtxt dctxtCommentChars(dfaCommentChars);
  gen_dfa(CommentChars, dfaCommentChars, dctxtCommentChars);

  _TyDfa dfaNCName;
  _TyDfaCtxt dctxtNCName(dfaNCName);
  gen_dfa(NCName, dfaNCName, dctxtNCName);

  _TyDfa dfaCharData;
  _TyDfaCtxt dctxtCharData(dfaCharData);
  gen_dfa(CharData, dfaCharData, dctxtCharData);

  _TyDfa dfaCDCharsOutputValidate;
  _TyDfaCtxt dctxtCDCharsOutputValidate( dfaCDCharsOutputValidate );
  gen_dfa( CDCharsOutputValidate, dfaCDCharsOutputValidate, dctxtCDCharsOutputValidate );

  _TyDfa dfaAttCharDataNoSingleQuote;
  _TyDfaCtxt dctxtAttCharDataNoSingleQuote(dfaAttCharDataNoSingleQuote);
  gen_dfa(AttCharDataNoSingleQuoteOutputValidate, dfaAttCharDataNoSingleQuote, dctxtAttCharDataNoSingleQuote);

  _TyDfa dfaAttCharDataNoDoubleQuote;
  _TyDfaCtxt dctxtAttCharDataNoDoubleQuote(dfaAttCharDataNoDoubleQuote);
  gen_dfa(AttCharDataNoDoubleQuoteOutputValidate, dfaAttCharDataNoDoubleQuote, dctxtAttCharDataNoDoubleQuote);

  _TyDfa dfaName;
  _TyDfaCtxt dctxtName(dfaName);
  gen_dfa(Name, dfaName, dctxtName);

  _TyDfa dfaCharRefDecData;
  _TyDfaCtxt dctxtCharRefDecData(dfaCharRefDecData);
  gen_dfa(CharRefDecData, dfaCharRefDecData, dctxtCharRefDecData);

  _TyDfa dfaCharRefHexData;
  _TyDfaCtxt dctxtCharRefHexData(dfaCharRefHexData);
  gen_dfa(CharRefHexData, dfaCharRefHexData, dctxtCharRefHexData);

  _TyDfa dfaEncName;
  _TyDfaCtxt dctxtEncName(dfaEncName);
  gen_dfa(EncName, dfaEncName, dctxtEncName);

  _TyDfa dfaPITarget;
  _TyDfaCtxt dctxtPITarget(dfaPITarget);
  gen_dfa(PITarget, dfaPITarget, dctxtPITarget);

  _TyDfa dfaPITargetMeatOutputValidate;
  _TyDfaCtxt dctxtPITargetMeatOutputValidate(dfaPITargetMeatOutputValidate);
  gen_dfa(PITargetMeatOutputValidate, dfaPITargetMeatOutputValidate, dctxtPITargetMeatOutputValidate);

  // During output we want to be able to detect valid references in CharData and AttrData.
  EntityRef.SetAction( TyGetTokenEntityRef<_TyLexT>() );
  CharRefDec.SetAction( TyGetTokenCharRefDec<_TyLexT>() );
  CharRefHex.SetAction( TyGetTokenCharRefHex<_TyLexT>() );
  _TyFinal AllReferences( EntityRef );
  AllReferences.AddRule( CharRefDec );
  AllReferences.AddRule( CharRefHex );

  _TyDfa dfaAllReferences;
  _TyDfaCtxt dctxtAllReferences(dfaAllReferences);
  // For this we need to ignore the declared triggers within the above productions - we only want to templatize by char type.
  gen_dfa(AllReferences, dfaAllReferences, dctxtAllReferences, ( 1 << encoIgnoreTriggers ) );

  // The DFAs of the DTD - the modes of the lexical analyzer for the parser of the DTD. The order in which they are added to
  //  the generator below is the order of the modes: when the parser needs more than one for a token it tries them in that order.
  // The main one: everything which is not also something else.
  _TyFinal AllDtdTokens( DtdS );
  AllDtdTokens.AddRule( DtdName );
  AllDtdTokens.AddRule( DtdDoctypeBegin );
  AllDtdTokens.AddRule( DtdElementBegin );
  AllDtdTokens.AddRule( DtdAttlistBegin );
  AllDtdTokens.AddRule( DtdEntityBegin );
  AllDtdTokens.AddRule( DtdNotationBegin );
  AllDtdTokens.AddRule( DtdCondSectBegin );
  AllDtdTokens.AddRule( DtdCondSectEnd );
  AllDtdTokens.AddRule( DtdLBracket );
  AllDtdTokens.AddRule( DtdRBracket );
  AllDtdTokens.AddRule( DtdGreater );
  AllDtdTokens.AddRule( DtdLParen );
  AllDtdTokens.AddRule( DtdRParen );
  AllDtdTokens.AddRule( DtdBar );
  AllDtdTokens.AddRule( DtdComma );
  AllDtdTokens.AddRule( DtdQuestion );
  AllDtdTokens.AddRule( DtdStar );
  AllDtdTokens.AddRule( DtdPlus );
  AllDtdTokens.AddRule( DtdPercent );
  AllDtdTokens.AddRule( DtdPCDATA );
  AllDtdTokens.AddRule( DtdREQUIRED );
  AllDtdTokens.AddRule( DtdIMPLIED );
  AllDtdTokens.AddRule( DtdFIXED );
  AllDtdTokens.AddRule( PEReference );
  AllDtdTokens.AddRule( PI );
  AllDtdTokens.AddRule( Comment );
  _TyDfa dfaDtd;
  _TyDfaCtxt dctxtDtd(dfaDtd);
  gen_dfa(AllDtdTokens, dfaDtd, dctxtDtd);

  _TyFinal AllDtdKeywords( DtdEMPTY );
  AllDtdKeywords.AddRule( DtdANY );
  AllDtdKeywords.AddRule( DtdCDATA );
  AllDtdKeywords.AddRule( DtdID );
  AllDtdKeywords.AddRule( DtdIDREF );
  AllDtdKeywords.AddRule( DtdIDREFS );
  AllDtdKeywords.AddRule( DtdENTITY );
  AllDtdKeywords.AddRule( DtdENTITIES );
  AllDtdKeywords.AddRule( DtdNMTOKEN );
  AllDtdKeywords.AddRule( DtdNMTOKENS );
  AllDtdKeywords.AddRule( DtdNOTATION );
  AllDtdKeywords.AddRule( DtdSYSTEM );
  AllDtdKeywords.AddRule( DtdPUBLIC );
  AllDtdKeywords.AddRule( DtdNDATA );
  AllDtdKeywords.AddRule( DtdINCLUDE );
  AllDtdKeywords.AddRule( DtdIGNORE );
  _TyDfa dfaDtdKeyword;
  _TyDfaCtxt dctxtDtdKeyword(dfaDtdKeyword);
  gen_dfa(AllDtdKeywords, dfaDtdKeyword, dctxtDtdKeyword);

  _TyDfa dfaDtdNmtoken;
  _TyDfaCtxt dctxtDtdNmtoken(dfaDtdNmtoken);
  gen_dfa(DtdNmtoken, dfaDtdNmtoken, dctxtDtdNmtoken);

  _TyDfa dfaDtdEntityValue;
  _TyDfaCtxt dctxtDtdEntityValue(dfaDtdEntityValue);
  gen_dfa(EntityValue, dfaDtdEntityValue, dctxtDtdEntityValue);

  _TyDfa dfaDtdAttValue;
  _TyDfaCtxt dctxtDtdAttValue(dfaDtdAttValue);
  gen_dfa(AttValue, dfaDtdAttValue, dctxtDtdAttValue);

  _TyDfa dfaDtdSystemLiteral;
  _TyDfaCtxt dctxtDtdSystemLiteral(dfaDtdSystemLiteral);
  gen_dfa(SystemLiteral, dfaDtdSystemLiteral, dctxtDtdSystemLiteral);

  _TyDfa dfaDtdPubidLiteral;
  _TyDfaCtxt dctxtDtdPubidLiteral(dfaDtdPubidLiteral);
  gen_dfa(PubidLiteral, dfaDtdPubidLiteral, dctxtDtdPubidLiteral);

  // TextDecl is in its own DFA as XMLDecl is: it looks like a processing instruction.
  _TyDfa dfaDtdTextDecl;
  _TyDfaCtxt dctxtDtdTextDecl(dfaDtdTextDecl);
  gen_dfa(TextDecl, dfaDtdTextDecl, dctxtDtdTextDecl);

  // Within an ignored conditional section: anything at all, and the beginnings and ends of sections.
  _TyFinal AllDtdIgnoreTokens( DtdIgnore );
  AllDtdIgnoreTokens.AddRule( DtdCondSectBegin );
  AllDtdIgnoreTokens.AddRule( DtdCondSectEnd );
  _TyDfa dfaDtdIgnore;
  _TyDfaCtxt dctxtDtdIgnore(dfaDtdIgnore);
  gen_dfa(AllDtdIgnoreTokens, dfaDtdIgnore, dctxtDtdIgnore);

  _l_generator< _TyDfa, char >
    gen( _egfdFamilyDisp, "_xmlplex_utf8.h",
      "XMLPLEX", true, "ns_xmlplex",
      "stateUTF8", "char8_t", "u8'", "'");
  gen.add_dfa( dfaAll, dctxtAll, "startUTF8All" );
  gen.add_dfa( dfaXMLDecl, dctxtXMLDecl, "startUTF8XMLDecl" );
  gen.add_dfa( dfaSpaces, dctxtSpaces, "startUTF8Spaces", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaCommentChars, dctxtCommentChars, "startUTF8CommentChars", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaNCName, dctxtNCName, "startUTF8NCName", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaCharData, dctxtCharData, "startUTF8CharData", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaCDCharsOutputValidate, dctxtCDCharsOutputValidate, "startUTF8CDCharsOutputValidate", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaAttCharDataNoSingleQuote, dctxtAttCharDataNoSingleQuote, "startUTF8AttCharDataNoSingleQuote", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaAttCharDataNoDoubleQuote, dctxtAttCharDataNoDoubleQuote, "startUTF8AttCharDataNoDoubleQuote", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaName, dctxtName, "startUTF8Name", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaCharRefDecData, dctxtCharRefDecData, "startUTF8CharRefDecData", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaCharRefHexData, dctxtCharRefHexData, "startUTF8CharRefHexData", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaEncName, dctxtEncName, "startUTF8EncName", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaPITarget, dctxtPITarget, "startUTF8PITarget", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaPITargetMeatOutputValidate, dctxtPITargetMeatOutputValidate, "startUTF8PITargetMeatOutputValidate", ( 1ul << egdoDontTemplatizeStates ) );
  gen.add_dfa( dfaAllReferences, dctxtAllReferences, "startUTF8AllReferences", ( 1ul << egdoDontTemplatizeStates ) );
  // The DFAs of the DTD - in the order of the modes of the lexical analyzer which the parser of the DTD has: see xmlpgen_dtd.cpp.
  gen.add_dfa( dfaDtdTextDecl, dctxtDtdTextDecl, "startUTF8DtdTextDecl" );
  gen.add_dfa( dfaDtdKeyword, dctxtDtdKeyword, "startUTF8DtdKeyword" );
  gen.add_dfa( dfaDtdNmtoken, dctxtDtdNmtoken, "startUTF8DtdNmtoken" );
  gen.add_dfa( dfaDtdEntityValue, dctxtDtdEntityValue, "startUTF8DtdEntityValue" );
  gen.add_dfa( dfaDtdAttValue, dctxtDtdAttValue, "startUTF8DtdAttValue" );
  gen.add_dfa( dfaDtdSystemLiteral, dctxtDtdSystemLiteral, "startUTF8DtdSystemLiteral" );
  gen.add_dfa( dfaDtdPubidLiteral, dctxtDtdPubidLiteral, "startUTF8DtdPubidLiteral" );
  gen.add_dfa( dfaDtd, dctxtDtd, "startUTF8Dtd" );
  gen.add_dfa( dfaDtdIgnore, dctxtDtdIgnore, "startUTF8DtdIgnore" );
  gen.generate();

  // The modes of the lexical analyzer for the parser of the DTD: the DTD DFAs in the order in which they were added above.
  // The names are the same for each character type: the parser is the same for each.
  ns_pg::AddLexerDfa( _rmodesDtd, "DtdTextDecl", dctxtDtdTextDecl );
  ns_pg::AddLexerDfa( _rmodesDtd, "DtdKeyword", dctxtDtdKeyword );
  ns_pg::AddLexerDfa( _rmodesDtd, "DtdNmtoken", dctxtDtdNmtoken );
  ns_pg::AddLexerDfa( _rmodesDtd, "DtdEntityValue", dctxtDtdEntityValue );
  ns_pg::AddLexerDfa( _rmodesDtd, "DtdAttValue", dctxtDtdAttValue );
  ns_pg::AddLexerDfa( _rmodesDtd, "DtdSystemLiteral", dctxtDtdSystemLiteral );
  ns_pg::AddLexerDfa( _rmodesDtd, "DtdPubidLiteral", dctxtDtdPubidLiteral );
  ns_pg::AddLexerDfa( _rmodesDtd, "Dtd", dctxtDtd );
  ns_pg::AddLexerDfa( _rmodesDtd, "DtdIgnore", dctxtDtdIgnore );
}