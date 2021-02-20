
// xmlpgen_utf16.cpp
// Generate the UTF16 XML lexicographical analyzer.
// dbien
// 18FEB2021
// Pulling this stuff out so I can put each generator in a different file merely so I can easily diff the generators.

#include "xmlpgen.h"

__REGEXP_USING_NAMESPACE
__XMLP_USING_NAMESPACE

void
GenerateUTF16XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp )
{
	__REGEXP_OP_USING_NAMESPACE

	typedef _l_transport_fixedmem< char16_t > _TyTransport;
	typedef xml_traits< _TyTransport, false, false > _TyXmlTraits;
	typedef typename _TyXmlTraits::_TyLexTraits _TyLexTraits;
	typedef _TyLexTraits _TyLexT; // shorthand for below.

	typedef char16_t _TyCTok;
  typedef _TyDefaultAllocator  _TyAllocator;
	typedef _regexp_final< _TyCTok, _TyAllocator > _TyFinal;
	typedef _regexp_trigger< _TyCTok, _TyAllocator > _TyTrigger;
	typedef _regexp_action< _TyCTok, _TyAllocator > _TyFreeAction;

// Define some simple macros to make the productions more readable.
// We don't include surrogates when defining a UTF32 set of productions as these are not valid UTF32 values.
// However for the UTF16 we necessarily need to allow these surrogates to be present in productions.
#define lnot(x) litnotset<_TyCTok>(x)

// Utility and multiple use non-triggered productions:
	_TyFinal	S = ++( l(0x20) | l(0x09) | l(0x0d) | l(0x0a) ); // [3]
	_TyFinal	Eq = --S * l(u'=') * --S; // [25].
	_TyFinal	Char =	l(0x09) | l(0x0a) | l(0x0d) | lr(0x020,0xfffd); // [2].

	_TyFinal NameStartChar = l(u':') | lr(u'A',u'Z') | l(u'_') | lr(u'a',u'z') | lr(0xC0,0xD6) | lr(0xD8,0xF6) // [4]
														| lr(0xF8,0x2FF) | lr(0x370,0x37D) | lr(0x37F,0x1FFF) | lr(0x200C,0x200D) | lr(0x2070,0x218F) 
														| lr(0x2C00,0x2FEF) | lr(0x3001,0xDFFF) | lr(0xF900,0xFDCF) | lr(0xFDF0,0xFFFD);
	_TyFinal NameChar = NameStartChar | l(u'-') | l(u'.') | lr(u'0',u'9') | l(0xB7) | lr(0x0300,0x036F) | lr(0x203F,0x2040);	// [4a]
	_TyFinal Name = NameStartChar * ~NameChar; // [5]
	_TyFinal Names = Name * ~( S * Name ); // [6]
	_TyFinal Nmtoken = ++NameChar;
	_TyFinal Nmtokens = Nmtoken * ~( S * Nmtoken );

	_TyFinal NameStartCharNoColon = lr(u'A',u'Z') | l(u'_') | lr(u'a',u'z') | lr(0xC0,0xD6) | lr(0xD8,0xF6) // [4]
													| lr(0xF8,0x2FF) | lr(0x370,0x37D) | lr(0x37F,0x1FFF) | lr(0x200C,0x200D) | lr(0x2070,0x218F) 
													| lr(0x2C00,0x2FEF) | lr(0x3001,0xDFFF) | lr(0xF900,0xFDCF) | lr(0xFDF0,0xFFFD);
	_TyFinal NameCharNoColon = NameStartCharNoColon | l(u'-') | l(u'.') | lr(u'0',u'9') | l(0xB7) | lr(0x0300,0x036F) | lr(0x203F,0x2040);	// [4a]
	_TyFinal NCName = NameStartCharNoColon * ~NameCharNoColon;
	_TyFinal	Prefix = NCName;
	_TyFinal	LocalPart = NCName;

// Qualified name:
	_TyFinal	QName = t(TyGetTriggerPrefixBegin<_TyLexT>()) * Prefix * t(TyGetTriggerPrefixEnd<_TyLexT>()) * --( l(u':')	* t( TyGetTriggerLocalPartBegin<_TyLexT>() ) * LocalPart * t( TyGetTriggerLocalPartEnd<_TyLexT>() ) );
#if 0 // We have to rid these because they match the QName production and therefore they cause trigger confusion. We'll have to check in post-processing for xmlns as the prefix.
	_TyFinal	DefaultAttName = ls(u"xmlns") /* * t( TyGetTriggerDefaultAttName<_TyLexT>() ) */;
	_TyFinal	PrefixedAttName = ls(u"xmlns:") * t( TyGetTriggerPrefixedAttNameBegin<_TyLexT>() ) * NCName * t( TyGetTriggerPrefixedAttNameEnd<_TyLexT>() );
	_TyFinal	NSAttName = PrefixedAttName | DefaultAttName;
#endif //0

// Attribute value and character data productions:
	_TyFinal	EntityRef = l(u'&') * t( TyGetTriggerEntityRefBegin<_TyLexT>() ) 	// [49]
																* Name * t( TyGetTriggerEntityRefEnd<_TyLexT>() ) * l(u';');
	_TyFinal	CharRefDecData = ++lr(u'0',u'9');
	_TyFinal	CharRefHexData = ++( lr(u'0',u'9') | lr(u'A',u'F') | lr(u'a',u'f') );
	_TyFinal	CharRef = ( ls(u"&#") * t( TyGetTriggerCharDecRefBegin<_TyLexT>() ) * CharRefDecData * t( TyGetTriggerCharDecRefEnd<_TyLexT>()  ) * l(u';') )	// [66]
							| ( ls(u"&#x") * t( TyGetTriggerCharHexRefBegin<_TyLexT>() ) * CharRefHexData * t( TyGetTriggerCharHexRefEnd<_TyLexT>() ) * l(u';') );
	_TyFinal	Reference = EntityRef | CharRef;	// [67]
	// We use the same triggers for these just to save implementation space and because we know by the final trigger
	//	whether a single or double quote was present.
	_TyFinal	_AVCharRangeNoAmperLessDouble =	t(TyGetTriggerCharDataBegin<_TyLexT>()) * ++lnot(u"&<\"") * t(TyGetTriggerCharDataEnd<_TyLexT>());
	_TyFinal	_AVCharRangeNoAmperLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyLexT>()) * ++lnot(u"&<\'") * t(TyGetTriggerCharDataSingleQuoteEnd<_TyLexT>());
	_TyFinal	AttCharDataNoDoubleQuoteOutputValidate = ~lnot(u"&<\"");
	_TyFinal	AttCharDataNoSingleQuoteOutputValidate = ~lnot(u"&<\'");
	// We need only record whether single or double quotes were used as a convenience to any reader/writer system that may want to duplicate the current manner of quoting.
	_TyFinal	AttValue =	l(u'\"') * /* t( TyGetTriggerAttValueDoubleQuote<_TyLexT>() )  * */ ~( _AVCharRangeNoAmperLessDouble | Reference )  * l(u'\"') |	// [10]
												l(u'\'') * ~( _AVCharRangeNoAmperLessSingle | Reference ) * l(u'\''); // No need to record a single quote trigger as the lack of double quote is adequate.
	_TyFinal	Attribute = /* NSAttName * Eq * AttValue | */ // [41]
												QName * Eq * AttValue;

