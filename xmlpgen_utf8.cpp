
// xmlpgen_utf8.cpp
// Generate the UTF8 XML lexicographical analyzer.
// dbien
// 18FEB2021
// Pulling this stuff out so I can put each generator in a different file merely so I can easily diff the generators.

#include "xmlpgen.h"

__REGEXP_USING_NAMESPACE
__XMLP_USING_NAMESPACE

void
GenerateUTF8XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp )
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
	_TyFinal	Prefix = NCName;
	_TyFinal	LocalPart = NCName;

// Qualified name:
	_TyFinal	QName = t(TyGetTriggerPrefixBegin<_TyLexT>()) * Prefix * t(TyGetTriggerPrefixEnd<_TyLexT>()) * --( l(u8':')	* t( TyGetTriggerLocalPartBegin<_TyLexT>() ) * LocalPart * t( TyGetTriggerLocalPartEnd<_TyLexT>() ) );
#if 0 // We have to rid these because they match the QName production and therefore they cause trigger confusion. We'll have to check in post-processing for xmlns as the prefix.
	_TyFinal	DefaultAttName = ls(u8"xmlns") /* * t( TyGetTriggerDefaultAttName<_TyLexT>() ) */;
	_TyFinal	PrefixedAttName = ls(u8"xmlns:") * t( TyGetTriggerPrefixedAttNameBegin<_TyLexT>() ) * NCName * t( TyGetTriggerPrefixedAttNameEnd<_TyLexT>() );
	_TyFinal	NSAttName = PrefixedAttName | DefaultAttName;
#endif //0

// Attribute value and character data productions:
	_TyFinal	EntityRef = l(u8'&') * t( TyGetTriggerEntityRefBegin<_TyLexT>() ) 	// [49]
																* Name * t( TyGetTriggerEntityRefEnd<_TyLexT>() ) * l(u8';');
	_TyFinal	CharRefDecData = ++lr(u8'0',u8'9');
	_TyFinal	CharRefHexData = ++( lr(u8'0',u8'9') | lr(u8'A',u8'F') | lr(u8'a',u8'f') );
  _TyFinal	CharRefDec = ls(u8"&#") * t( TyGetTriggerCharDecRefBegin<_TyLexT>() ) * CharRefDecData * t( TyGetTriggerCharDecRefEnd<_TyLexT>()  ) * l(u8';');
  _TyFinal	CharRefHex = ls(u8"&#x") * t( TyGetTriggerCharHexRefBegin<_TyLexT>() ) * CharRefHexData * t( TyGetTriggerCharHexRefEnd<_TyLexT>() ) * l(u8';');
	_TyFinal	CharRef = CharRefDec | CharRefHex; // [66]
	_TyFinal	Reference = EntityRef | CharRef;	// [67]
  
	// We use the same triggers for these just to save implementation space and because we know by the final trigger
	//	whether a single or double quote was present.
	_TyFinal	_AVCharRangeNoAmperLessDouble =	t(TyGetTriggerCharDataBegin<_TyLexT>()) * ++lnot(u8"&<\"") * t(TyGetTriggerCharDataEnd<_TyLexT>());
	_TyFinal	_AVCharRangeNoAmperLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyLexT>()) * ++lnot(u8"&<\'") * t(TyGetTriggerCharDataSingleQuoteEnd<_TyLexT>());
	_TyFinal	AttCharDataNoDoubleQuoteOutputValidate = ~lnot(u8"&<\"");
	_TyFinal	AttCharDataNoSingleQuoteOutputValidate = ~lnot(u8"&<\'");
	// We need only record whether single or double quotes were used as a convenience to any reader/writer system that may want to duplicate the current manner of quoting.
	_TyFinal	AttValue =	l(u8'\"') * t( TyGetTriggerAttValueDoubleQuote<_TyLexT>() ) * ~( _AVCharRangeNoAmperLessDouble | Reference )  * l(u8'\"') |	// [10]
												l(u8'\'') * ~( _AVCharRangeNoAmperLessSingle | Reference ) * l(u8'\''); // No need to record a single quote trigger as the lack of double quote is adequate.
	_TyFinal	Attribute = /* NSAttName * Eq * AttValue | */ // [41]
												QName * Eq * AttValue;

// The various types of tags:
  _TyFinal	STag = l(u8'<') * QName * t(TyGetTriggerSaveTagName<_TyLexT>()) * ~( S * Attribute * t(TyGetTriggerSaveAttributes<_TyLexT>()) ) * --S * l(u8'>'); // [12]
	_TyFinal	ETag = ls(u8"</") * QName * --S * l(u8'>');// [13]
  _TyFinal	EmptyElemTag = l(u8'<') * QName * t(TyGetTriggerSaveTagName<_TyLexT>()) * ~( S * Attribute * t(TyGetTriggerSaveAttributes<_TyLexT>()) ) * --S * ls(u8"/>");// [14]								

