
// xmlpgen_utf32.cpp
// Generate the UTF32 XML lexicographical analyzer.
// dbien
// 18FEB2021
// Pulling this stuff out so I can put each generator in a different file merely so I can easily diff the generators.

#include "xmlpgen.h"

__REGEXP_USING_NAMESPACE
__XMLP_USING_NAMESPACE

void
GenerateUTF32XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp )
{
	__REGEXP_OP_USING_NAMESPACE

	typedef _l_transport_fixedmem< char32_t > _TyTransport;
	typedef xml_traits< _TyTransport, false, false > _TyXmlTraits;
	typedef typename _TyXmlTraits::_TyLexTraits _TyLexTraits;
	typedef _TyLexTraits _TyLexT; // shorthand for below.

	typedef char32_t _TyCTok;
  typedef _TyDefaultAllocator  _TyAllocator;
	typedef _regexp_final< _TyCTok, _TyAllocator > _TyFinal;
	typedef _regexp_trigger< _TyCTok, _TyAllocator > _TyTrigger;
	typedef _regexp_action< _TyCTok, _TyAllocator > _TyFreeAction;

// Define some simple macros to make the productions more readable.
// We don't include surrogates when defining a UTF32 set of productions as these are not valid UTF32 values.
// However for the UTF16 we necessarily need to allow these surrogates to be present in productions.
#define lnot(x) litnotset_no_surrogates<_TyCTok>(x)

// Utility and multiple use non-triggered productions:
	_TyFinal	S = ++( l(0x20) | l(0x09) | l(0x0d) | l(0x0a) ); // [3]
	_TyFinal	Eq = --S * l(U'=') * --S; // [25].
	_TyFinal	Char =	l(0x09) | l(0x0a) | l(0x0d) | lr(0x020,0xd7ff) | lr(0xe000,0xfffd) | lr(0x10000,0x10ffff); // [2].

	_TyFinal NameStartChar = l(U':') | lr(U'A',U'Z') | l(U'_') | lr(U'a',U'z') | lr(0xC0,0xD6) | lr(0xD8,0xF6) // [4]
														| lr(0xF8,0x2FF) | lr(0x370,0x37D) | lr(0x37F,0x1FFF) | lr(0x200C,0x200D) | lr(0x2070,0x218F) 
														| lr(0x2C00,0x2FEF) | lr(0x3001,0xD7FF) | lr(0xF900,0xFDCF) | lr(0xFDF0,0xFFFD) | lr(0x10000,0xEFFFF);
	_TyFinal NameChar = NameStartChar | l(U'-') | l(U'.') | lr(U'0',U'9') | l(0xB7) | lr(0x0300,0x036F) | lr(0x203F,0x2040);	// [4a]
	_TyFinal Name = NameStartChar * ~NameChar; // [5]
	_TyFinal Names = Name * ~( S * Name ); // [6]
	_TyFinal Nmtoken = ++NameChar;
	_TyFinal Nmtokens = Nmtoken * ~( S * Nmtoken );

	_TyFinal NameStartCharNoColon = lr(U'A',U'Z') | l(U'_') | lr(U'a',U'z') | lr(0xC0,0xD6) | lr(0xD8,0xF6) // [4]
													| lr(0xF8,0x2FF) | lr(0x370,0x37D) | lr(0x37F,0x1FFF) | lr(0x200C,0x200D) | lr(0x2070,0x218F) 
													| lr(0x2C00,0x2FEF) | lr(0x3001,0xD7FF) | lr(0xF900,0xFDCF) | lr(0xFDF0,0xFFFD) | lr(0x10000,0xEFFFF);
	_TyFinal NameCharNoColon = NameStartCharNoColon | l(U'-') | l(U'.') | lr(U'0',U'9') | l(0xB7) | lr(0x0300,0x036F) | lr(0x203F,0x2040);	// [4a]
	_TyFinal NCName = NameStartCharNoColon * ~NameCharNoColon;
	_TyFinal	Prefix = NCName;
	_TyFinal	LocalPart = NCName;

// Qualified name:
	_TyFinal	QName = t(TyGetTriggerPrefixBegin<_TyLexT>()) * Prefix * t(TyGetTriggerPrefixEnd<_TyLexT>()) * --( l(U':')	* t( TyGetTriggerLocalPartBegin<_TyLexT>() ) * LocalPart * t( TyGetTriggerLocalPartEnd<_TyLexT>() ) );
#if 0 // We have to rid these because they match the QName production and therefore they cause trigger confusion. We'll have to check in post-processing for xmlns as the prefix.
	_TyFinal	DefaultAttName = ls(U"xmlns") /* * t( TyGetTriggerDefaultAttName<_TyLexT>() ) */;
	_TyFinal	PrefixedAttName = ls(U"xmlns:") * t( TyGetTriggerPrefixedAttNameBegin<_TyLexT>() ) * NCName * t( TyGetTriggerPrefixedAttNameEnd<_TyLexT>() );
	_TyFinal	NSAttName = PrefixedAttName | DefaultAttName;
#endif //0

// Attribute value and character data productions:
	_TyFinal	EntityRef = l(U'&') * t( TyGetTriggerEntityRefBegin<_TyLexT>() ) 	// [49]
																* Name * t( TyGetTriggerEntityRefEnd<_TyLexT>() ) * l(U';');
	_TyFinal	CharRefDecData = ++lr(U'0',U'9');
	_TyFinal	CharRefHexData = ++( lr(U'0',U'9') | lr(U'A',U'F') | lr(U'a',U'f') );
	_TyFinal	CharRef = ( ls(U"&#") * t( TyGetTriggerCharDecRefBegin<_TyLexT>() ) * CharRefDecData * t( TyGetTriggerCharDecRefEnd<_TyLexT>()  ) * l(U';') )	// [66]
							| ( ls(U"&#x") * t( TyGetTriggerCharHexRefBegin<_TyLexT>() ) * CharRefHexData * t( TyGetTriggerCharHexRefEnd<_TyLexT>() ) * l(U';') );
	_TyFinal	Reference = EntityRef | CharRef;	// [67]
	// We use the same triggers for these just to save implementation space and because we know by the final trigger
	//	whether a single or double quote was present.
	_TyFinal	_AVCharRangeNoAmperLessDouble =	t(TyGetTriggerCharDataBegin<_TyLexT>()) * ++lnot(U"&<\"") * t(TyGetTriggerCharDataEnd<_TyLexT>());
	_TyFinal	_AVCharRangeNoAmperLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyLexT>()) * ++lnot(U"&<\'") * t(TyGetTriggerCharDataSingleQuoteEnd<_TyLexT>());
	_TyFinal	AttCharDataNoDoubleQuoteOutputValidate = ~lnot(U"&<\"");
	_TyFinal	AttCharDataNoSingleQuoteOutputValidate = ~lnot(U"&<\'");
	// We need only record whether single or double quotes were used as a convenience to any reader/writer system that may want to duplicate the current manner of quoting.
	_TyFinal	AttValue =	l(U'\"') * /* t( TyGetTriggerAttValueDoubleQuote<_TyLexT>() )  * */ ~( _AVCharRangeNoAmperLessDouble | Reference )  * l(U'\"') |	// [10]
												l(U'\'') * ~( _AVCharRangeNoAmperLessSingle | Reference ) * l(U'\''); // No need to record a single quote trigger as the lack of double quote is adequate.
	_TyFinal	Attribute = /* NSAttName * Eq * AttValue | */ // [41]
												QName * Eq * AttValue;