// The various types of tags:
  _TyFinal	STag = l(u'<') * QName * t(TyGetTriggerSaveTagName<_TyLexT>()) * ~( S * Attribute * t(TyGetTriggerSaveAttributes<_TyLexT>()) ) * --S * l(u'>'); // [12]
	_TyFinal	ETag = ls(u"</") * QName * --S * l(u'>');// [13]
  _TyFinal	EmptyElemTag = l(u'<') * QName * t(TyGetTriggerSaveTagName<_TyLexT>()) * ~( S * Attribute * t(TyGetTriggerSaveAttributes<_TyLexT>()) ) * --S * ls(u"/>");// [14]								

// Processsing instruction:
	_TyFinal PITarget = Name - ( ( ( l(u'x') | l(u'X') ) * ( l(u'm') | l(u'M') ) * ( l(u'l') | l(u'L') ) ) ); // This produces an anti-accepting state for "xml" or "XML".;
	_TyFinal	PITargetMeat = ~Char;
	_TyFinal	PI = ls(u"<?")	* t( TyGetTriggerPITargetStart<_TyLexT>() ) * PITarget * t( TyGetTriggerPITargetEnd<_TyLexT>() )
			* ( ls(u"?>") | ( S * t( TyGetTriggerPITargetMeatBegin<_TyLexT>() ) * ( PITargetMeat + ls(u"?>") ) ) );
	// For output validation we need to ensure that we don't encounter a "?>" in the midst of the PITargetMeat:
	_TyFinal	PITargetMeatOutputValidate = ~Char - ( ~Char * ls(u"?>") * ~Char );