// Processsing instruction:
	_TyFinal	PITarget = Name - ( ( ( l(u8'x') | l(u8'X') ) * ( l(u8'm') | l(u8'M') ) * ( l(u8'l') | l(u8'L') ) ) ); // This produces an anti-accepting state for "xml" or "XML" (or "xMl", ...).
	_TyFinal	PITargetMeat = ~Char;
	_TyFinal	PI = ls(u8"<?")	* t( TyGetTriggerPITargetStart<_TyLexT>() ) * PITarget * t( TyGetTriggerPITargetEnd<_TyLexT>() )
			* ( ls(u8"?>") | ( S * t( TyGetTriggerPITargetMeatBegin<_TyLexT>() ) * ( PITargetMeat + ls(u8"?>") ) ) );
	// For output validation we need to ensure that we don't encounter a "?>" in the midst of the PITargetMeat:
	_TyFinal	PITargetMeatOutputValidate = ~Char - ( ~Char * ls(u8"?>") * ~Char );

// Comment:
	_TyFinal	_CharNoMinus =	l(0x09) | l(0x0a) | l(0x0d) | lr(0x020,0x02c) | lr(0x2e,0xff); // [2].
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
	_TyFinal	_YesSDDecl = ls(u8"yes") * t(TyGetTriggerStandaloneYes<_TyLexT>());
	_TyFinal	_NoSDDecl = ls(u8"no"); // Don't need to record when we get no.
	_TyFinal	SDDecl = S * ls(u8"standalone") * Eq *		// [32]
		( ( l(u8'\"') * t(TyGetTriggerStandaloneDoubleQuote<_TyLexT>()) * ( _YesSDDecl | _NoSDDecl ) * l(u8'\"') ) |
			( l(u8'\'') * ( _YesSDDecl | _NoSDDecl ) * l(u8'\'') ) );
	_TyFinal	EncName = ( lr(u8'A',u8'Z') | lr(u8'a',u8'z') ) * // [81]
											~(	lr(u8'A',u8'Z') | lr(u8'a',u8'z') | 
													lr(u8'0',u8'9') | l(u8'.') | l(u8'_') | l(u8'-') );
	_TyFinal	_TrEncName =	t(TyGetTriggerEncodingNameBegin<_TyLexT>()) *  EncName * t(TyGetTriggerEncodingNameEnd<_TyLexT>());
	_TyFinal	EncodingDecl = S * ls(u8"encoding") * Eq *			// [80].
			( l(u8'\"') * t( TyGetTriggerEncDeclDoubleQuote<_TyLexT>()) * _TrEncName * l(u8'\"') | 
				l(u8'\'') * _TrEncName * l(u8'\'') );

	_TyFinal	VersionNum = ls(u8"1.") * t(TyGetTriggerVersionNumBegin<_TyLexT>()) * ++lr(u8'0',u8'9') * t(TyGetTriggerVersionNumEnd<_TyLexT>());
	_TyFinal	VersionInfo = S * ls(u8"version") * Eq * // [24]
													(	l(u8'\"') * t(TyGetTriggerVersionNumDoubleQuote<_TyLexT>()) * VersionNum * l(u8'\"') |
														l(u8'\'') * VersionNum * l(u8'\'') );

	_TyFinal	XMLDecl = ls(u8"<?xml") * VersionInfo * --EncodingDecl * --SDDecl * --S * ls(u8"?>"); // [23]
 	_TyFinal	TextDecl = ls(u8"<?xml") * --VersionInfo * EncodingDecl * --S * ls(u8"?>"); //[77]
	// In order to have both the PI production (processing instruction) and the XMLDecl production in the same DFA you have to match them up initially - including the triggers - the same triggers.
	// However I am just going to move XMLDecl and TextDecl to their own DFAs and only use them at the start of a file. This allows better validation at the same time.
	//_TyFinal	XMLDecl = ls(u8"<?") * t( TyGetTriggerPITargetStart<_TyLexT>() ) * ls(u8"xml") * t( TyGetTriggerPITargetEnd<_TyLexT>() ) * VersionInfo * --EncodingDecl * --SDDecl * --S * ls(u8"?>"); // [23]
 //	_TyFinal	TextDecl = ls(u8"<?") * t( TyGetTriggerPITargetStart<_TyLexT>() ) * ls(u8"xml") * t( TyGetTriggerPITargetEnd<_TyLexT>() ) *  --VersionInfo * EncodingDecl * --S * ls(u8"?>"); //[77]

// DTD stuff:
	_TyFinal PEReference = l(u8'%') * t(TyGetTriggerPEReferenceBegin<_TyLexT>()) * Name * t(TyGetTriggerPEReferenceEnd<_TyLexT>()) * l(u8';');
// Mixed:
	_TyFinal	_MixedBegin = l(u8'(') * --S * ls(u8"#PCDATA");
	_TyFinal	_TriggeredMixedName = t( TyGetTriggerMixedNameBegin<_TyLexT>() ) * Name  * t( TyGetTriggerMixedNameEnd<_TyLexT>() );
	_TyFinal	Mixed = _MixedBegin * ~( --S * l(u8'|') * --S * _TriggeredMixedName )
														* --S * ls(u8")*") |
										_MixedBegin * --S * l(u8')'); // [51].