// The various types of tags:
  _TyFinal	STag = l(U'<') * QName * t(TyGetTriggerSaveTagName<_TyLexT>()) * ~( S * Attribute * t(TyGetTriggerSaveAttributes<_TyLexT>()) ) * --S * l(U'>'); // [12]
	_TyFinal	ETag = ls(U"</") * QName * --S * l(U'>');// [13]
  _TyFinal	EmptyElemTag = l(U'<') * QName * t(TyGetTriggerSaveTagName<_TyLexT>()) * ~( S * Attribute * t(TyGetTriggerSaveAttributes<_TyLexT>()) ) * --S * ls(U"/>");// [14]								

// Processsing instruction:
	_TyFinal PITarget = Name - ( ( ( l(U'x') | l(U'X') ) * ( l(U'm') | l(U'M') ) * ( l(U'l') | l(U'L') ) ) ); // This produces an anti-accepting state for "xml" or "XML".;
	_TyFinal	PITargetMeat = ~Char;
	_TyFinal	PI = ls(U"<?")	* t( TyGetTriggerPITargetStart<_TyLexT>() ) * PITarget * t( TyGetTriggerPITargetEnd<_TyLexT>() )
			* ( ls(U"?>") | ( S * t( TyGetTriggerPITargetMeatBegin<_TyLexT>() ) * ( PITargetMeat + ls(U"?>") ) ) );
	// For output validation we need to ensure that we don't encounter a "?>" in the midst of the PITargetMeat:
	_TyFinal	PITargetMeatOutputValidate = ~Char - ( ~Char * ls(U"?>") );