// Comment:
	_TyFinal	_CharNoMinus =	l(0x09) | l(0x0a) | l(0x0d) | lr(0x020,0x02c) | lr(0x02e,0xfffd); // [2].

	// For a comment the end trigger was messing with the production since it kept the second '-'
	_TyFinal CommentChars = ~(_CharNoMinus | (l(u'-') * _CharNoMinus));
	_TyFinal Comment = ls(u"<!--") * CommentChars * ls(u"-->");

// CDataSection:
	// As with Comments we cannot use triggers in this production since the characters in "]]>" are also present in the production Char.
	// There is no transition from character classes where the trigger could possibly fire.
	_TyFinal CDStart = ls(u"<![CDATA["); //[18]
	_TyFinal CDEnd = ls(u"]]>"); //[21]
	_TyFinal CDSect = CDStart * ~Char + CDEnd; 
  // We need a separate production that finds any contained "]]>" so we can escape it appropriately.
  _TyFinal CDCharsOutputValidate = ~Char - ( ~Char * ls(u"]]>") * ~Char );

// CharData and/or references:
	// We must have at least one character for the triggers to fire correctly.
	// We can only use an ending position on this CharData production. Triggers at the beginning of a production do not work because they would fire everytime.
	// This requires fixing up after the fact - easy to do - but also specific to the XML parser as we must know how to fix it up correctly and that will be specific to the XML parser.
	_TyFinal CharData = ++lnot(u"<&") - ( ~lnot(u"<&") * ls(u"]]>") * ~lnot(u"<&") );	
	_TyFinal CharDataWithTrigger = CharData * t(TyGetTriggerCharDataEndpoint<_TyLexT>());	
	_TyFinal CharDataAndReferences = ++( CharDataWithTrigger | Reference );

// prolog stuff:
// XMLDecl and TextDecl(external DTDs):
	// This makes things default to "standalone='no'" when no standalone statement is present.
	// This is the statement of the specification that reflects this:
	// "If there are external markup declarations but there is no standalone document declaration, the value 'no' is assumed."
	// So we assume no and then the value is meaningless if there are no external declarations.
	_TyFinal	_YesSDDecl = ls(u"yes") * t(TyGetTriggerStandaloneYes<_TyLexT>());
	_TyFinal	_NoSDDecl = ls(u"no"); // Don't need to record when we get no.
	_TyFinal	SDDecl = S * ls(u"standalone") * Eq *		// [32]
		( ( l(u'\"') * t(TyGetTriggerStandaloneDoubleQuote<_TyLexT>()) * ( _YesSDDecl | _NoSDDecl ) * l(u'\"') ) |
			( l(u'\'') * ( _YesSDDecl | _NoSDDecl ) * l(u'\'') ) );
	_TyFinal	EncName = ( lr(u'A',u'Z') | lr(u'a',u'z') ) * // [81]
											~(	lr(u'A',u'Z') | lr(u'a',u'z') | 
													lr(u'0',u'9') | l(u'.') | l(u'_') | l(u'-') );
	_TyFinal	_TrEncName =	t(TyGetTriggerEncodingNameBegin<_TyLexT>()) *  EncName * t(TyGetTriggerEncodingNameEnd<_TyLexT>());
	_TyFinal	EncodingDecl = S * ls(u"encoding") * Eq *			// [80].
			( l(u'\"') * t( TyGetTriggerEncDeclDoubleQuote<_TyLexT>()) * _TrEncName * l(u'\"') | 
				l(u'\'') * _TrEncName * l(u'\'') );

	_TyFinal	VersionNum = ls(u"1.") * t(TyGetTriggerVersionNumBegin<_TyLexT>()) * ++lr(u'0',u'9') * t(TyGetTriggerVersionNumEnd<_TyLexT>());
	_TyFinal	VersionInfo = S * ls(u"version") * Eq * // [24]
													(	l(u'\"') * t(TyGetTriggerVersionNumDoubleQuote<_TyLexT>()) * VersionNum * l(u'\"') |
														l(u'\'') * VersionNum * l(u'\'') );

	_TyFinal	XMLDecl = ls(u"<?xml") * VersionInfo * --EncodingDecl * --SDDecl * --S * ls(u"?>"); // [23]
 	_TyFinal	TextDecl = ls(u"<?xml") * --VersionInfo * EncodingDecl * --S * ls(u"?>"); //[77]
	// In order to have both the PI production (processing instruction) and the XMLDecl production in the same DFA you have to match them up initially - including the triggers - the same triggers.
	// However I am just going to move XMLDecl and TextDecl to their own DFAs and only use them at the start of a file. This allows better validation at the same time.
	//_TyFinal	XMLDecl = ls(u"<?") * t( TyGetTriggerPITargetStart<_TyLexT>() ) * ls(u"xml") * t( TyGetTriggerPITargetEnd<_TyLexT>() ) * VersionInfo * --EncodingDecl * --SDDecl * --S * ls(u"?>"); // [23]
 //	_TyFinal	TextDecl = ls(u"<?") * t( TyGetTriggerPITargetStart<_TyLexT>() ) * ls(u"xml") * t( TyGetTriggerPITargetEnd<_TyLexT>() ) *  --VersionInfo * EncodingDecl * --S * ls(u"?>"); //[77]