// EntityDecl stuff:
  _TyFinal	SystemLiteral =	l(u8'\"') * t(TyGetTriggerSystemLiteralBegin<_TyLexT>()) * ~lnot(u8"\"") * t(TyGetTriggerSystemLiteralDoubleQuoteEnd<_TyLexT>()) * l(u8'\"') | //[11]
														l(u8'\'') * t(TyGetTriggerSystemLiteralBegin<_TyLexT>()) * ~lnot(u8"\'") * t(TyGetTriggerSystemLiteralSingleQuoteEnd<_TyLexT>()) * l(u8'\'');
	_TyFinal	PubidCharLessSingleQuote = l(0x20) | l(0xD) | l(0xA) | lr(u8'a',u8'z') | lr(u8'A',u8'Z') | lr(u8'0',u8'9') | lanyof(u8"-()+,./:=?;!*#@$_%");
	_TyFinal	PubidChar = PubidCharLessSingleQuote | l('\''); //[13]
	_TyFinal	PubidLiteral = l(u8'\"') * t(TyGetTriggerPubidLiteralBegin<_TyLexT>()) * ~PubidChar * t(TyGetTriggerPubidLiteralDoubleQuoteEnd<_TyLexT>()) * l(u8'\"') | //[12]
														l(u8'\'') * t(TyGetTriggerPubidLiteralBegin<_TyLexT>()) * ~PubidCharLessSingleQuote * t(TyGetTriggerPubidLiteralSingleQuoteEnd<_TyLexT>()) * l(u8'\'');
	_TyFinal	ExternalID = ls(u8"SYSTEM") * S * SystemLiteral | ls(u8"PUBLIC") * S * PubidLiteral * S * SystemLiteral; //[75]
  _TyFinal	NDataDecl = S * ls(u8"NDATA") * S * Name; //[76]

	// Because this production is sharing triggers with AttValue they cannot appear in the same resultant production (token). That's fine because that isn't possible.
	_TyFinal	_EVCharRangeNoPercentLessDouble =	t(TyGetTriggerCharDataBegin<_TyLexT>()) * ++lnot(u8"%&\"") * t(TyGetTriggerCharDataEnd<_TyLexT>());
	_TyFinal	_EVCharRangeNoPercentLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyLexT>()) * ++lnot(u8"%&\'") * t(TyGetTriggerCharDataSingleQuoteEnd<_TyLexT>());
	_TyFinal	EntityValue =	l(u8'\"') * ~( _EVCharRangeNoPercentLessDouble | PEReference | Reference )  * l(u8'\"') |	// [9]
													l(u8'\'') * ~( _EVCharRangeNoPercentLessSingle | PEReference | Reference ) * l(u8'\''); // No need to record a single quote trigger as the lack of double quote is adequate.
 	_TyFinal	EntityDef = EntityValue | ( ExternalID * --NDataDecl ); //[73]
  _TyFinal	PEDef = EntityValue | ExternalID; //[74]
	_TyFinal	GEDecl = ls(u8"<!ENTITY") * S * Name * S * EntityDef * --S * l(u8'>'); //[71]
	_TyFinal	PEDecl = ls(u8"<!ENTITY") * S * l(u8'%') * S * Name * S * PEDef * --S * l(u8'>'); //[72]
	_TyFinal	EntityDecl = GEDecl | PEDecl; //[70]

// DOCTYPE stuff: For DOCTYPE we will have a couple of different productions - depending on whether the intSubset is present (in between []).
// This is because the various possible markupdecl productions need to be treated as tokens and the parser will need to switch the start symbol
// 	to the set of productions for the intSubset as these represent a entirely distinct (for the most part) production set.
// Note that an external DTD may start with TextDecl production.
	_TyFinal 	doctype_begin = ls(u8"<!DOCTYPE") * S * Name * --(S * ExternalID) * --S;
	_TyFinal 	doctypedecl_no_intSubset = doctype_begin * l(u8'>');
	_TyFinal 	doctypedecl_intSubset_begin = doctype_begin * l(u8'['); // This is returned as a token different from doctypedecl_no_intSubset. This will cause the parser to go into "DTD" mode, recognizing DTD markup, etc.
	_TyFinal 	doctypedecl_intSubset_end = l(u8']') * --S * l(u8'>');

// [70]   	EntityDecl	   ::=   	GEDecl | PEDecl
// [71]   	GEDecl	   ::=   	'<!ENTITY' S Name S EntityDef S? '>'
// [72]   	PEDecl	   ::=   	'<!ENTITY' S '%' S Name S PEDef S? '>'
// [73]   	EntityDef	   ::=   	EntityValue | (ExternalID NDataDecl?)
// [74]   	PEDef	   ::=   	EntityValue | ExternalID	

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
	gen.generate();
}