// Comment:
	_TyFinal	_CharNoMinus =	l(0x09) | l(0x0a) | l(0x0d) | // [2].
										lr(0x020,0x02c) | lr(0x02e,0xd7ff) | lr(0xe000,0xfffd) | lr(0x10000,0x10ffff);
	// For a comment the end trigger was messing with the production since it kept the second '-'
	_TyFinal CommentChars = ~(_CharNoMinus | (l(U'-') * _CharNoMinus));
	_TyFinal Comment = ls(U"<!--") * CommentChars * ls(U"-->");

// CDataSection:
	// As with Comments we cannot use triggers in this production since the characters in "]]>" are also present in the production Char.
	// There is no transition from character classes where the trigger could possibly fire.
	_TyFinal CDStart = ls(U"<![CDATA["); //[18]
	_TyFinal CDEnd = ls(U"]]>"); //[21]
	_TyFinal CDSect = CDStart * ~Char + CDEnd; 
  // As with CharData output validation we need a separate production that finds any contained "]]>" quickly so we can escape it appropriately.
  _TyFinal CDCharsOutputValidate = ~Char - ( ~Char * ls(U"]]>") );

// CharData and/or references:
	// We must have at least one character for the triggers to fire correctly.
	// We can only use an ending position on this CharData production. Triggers at the beginning of a production do not work because they would fire everytime.
	// This requires fixing up after the fact - easy to do - but also specific to the XML parser as we must know how to fix it up correctly and that will be specific to the XML parser.
	_TyFinal CharData = ++lnot(U"<&") - ( ~lnot(U"<&") * ls(U"]]>") * ~lnot(U"<&") );	
	_TyFinal CharDataWithTrigger = CharData * t(TyGetTriggerCharDataEndpoint<_TyLexT>());	
	_TyFinal CharDataAndReferences = ++( CharDataWithTrigger | Reference );
	// For output validation we want to stop as soon as we find a "]]>" so we can escape it - we'll see if this works:
	_TyFinal CharDataOutputValidate = ++lnot(U"<&") - ( ~lnot(U"<&") * ls(U"]]>") );