// DTD stuff:
	_TyFinal PEReference = l(u'%') * t(TyGetTriggerPEReferenceBegin<_TyLexT>()) * Name * t(TyGetTriggerPEReferenceEnd<_TyLexT>()) * l(u';');
// Mixed:
	_TyFinal	_MixedBegin = l(u'(') * --S * ls(u"#PCDATA");
	_TyFinal	_TriggeredMixedName = t( TyGetTriggerMixedNameBegin<_TyLexT>() ) * Name  * t( TyGetTriggerMixedNameEnd<_TyLexT>() );
	_TyFinal	Mixed = _MixedBegin * ~( --S * l(u'|') * --S * _TriggeredMixedName )
														* --S * ls(u")*") |
										_MixedBegin * --S * l(u')'); // [51].
// EntityDecl stuff:
  _TyFinal	SystemLiteral =	l(u'\"') * t(TyGetTriggerSystemLiteralBegin<_TyLexT>()) * ~lnot(u"\"") * t(TyGetTriggerSystemLiteralDoubleQuoteEnd<_TyLexT>()) * l(u'\"') | //[11]
														l(u'\'') * t(TyGetTriggerSystemLiteralBegin<_TyLexT>()) * ~lnot(u"\'") * t(TyGetTriggerSystemLiteralSingleQuoteEnd<_TyLexT>()) * l(u'\'');
	_TyFinal	PubidCharLessSingleQuote = l(0x20) | l(0xD) | l(0xA) | lr(u'a',u'z') | lr(u'A',u'Z') | lr(u'0',u'9') | lanyof(u"-()+,./:=?;!*#@$_%");
	_TyFinal	PubidChar = PubidCharLessSingleQuote | l('\''); //[13]
	_TyFinal	PubidLiteral = l(u'\"') * t(TyGetTriggerPubidLiteralBegin<_TyLexT>()) * ~PubidChar * t(TyGetTriggerPubidLiteralDoubleQuoteEnd<_TyLexT>()) * l(u'\"') | //[12]
														l(u'\'') * t(TyGetTriggerPubidLiteralBegin<_TyLexT>()) * ~PubidCharLessSingleQuote * t(TyGetTriggerPubidLiteralSingleQuoteEnd<_TyLexT>()) * l(u'\'');
	_TyFinal	ExternalID = ls(u"SYSTEM") * S * SystemLiteral | ls(u"PUBLIC") * S * PubidLiteral * S * SystemLiteral; //[75]
  _TyFinal	NDataDecl = S * ls(u"NDATA") * S * Name; //[76]

	// Because this production is sharing triggers with AttValue they cannot appear in the same resultant production (token). That's fine because that isn't possible.
	_TyFinal	_EVCharRangeNoPercentLessDouble =	t(TyGetTriggerCharDataBegin<_TyLexT>()) * ++lnot(u"%&\"") * t(TyGetTriggerCharDataEnd<_TyLexT>());
	_TyFinal	_EVCharRangeNoPercentLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyLexT>()) * ++lnot(u"%&\'") * t(TyGetTriggerCharDataSingleQuoteEnd<_TyLexT>());
	_TyFinal	EntityValue =	l(u'\"') * ~( _EVCharRangeNoPercentLessDouble | PEReference | Reference )  * l(u'\"') |	// [9]
													l(u'\'') * ~( _EVCharRangeNoPercentLessSingle | PEReference | Reference ) * l(u'\''); // No need to record a single quote trigger as the lack of double quote is adequate.
 	_TyFinal	EntityDef = EntityValue | ( ExternalID * --NDataDecl ); //[73]
  _TyFinal	PEDef = EntityValue | ExternalID; //[74]
	_TyFinal	GEDecl = ls(u"<!ENTITY") * S * Name * S * EntityDef * --S * l(u'>'); //[71]
	_TyFinal	PEDecl = ls(u"<!ENTITY") * S * l(u'%') * S * Name * S * PEDef * --S * l(u'>'); //[72]
	_TyFinal	EntityDecl = GEDecl | PEDecl; //[70]