// prolog stuff:
// XMLDecl and TextDecl(external DTDs):
	// This makes things default to "standalone='no'" when no standalone statement is present.
	// This is the statement of the specification that reflects this:
	// "If there are external markup declarations but there is no standalone document declaration, the value 'no' is assumed."
	// So we assume no and then the value is meaningless if there are no external declarations.
	_TyFinal	_YesSDDecl = ls(U"yes") * t(TyGetTriggerStandaloneYes<_TyLexT>());
	_TyFinal	_NoSDDecl = ls(U"no"); // Don't need to record when we get no.
	_TyFinal	SDDecl = S * ls(U"standalone") * Eq *		// [32]
		( ( l(U'\"') * t(TyGetTriggerStandaloneDoubleQuote<_TyLexT>()) * ( _YesSDDecl | _NoSDDecl ) * l(U'\"') ) |
			( l(U'\'') * ( _YesSDDecl | _NoSDDecl ) * l(U'\'') ) );
	_TyFinal	EncName = ( lr(U'A',U'Z') | lr(U'a',U'z') ) * // [81]
											~(	lr(U'A',U'Z') | lr(U'a',U'z') | 
													lr(U'0',U'9') | l(U'.') | l(U'_') | l(U'-') );
	_TyFinal	_TrEncName =	t(TyGetTriggerEncodingNameBegin<_TyLexT>()) *  EncName * t(TyGetTriggerEncodingNameEnd<_TyLexT>());
	_TyFinal	EncodingDecl = S * ls(U"encoding") * Eq *			// [80].
			( l(U'\"') * t( TyGetTriggerEncDeclDoubleQuote<_TyLexT>()) * _TrEncName * l(U'\"') | 
				l(U'\'') * _TrEncName * l(U'\'') );

	_TyFinal	VersionNum = ls(U"1.") * t(TyGetTriggerVersionNumBegin<_TyLexT>()) * ++lr(U'0',U'9') * t(TyGetTriggerVersionNumEnd<_TyLexT>());
	_TyFinal	VersionInfo = S * ls(U"version") * Eq * // [24]
													(	l(U'\"') * t(TyGetTriggerVersionNumDoubleQuote<_TyLexT>()) * VersionNum * l(U'\"') |
														l(U'\'') * VersionNum * l(U'\'') );

	_TyFinal	XMLDecl = ls(U"<?xml") * VersionInfo * --EncodingDecl * --SDDecl * --S * ls(U"?>"); // [23]
 	_TyFinal	TextDecl = ls(U"<?xml") * --VersionInfo * EncodingDecl * --S * ls(U"?>"); //[77]
	// In order to have both the PI production (processing instruction) and the XMLDecl production in the same DFA you have to match them up initially - including the triggers - the same triggers.
	// However I am just going to move XMLDecl and TextDecl to their own DFAs and only use them at the start of a file. This allows better validation at the same time.
	//_TyFinal	XMLDecl = ls(U"<?") * t( TyGetTriggerPITargetStart<_TyLexT>() ) * ls(U"xml") * t( TyGetTriggerPITargetEnd<_TyLexT>() ) * VersionInfo * --EncodingDecl * --SDDecl * --S * ls(U"?>"); // [23]
 //	_TyFinal	TextDecl = ls(U"<?") * t( TyGetTriggerPITargetStart<_TyLexT>() ) * ls(U"xml") * t( TyGetTriggerPITargetEnd<_TyLexT>() ) *  --VersionInfo * EncodingDecl * --S * ls(U"?>"); //[77]

// DTD stuff:
	_TyFinal PEReference = l(U'%') * t(TyGetTriggerPEReferenceBegin<_TyLexT>()) * Name * t(TyGetTriggerPEReferenceEnd<_TyLexT>()) * l(U';');
// Mixed:
	_TyFinal	_MixedBegin = l(U'(') * --S * ls(U"#PCDATA");
	_TyFinal	_TriggeredMixedName = t( TyGetTriggerMixedNameBegin<_TyLexT>() ) * Name  * t( TyGetTriggerMixedNameEnd<_TyLexT>() );
	_TyFinal	Mixed = _MixedBegin * ~( --S * l(U'|') * --S * _TriggeredMixedName )
														* --S * ls(U")*") |
										_MixedBegin * --S * l(U')'); // [51].