// DOCTYPE stuff: For DOCTYPE we will have a couple of different productions - depending on whether the intSubset is present (in between []).
// This is because the various possible markupdecl productions need to be treated as tokens and the parser will need to switch the start symbol
// 	to the set of productions for the intSubset as these represent a entirely distinct (for the most part) production set.
// Note that an external DTD may start with TextDecl production.
	_TyFinal 	doctype_begin = ls(u"<!DOCTYPE") * S * Name * --(S * ExternalID) * --S;
	_TyFinal 	doctypedecl_no_intSubset = doctype_begin * l(u'>');
	_TyFinal 	doctypedecl_intSubset_begin = doctype_begin * l(u'['); // This is returned as a token different from doctypedecl_no_intSubset. This will cause the parser to go into "DTD" mode, recognizing DTD markup, etc.
	_TyFinal 	doctypedecl_intSubset_end = l(u']') * --S * l(u'>');

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
#ifdef XMLPGEN_VALIDATION_ACTIONS // Don't need these.
	CommentChars.SetAction( TyGetTokenValidCommentChars<_TyLexT>() );
	NCName.SetAction( TyGetTokenValidNCName<_TyLexT>() );
	CharData.SetAction( TyGetTokenValidCharData<_TyLexT>() );
	AttCharDataNoSingleQuoteOutputValidate.SetAction( TyGetTokenValidAttCharDataNoSingleQuote<_TyLexT>() );
	AttCharDataNoDoubleQuoteOutputValidate.SetAction( TyGetTokenValidAttCharDataNoDoubleQuote<_TyLexT>() );
	Name.SetAction( TyGetTokenValidName<_TyLexT>() );
	CharRefDecData.SetAction( TyGetTokenValidCharRefDec<_TyLexT>() );
	CharRefHexData.SetAction( TyGetTokenValidCharRefHex<_TyLexT>() );
	EncName.SetAction( TyGetTokenValidEncName<_TyLexT>() );
	PITarget.SetAction( TyGetTokenValidPITarget<_TyLexT>() );
	PITargetMeatOutputValidate.SetAction( TyGetTokenValidPITargetMeat<_TyLexT>() );
#endif //XMLPGEN_VALIDATION_ACTIONS

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

	_TyDfa dfaCharData;
	_TyDfaCtxt dctxtCharData(dfaCharData);
	gen_dfa(CharData, dfaCharData, dctxtCharData);

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
		gen( _egfdFamilyDisp, "_xmlplex_utf16.h",
			"XMLPLEX", true, "ns_xmlplex",
			"stateUTF16", "char16_t", "u'", "'");
	gen.add_dfa( dfaAll, dctxtAll, "startUTF16All" );
	gen.add_dfa( dfaXMLDecl, dctxtXMLDecl, "startUTF16XMLDecl" );
	gen.add_dfa( dfaCommentChars, dctxtCommentChars, "startUTF16CommentChars", ( 1ul << egdoDontTemplatizeStates ) );
	gen.add_dfa( dfaNCName, dctxtNCName, "startUTF16NCName", ( 1ul << egdoDontTemplatizeStates ) );
	gen.add_dfa( dfaCharData, dctxtCharData, "startUTF16CharData", ( 1ul << egdoDontTemplatizeStates ) );
	gen.add_dfa( dfaAttCharDataNoSingleQuote, dctxtAttCharDataNoSingleQuote, "startUTF16AttCharDataNoSingleQuote", ( 1ul << egdoDontTemplatizeStates ) );
	gen.add_dfa( dfaAttCharDataNoDoubleQuote, dctxtAttCharDataNoDoubleQuote, "startUTF16AttCharDataNoDoubleQuote", ( 1ul << egdoDontTemplatizeStates ) );
	gen.add_dfa( dfaName, dctxtName, "startUTF16Name", ( 1ul << egdoDontTemplatizeStates ) );
	gen.add_dfa( dfaCharRefDecData, dctxtCharRefDecData, "startUTF16CharRefDecData", ( 1ul << egdoDontTemplatizeStates ) );
	gen.add_dfa( dfaCharRefHexData, dctxtCharRefHexData, "startUTF16CharRefHexData", ( 1ul << egdoDontTemplatizeStates ) );
	gen.add_dfa( dfaEncName, dctxtEncName, "startUTF16EncName", ( 1ul << egdoDontTemplatizeStates ) );
	gen.add_dfa( dfaPITarget, dctxtPITarget, "startUTF16PITarget", ( 1ul << egdoDontTemplatizeStates ) );
	gen.add_dfa( dfaPITargetMeatOutputValidate, dctxtPITargetMeatOutputValidate, "startUTF16PITargetMeatOutputValidate", ( 1ul << egdoDontTemplatizeStates ) );
	gen.generate();
}