// EntityDecl stuff:
  _TyFinal	SystemLiteral =	l(U'\"') * t(TyGetTriggerSystemLiteralBegin<_TyLexT>()) * ~lnot(U"\"") * t(TyGetTriggerSystemLiteralDoubleQuoteEnd<_TyLexT>()) * l(U'\"') | //[11]
														l(U'\'') * t(TyGetTriggerSystemLiteralBegin<_TyLexT>()) * ~lnot(U"\'") * t(TyGetTriggerSystemLiteralSingleQuoteEnd<_TyLexT>()) * l(U'\'');
	_TyFinal	PubidCharLessSingleQuote = l(0x20) | l(0xD) | l(0xA) | lr(U'a',U'z') | lr(U'A',U'Z') | lr(U'0',U'9') | lanyof(U"-()+,./:=?;!*#@$_%");
	_TyFinal	PubidChar = PubidCharLessSingleQuote | l('\''); //[13]
	_TyFinal	PubidLiteral = l(U'\"') * t(TyGetTriggerPubidLiteralBegin<_TyLexT>()) * ~PubidChar * t(TyGetTriggerPubidLiteralDoubleQuoteEnd<_TyLexT>()) * l(U'\"') | //[12]
														l(U'\'') * t(TyGetTriggerPubidLiteralBegin<_TyLexT>()) * ~PubidCharLessSingleQuote * t(TyGetTriggerPubidLiteralSingleQuoteEnd<_TyLexT>()) * l(U'\'');
	_TyFinal	ExternalID = ls(U"SYSTEM") * S * SystemLiteral | ls(U"PUBLIC") * S * PubidLiteral * S * SystemLiteral; //[75]
  _TyFinal	NDataDecl = S * ls(U"NDATA") * S * Name; //[76]

	// Because this production is sharing triggers with AttValue they cannot appear in the same resultant production (token). That's fine because that isn't possible.
	_TyFinal	_EVCharRangeNoPercentLessDouble =	t(TyGetTriggerCharDataBegin<_TyLexT>()) * ++lnot(U"%&\"") * t(TyGetTriggerCharDataEnd<_TyLexT>());
	_TyFinal	_EVCharRangeNoPercentLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyLexT>()) * ++lnot(U"%&\'") * t(TyGetTriggerCharDataSingleQuoteEnd<_TyLexT>());
	_TyFinal	EntityValue =	l(U'\"') * ~( _EVCharRangeNoPercentLessDouble | PEReference | Reference )  * l(U'\"') |	// [9]
													l(U'\'') * ~( _EVCharRangeNoPercentLessSingle | PEReference | Reference ) * l(U'\''); // No need to record a single quote trigger as the lack of double quote is adequate.
 	_TyFinal	EntityDef = EntityValue | ( ExternalID * --NDataDecl ); //[73]
  _TyFinal	PEDef = EntityValue | ExternalID; //[74]
	_TyFinal	GEDecl = ls(U"<!ENTITY") * S * Name * S * EntityDef * --S * l(U'>'); //[71]
	_TyFinal	PEDecl = ls(U"<!ENTITY") * S * l(U'%') * S * Name * S * PEDef * --S * l(U'>'); //[72]
	_TyFinal	EntityDecl = GEDecl | PEDecl; //[70]

// DOCTYPE stuff: For DOCTYPE we will have a couple of different productions - depending on whether the intSubset is present (in between []).
// This is because the various possible markupdecl productions need to be treated as tokens and the parser will need to switch the start symbol
// 	to the set of productions for the intSubset as these represent a entirely distinct (for the most part) production set.
// Note that an external DTD may start with TextDecl production.
	_TyFinal 	doctype_begin = ls(U"<!DOCTYPE") * S * Name * --(S * ExternalID) * --S;
	_TyFinal 	doctypedecl_no_intSubset = doctype_begin * l('>');
	_TyFinal 	doctypedecl_intSubset_begin = doctype_begin * l('['); // This is returned as a token different from doctypedecl_no_intSubset. This will cause the parser to go into "DTD" mode, recognizing DTD markup, etc.
	_TyFinal 	doctypedecl_intSubset_end = l(U']') * --S * l(U'>');


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

// Write-validation tokens. These are used during writing to:
// 1) Validate written tokens of various types.
// 2) Determine the production of CharRefs withing Attribute Values and CharData and CDataSections.
//    We will automatically substitute CharRefs for disallowed characters in these scenarios. Clearly
//    CDataSections involve a different mechanism than AttrValues and CharData - we will correctly
//    nest CDataSections to allow for the existences of "]]>" strings in the output.
// For these we needn't actually set an action but we do so anyway.
// The output validator won't execute any triggers/actions - it just stores the last token match point.
// We don't conglomerate these because they are used individually.
	CommentChars.SetAction( TyGetTokenValidCommentChars<_TyLexT>() );
	NCName.SetAction( TyGetTokenValidNCName<_TyLexT>() );
	CharDataOutputValidate.SetAction( TyGetTokenValidCharData<_TyLexT>() );
	AttCharDataNoSingleQuoteOutputValidate.SetAction( TyGetTokenValidAttCharDataNoSingleQuote<_TyLexT>() );
	AttCharDataNoDoubleQuoteOutputValidate.SetAction( TyGetTokenValidAttCharDataNoDoubleQuote<_TyLexT>() );
	Name.SetAction( TyGetTokenValidName<_TyLexT>() );
	CharRefDecData.SetAction( TyGetTokenValidCharRefDec<_TyLexT>() );
	CharRefHexData.SetAction( TyGetTokenValidCharRefHex<_TyLexT>() );
	EncName.SetAction( TyGetTokenValidEncName<_TyLexT>() );
	PITarget.SetAction( TyGetTokenValidPITarget<_TyLexT>() );
	PITargetMeatOutputValidate.SetAction( TyGetTokenValidPITargetMeat<_TyLexT>() );

	typedef _dfa< _TyCTok, _TyAllocator >	_TyDfa;
	typedef _TyDfa::_TyDfaCtxt _TyDfaCtxt;

		// Separate XMLDecl out into its own DFA since it clashes with processing instruction (PI).
	_TyDfa dfaXMLDecl;
	_TyDfaCtxt dctxtXMLDecl(dfaXMLDecl);
	gen_dfa(XMLDecl, dfaXMLDecl, dctxtXMLDecl);

	_TyDfa dfaAll;
	_TyDfaCtxt	dctxtAll(dfaAll);
	gen_dfa(AllXmlTokens, dfaAll, dctxtAll);

	_TyDfa dfaCommentChars;
	_TyDfaCtxt dctxtCommentChars(dfaCommentChars);
	gen_dfa(CommentChars, dfaCommentChars, dctxtCommentChars);

	_TyDfa dfaNCName;
	_TyDfaCtxt dctxtNCName(dfaNCName);
	gen_dfa(NCName, dfaNCName, dctxtNCName);

	_TyDfa dfaCharDataOutputValidate;
	_TyDfaCtxt dctxtCharDataOutputValidate(dfaCharDataOutputValidate);
	gen_dfa(CharDataOutputValidate, dfaCharDataOutputValidate, dctxtCharDataOutputValidate);

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

	_l_generator< _TyDfa, char >
		gen( _egfdFamilyDisp, "_xmlplex_utf32.h",
			"XMLPLEX", true, "ns_xmlplex",
			"stateUTF32", "char32_t", "U'", "'");
	gen.add_dfa(dfaAll, dctxtAll, "startUTF32All");
	gen.add_dfa(dfaXMLDecl, dctxtXMLDecl, "startUTF32XMLDecl");
	gen.add_dfa( dfaCommentChars, dctxtCommentChars, "startUTF32CommentChars" );
	gen.add_dfa( dfaNCName, dctxtNCName, "startUTF32NCName" );
	gen.add_dfa( dfaCharDataOutputValidate, dctxtCharDataOutputValidate, "startUTF32CharDataOutputValidate" );
	gen.add_dfa( dfaAttCharDataNoSingleQuote, dctxtAttCharDataNoSingleQuote, "startUTF32AttCharDataNoSingleQuote" );
	gen.add_dfa( dfaAttCharDataNoDoubleQuote, dctxtAttCharDataNoDoubleQuote, "startUTF32AttCharDataNoDoubleQuote" );
	gen.add_dfa( dfaName, dctxtName, "startUTF32Name" );
	gen.add_dfa( dfaCharRefDecData, dctxtCharRefDecData, "startUTF32CharRefDecData" );
	gen.add_dfa( dfaCharRefHexData, dctxtCharRefHexData, "startUTF32CharRefHexData" );
	gen.add_dfa( dfaEncName, dctxtEncName, "startUTF32EncName" );
	gen.add_dfa( dfaPITarget, dctxtPITarget, "startUTF32PITarget" );
	gen.add_dfa( dfaPITargetMeatOutputValidate, dctxtPITargetMeatOutputValidate, "startUTF32PITargetMeatOutputValidate" );
	gen.generate();
}