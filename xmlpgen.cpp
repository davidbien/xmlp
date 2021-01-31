
// xmlpgen.cpp
// dbien 25DEC2020
// Generate the lexicographical analyzer for the xmlparser implementation using the lexang templates.

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
typedef std::allocator< char >	_TyDefaultAllocator;

#include "_compat.h"

#define _L_REGEXP_DEFAULTCHAR	char32_t
#define __L_DEFAULT_ALLOCATOR _TyDefaultAllocator
#include "_l_inc.h"
#include <fstream>
#include <tuple>

#include "jsonstrm.h"
#include "jsonobjs.h"
#include "_compat.inl"
#include "syslogmgr.inl"
#include "xml_traits.h"

#ifdef __DGRAPH_COUNT_EL_ALLOC_LIFETIME
int gs_iNodesAllocated = 0;
int gs_iLinksAllocated = 0;
int gs_iNodesConstructed = 0;
int gs_iLinksConstructed = 0;
#endif //__DGRAPH_COUNT_EL_ALLOC_LIFETIME

__REGEXP_USING_NAMESPACE
__XMLP_USING_NAMESPACE

template < class t_TyFinal, class t_TyDfa, class t_TyDfaCtxt >
void
gen_dfa( t_TyFinal const & _rFinal, t_TyDfa & _rDfa, t_TyDfaCtxt & _rDfaCtxt );

std::string g_strProgramName;
int TryMain( int argc, char ** argv );

int
main( int argc, char **argv )
{
#ifdef WIN32
	_CrtSetDbgFlag ( _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
#endif //WIN32
	try
	{
		return TryMain( argc, argv );
	}
	catch ( const std::exception & rexc )
	{
		n_SysLog::Log( eslmtError, "%s: *** Exception: [%s]", g_strProgramName.c_str(), rexc.what() );
		fprintf( stderr, "%s: *** Exception: [%s]\n", g_strProgramName.c_str(), rexc.what() );
		return -123;
	}
}

// We want to be able to read each format natively. Each will have a different generated DFA but will be able to share the same _lexical_analyzer in the impl.
void GenerateUTF32XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp );
void GenerateUTF16XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp );
void GenerateUTF8XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp );

int
TryMain( int argc, char ** argv )
{
	g_strProgramName = *argv;
	n_SysLog::InitSysLog( argv[0],  
#ifndef WIN32
		LOG_PERROR, LOG_USER
#else //WIN32
		0, 0
#endif //WIN32
	);

	//GenerateUTF32XmlLex( egfdStandaloneGenerator ); // egfdBaseGenerator
	//GenerateUTF16XmlLex( egfdStandaloneGenerator ); // 
	GenerateUTF8XmlLex( egfdStandaloneGenerator/*egfdSpecializedGenerator*/ );
	return 0;
}

#define l(x)	literal<_TyCTok>(x)
#define ls(x)	litstr<_TyCTok>(x)
#define lr(x,y)	litrange<_TyCTok>(x,y)
#define lanyof(x) litanyinset<_TyCTok>(x)
#define u(n) unsatisfiable<_TyLexT>(n)
#define t(a) _TyTrigger(a)
#define a(a) _TyFreeAction(a)

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
	_TyFinal	CharRef = ( ls(U"&#") * t( TyGetTriggerCharDecRefBegin<_TyLexT>() )  * ++lr(U'0',U'9') * t( TyGetTriggerCharDecRefEnd<_TyLexT>()  ) * l(U';') )	// [66]
							| ( ls(U"&#x") * t( TyGetTriggerCharHexRefBegin<_TyLexT>() ) * ++( lr(U'0',U'9') | lr(U'A',U'F') | lr(U'a',U'f') ) * t( TyGetTriggerCharHexRefEnd<_TyLexT>() ) * l(U';') );
	_TyFinal	Reference = EntityRef | CharRef;	// [67]
	// We use the same triggers for these just to save implementation space and because we know by the final trigger
	//	whether a single or double quote was present.
	_TyFinal	_AVCharRangeNoAmperLessDouble =	t(TyGetTriggerCharDataBegin<_TyLexT>()) * ++lnot(U"&<\"") * t(TyGetTriggerCharDataEnd<_TyLexT>());
	_TyFinal	_AVCharRangeNoAmperLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyLexT>()) * ++lnot(U"&<\'") * t(TyGetTriggerCharDataSingleQuoteEnd<_TyLexT>());
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
	_TyFinal	PI = ls(U"<?")	* t( TyGetTriggerPITargetStart<_TyLexT>() ) * PITarget * t( TyGetTriggerPITargetEnd<_TyLexT>() )
			* ( ls(U"?>") | ( S * t( TyGetTriggerPITargetMeatBegin<_TyLexT>() ) * ( ~Char + ls(U"?>") ) ) );

// Comment:
	_TyFinal	_CharNoMinus =	l(0x09) | l(0x0a) | l(0x0d) | // [2].
										lr(0x020,0x02c) | lr(0x02e,0xd7ff) | lr(0xe000,0xfffd) | lr(0x10000,0x10ffff);
	// For a comment the end trigger was messing with the production since it kept the second '-'
	_TyFinal Comment = ls(U"<!--") /* * t(TyGetTriggerCommentBegin<_TyLexT>())  */* ~(_CharNoMinus | (l(U'-') * _CharNoMinus))/*  * t(TyGetTriggerCommentEnd<_TyLexT>()) */ * ls(U"-->");

// CDataSection:
	// As with Comments we cannot use triggers in this production since the characters in "]]>" are also present in the production Char.
	// There is no transition from character classes where the trigger could possibly fire.
	_TyFinal CDStart = ls(U"<![CDATA["); //[18]
	_TyFinal CDEnd = ls(U"]]>"); //[21]
	_TyFinal CDSect = CDStart * ~Char + CDEnd; 

// CharData and/or references:
	// We must have at least one character for the triggers to fire correctly.
	// We can only use an ending position on this CharData production. Triggers at the beginning of a production do not work because they would fire everytime.
	// This requires fixing up after the fact - easy to do - but also specific to the XML parser as we must know how to fix it up correctly and that will be specific to the XML parser.
	_TyFinal CharData = ( ++lnot(U"<&") - ( ~lnot(U"<&") * ls(U"]]>") * ~lnot(U"<&") ) ) * t(TyGetTriggerCharDataEndpoint<_TyLexT>());	
	_TyFinal CharDataAndReferences = ++( CharData | Reference );

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

	_TyFinal All( STag );
	All.AddRule( ETag );
	All.AddRule( EmptyElemTag );
	All.AddRule( Comment );
	All.AddRule( PI );
	All.AddRule( CharDataAndReferences );
	All.AddRule( CDSect );

	// We will separate out "prolog" processing from the rest of the document.
	// We should be able to have just two DFAs for the XML parsing: prolog and element.
	// We will have to check on content (of course and as we would anyway) to make sure
	//	that things appear in the right order, but that's easy.
	
	typedef _dfa< _TyCTok, _TyAllocator >	_TyDfa;
	typedef _TyDfa::_TyDfaCtxt _TyDfaCtxt;

		// Separate XMLDecl out into its own DFA since it clashes with processing instruction (PI).
	_TyDfa dfaXMLDecl;
	_TyDfaCtxt	dctxtXMLDecl(dfaXMLDecl);
	gen_dfa(XMLDecl, dfaXMLDecl, dctxtXMLDecl);

	_TyDfa dfaAll;
	_TyDfaCtxt	dctxtAll(dfaAll);
	gen_dfa(All, dfaAll, dctxtAll);

	// The UTF32 is, arbitrarily, the "base" generator and the _lexical_analyzer<> class that will be shared among all XML analyzers will be declared in this class.
	_l_generator< _TyDfa, char >
		gen( _egfdFamilyDisp, "_xmlplex_utf32.h",
			"XMLPLEX", true, "ns_xmlplex",
			"stateUTF32", "char32_t", "U'", "'");
	gen.add_dfa(dfaAll, dctxtAll, "startUTF32All");
	gen.add_dfa(dfaXMLDecl, dctxtXMLDecl, "startUTF32XMLDecl");
	gen.generate();
}

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
#undef lnot
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
	_TyFinal	CharRef = ( ls(u"&#") * t( TyGetTriggerCharDecRefBegin<_TyLexT>() )  * ++lr(u'0',u'9') * t( TyGetTriggerCharDecRefEnd<_TyLexT>()  ) * l(u';') )	// [66]
							| ( ls(u"&#x") * t( TyGetTriggerCharHexRefBegin<_TyLexT>() ) * ++( lr(u'0',u'9') | lr(u'A',u'F') | lr(u'a',u'f') ) * t( TyGetTriggerCharHexRefEnd<_TyLexT>() ) * l(u';') );
	_TyFinal	Reference = EntityRef | CharRef;	// [67]
	// We use the same triggers for these just to save implementation space and because we know by the final trigger
	//	whether a single or double quote was present.
	_TyFinal	_AVCharRangeNoAmperLessDouble =	t(TyGetTriggerCharDataBegin<_TyLexT>()) * ++lnot(u"&<\"") * t(TyGetTriggerCharDataEnd<_TyLexT>());
	_TyFinal	_AVCharRangeNoAmperLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyLexT>()) * ++lnot(u"&<\'") * t(TyGetTriggerCharDataSingleQuoteEnd<_TyLexT>());
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
	_TyFinal	PI = ls(u"<?")	* t( TyGetTriggerPITargetStart<_TyLexT>() ) * PITarget * t( TyGetTriggerPITargetEnd<_TyLexT>() )
			* ( ls(u"?>") | ( S * t( TyGetTriggerPITargetMeatBegin<_TyLexT>() ) * ( ~Char + ls(u"?>") ) ) );

// Comment:
	_TyFinal	_CharNoMinus =	l(0x09) | l(0x0a) | l(0x0d) | lr(0x020,0x02c) | lr(0x02e,0xfffd); // [2].
	// For a comment the end trigger was messing with the production since it kept the second '-'
	_TyFinal Comment = ls(u"<!--") /* * t(TyGetTriggerCommentBegin<_TyLexT>())  */* ~(_CharNoMinus | (l(u'-') * _CharNoMinus))/*  * t(TyGetTriggerCommentEnd<_TyLexT>()) */ * ls(u"-->");

// CDataSection:
	// As with Comments we cannot use triggers in this production since the characters in "]]>" are also present in the production Char.
	// There is no transition from character classes where the trigger could possibly fire.
	_TyFinal CDStart = ls(u"<![CDATA["); //[18]
	_TyFinal CDEnd = ls(u"]]>"); //[21]
	_TyFinal CDSect = CDStart * ~Char + CDEnd; 

// CharData and/or references:
	// We must have at least one character for the triggers to fire correctly.
	// We can only use an ending position on this CharData production. Triggers at the beginning of a production do not work because they would fire everytime.
	// This requires fixing up after the fact - easy to do - but also specific to the XML parser as we must know how to fix it up correctly and that will be specific to the XML parser.
	_TyFinal CharData = ( ++lnot(u"<&") - ( ~lnot(u"<&") * ls(u"]]>") * ~lnot(u"<&") ) ) * t(TyGetTriggerCharDataEndpoint<_TyLexT>());	
	_TyFinal CharDataAndReferences = ++( CharData | Reference );

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
	_TyFinal 	doctypedecl_no_intSubset = doctype_begin * l('>');
	_TyFinal 	doctypedecl_intSubset_begin = doctype_begin * l('['); // This is returned as a token different from doctypedecl_no_intSubset. This will cause the parser to go into "DTD" mode, recognizing DTD markup, etc.
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

	_TyFinal All( STag );
	All.AddRule( ETag );
	All.AddRule( EmptyElemTag );
	All.AddRule( Comment );
	All.AddRule( PI );
	All.AddRule( CharDataAndReferences );
	All.AddRule( CDSect );

	// We will separate out "prolog" processing from the rest of the document.
	// We should be able to have just two DFAs for the XML parsing: prolog and element.
	// We will have to check on content (of course and as we would anyway) to make sure
	//	that things appear in the right order, but that's easy.
	
	typedef _dfa< _TyCTok, _TyAllocator >	_TyDfa;
	typedef _TyDfa::_TyDfaCtxt _TyDfaCtxt;

		// Separate XMLDecl out into its own DFA since it clashes with processing instruction (PI).
	_TyDfa dfaXMLDecl;
	_TyDfaCtxt dctxtXMLDecl(dfaXMLDecl);
	gen_dfa(XMLDecl, dfaXMLDecl, dctxtXMLDecl);

	_TyDfa dfaAll;
	_TyDfaCtxt	dctxtAll(dfaAll);
	gen_dfa(All, dfaAll, dctxtAll);

	_l_generator< _TyDfa, char >
		gen( _egfdFamilyDisp, "_xmlplex_utf16.h",
			"XMLPLEX", true, "ns_xmlplex",
			"stateUTF16", "char16_t", "u'", "'");
	gen.add_dfa(dfaAll, dctxtAll, "startUTF16All");
	gen.add_dfa(dfaXMLDecl, dctxtXMLDecl, "startUTF16XMLDecl");
	gen.generate();
}

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
#undef lnot
#define lnot(x) litnotset<_TyCTok>(x)

// Utility and multiple use non-triggered productions:
	_TyFinal	S = ++( l(0x20) | l(0x09) | l(0x0d) | l(0x0a) ); // [3]
	_TyFinal	Eq = --S * l(u8'=') * --S; // [25].
	_TyFinal	Char =	l(0x09) | l(0x0a) | l(0x0d) | lr(0x20,0xff); // [2].

	_TyFinal NameStartChar = l(u8':') | lr(u8'A',u8'Z') | l(u8'_') | lr(u8'a',u8'z') | lr(0xC0,0xFF); // [4]
	_TyFinal NameChar = NameStartChar | l(u8'-') | l(u8'.') | lr(u8'0',u8'9') | lr(0x80,0xFF);	// [4a]
	_TyFinal Name = NameStartChar * ~NameChar; // [5]
	_TyFinal Names = Name * ~( S * Name ); // [6]
	_TyFinal Nmtoken = ++NameChar;
	_TyFinal Nmtokens = Nmtoken * ~( S * Nmtoken );

	_TyFinal NameStartCharNoColon = lr(u8'A',u8'Z') | l(u8'_') | lr(u8'a',u8'z') | lr(0xC0,0xFF); // [4]
	_TyFinal NameCharNoColon = NameStartCharNoColon | l(u8'-') | l(u8'.') | lr(u8'0',u8'9') | lr(0x80,0xFF);	// [4a]
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
	_TyFinal	CharRef = ( ls(u8"&#") * t( TyGetTriggerCharDecRefBegin<_TyLexT>() )  * ++lr(u8'0',u8'9') * t( TyGetTriggerCharDecRefEnd<_TyLexT>()  ) * l(u8';') )	// [66]
							| ( ls(u8"&#x") * t( TyGetTriggerCharHexRefBegin<_TyLexT>() ) * ++( lr(u8'0',u8'9') | lr(u8'A',u8'F') | lr(u8'a',u8'f') ) * t( TyGetTriggerCharHexRefEnd<_TyLexT>() ) * l(u8';') );
	_TyFinal	Reference = EntityRef | CharRef;	// [67]
	// We use the same triggers for these just to save implementation space and because we know by the final trigger
	//	whether a single or double quote was present.
	_TyFinal	_AVCharRangeNoAmperLessDouble =	t(TyGetTriggerCharDataBegin<_TyLexT>()) * ++lnot(u8"&<\"") * t(TyGetTriggerCharDataEnd<_TyLexT>());
	_TyFinal	_AVCharRangeNoAmperLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyLexT>()) * ++lnot(u8"&<\'") * t(TyGetTriggerCharDataSingleQuoteEnd<_TyLexT>());
	// We need only record whether single or double quotes were used as a convenience to any reader/writer system that may want to duplicate the current manner of quoting.
	_TyFinal	AttValue =	l(u8'\"') * /* t( TyGetTriggerAttValueDoubleQuote<_TyLexT>() )  * */ ~( _AVCharRangeNoAmperLessDouble | Reference )  * l(u8'\"') |	// [10]
												l(u8'\'') * ~( _AVCharRangeNoAmperLessSingle | Reference ) * l(u8'\''); // No need to record a single quote trigger as the lack of double quote is adequate.
	_TyFinal	Attribute = /* NSAttName * Eq * AttValue | */ // [41]
												QName * Eq * AttValue;

// The various types of tags:
  _TyFinal	STag = l(u8'<') * QName * t(TyGetTriggerSaveTagName<_TyLexT>()) * ~( S * Attribute * t(TyGetTriggerSaveAttributes<_TyLexT>()) ) * --S * l(u8'>'); // [12]
	_TyFinal	ETag = ls(u8"</") * QName * --S * l(u8'>');// [13]
  _TyFinal	EmptyElemTag = l(u8'<') * QName * t(TyGetTriggerSaveTagName<_TyLexT>()) * ~( S * Attribute * t(TyGetTriggerSaveAttributes<_TyLexT>()) ) * --S * ls(u8"/>");// [14]								

// Processsing instruction:
	_TyFinal PITarget = Name - ( ( ( l(u8'x') | l(u8'X') ) * ( l(u8'm') | l(u8'M') ) * ( l(u8'l') | l(u8'L') ) ) ); // This produces an anti-accepting state for "xml" or "XML".;
	_TyFinal	PI = ls(u8"<?")	* t( TyGetTriggerPITargetStart<_TyLexT>() ) * PITarget * t( TyGetTriggerPITargetEnd<_TyLexT>() )
			* ( ls(u8"?>") | ( S * t( TyGetTriggerPITargetMeatBegin<_TyLexT>() ) * ( ~Char + ls(u8"?>") ) ) );

// Comment:
	_TyFinal	_CharNoMinus =	l(0x09) | l(0x0a) | l(0x0d) | lr(0x020,0x02c) | lr(0x2e,0xff); // [2].
	// For a comment the end trigger was messing with the production since it kept the second '-'
	_TyFinal Comment = ls(u8"<!--") /* * t(TyGetTriggerCommentBegin<_TyLexT>())  */* ~(_CharNoMinus | (l(u8'-') * _CharNoMinus))/*  * t(TyGetTriggerCommentEnd<_TyLexT>()) */ * ls(u8"-->");

// CDataSection:
	// As with Comments we cannot use triggers in this production since the characters in "]]>" are also present in the production Char.
	// There is no transition from character classes where the trigger could possibly fire.
	_TyFinal CDStart = ls(u8"<![CDATA["); //[18]
	_TyFinal CDEnd = ls(u8"]]>"); //[21]
	_TyFinal CDSect = CDStart * ~Char + CDEnd; 

// CharData and/or references:
	// We must have at least one character for the triggers to fire correctly.
	// We can only use an ending position on this CharData production. Triggers at the beginning of a production do not work because they would fire everytime.
	// This requires fixing up after the fact - easy to do - but also specific to the XML parser as we must know how to fix it up correctly and that will be specific to the XML parser.
	_TyFinal CharData = ( ++lnot(u8"<&") - ( ~lnot(u8"<&") * ls(u8"]]>") * ~lnot(u8"<&") ) ) * t(TyGetTriggerCharDataEndpoint<_TyLexT>());	
	_TyFinal CharDataAndReferences = ++( CharData | Reference );

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
	_TyFinal 	doctypedecl_no_intSubset = doctype_begin * l('>');
	_TyFinal 	doctypedecl_intSubset_begin = doctype_begin * l('['); // This is returned as a token different from doctypedecl_no_intSubset. This will cause the parser to go into "DTD" mode, recognizing DTD markup, etc.
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

	_TyFinal All( STag );
		All.AddRule( ETag );
		All.AddRule( EmptyElemTag );
	All.AddRule( Comment );
	//All.AddRule( PI );
	//All.AddRule( CharDataAndReferences );
	//All.AddRule( CDSect );

	// We will separate out "prolog" processing from the rest of the document.
	// We should be able to have just two DFAs for the XML parsing: prolog and element.
	// We will have to check on content (of course and as we would anyway) to make sure
	//	that things appear in the right order, but that's easy.
	
	typedef _dfa< _TyCTok, _TyAllocator >	_TyDfa;
	typedef _TyDfa::_TyDfaCtxt _TyDfaCtxt;

		// Separate XMLDecl out into its own DFA since it clashes with processing instruction (PI).
	_TyDfa dfaXMLDecl;
	_TyDfaCtxt	dctxtXMLDecl(dfaXMLDecl);
	gen_dfa(XMLDecl, dfaXMLDecl, dctxtXMLDecl);

	_TyDfa dfaAll;
	_TyDfaCtxt	dctxtAll(dfaAll);
	gen_dfa(All, dfaAll, dctxtAll);

	_l_generator< _TyDfa, char >
		gen( _egfdFamilyDisp, "_xmlplex_utf8.h",
			"XMLPLEX", true, "ns_xmlplex",
			"stateUTF8", "char8_t", "u8'", "'");
	gen.add_dfa(dfaAll, dctxtAll, "startUTF8All");
	gen.add_dfa(dfaXMLDecl, dctxtXMLDecl, "startUTF8XMLDecl");
	gen.generate();
}

template < class t_TyFinal, class t_TyDfa, class t_TyDfaCtxt >
void
gen_dfa( t_TyFinal const & _rFinal, t_TyDfa & _rDfa, t_TyDfaCtxt & _rDfaCtxt )
{
	bool	fOptimizeDfa = /* false */true;

	typedef t_TyDfa	_TyDfa;
	typedef t_TyDfaCtxt	_TyDfaCtxt;
	typedef _nfa< typename _TyDfa::_TyChar, typename _TyDfa::_TyAllocator >	_TyNfa;
	typedef typename _TyNfa::_TyNfaCtxt _TyNfaCtxt;

	{//B
		_TyNfa	nfa( _rDfa.get_allocator() );
		_TyNfaCtxt	nfaCtxt( nfa );

		_rFinal.ConstructNFA( nfaCtxt );

		{//B
			typedef JsonCharTraits< typename _TyDfa::_TyChar > _tyJsonCharTraits;
			typedef JsonFileOutputStream< _tyJsonCharTraits, char16_t > _tyJsonOutputStream;
			typedef JsonFormatSpec< _tyJsonCharTraits > _tyJsonFormatSpec;
			_tyJsonOutputStream josNfa;
			josNfa.Open( "nfadump.json");
			//josNfa.WriteByteOrderMark();
			_tyJsonFormatSpec jfs;
			JsonValueLifePoly< _tyJsonOutputStream > jvlp( josNfa, ejvtObject, &jfs );
			nfa.ToJSONStream( jvlp, nfaCtxt );
		}//EB

		{//B
			_create_dfa< _TyNfa, _TyDfa >	cdfa( nfa, nfaCtxt, _rDfa, _rDfaCtxt, fOptimizeDfa );
			cdfa.create();
		}//EB
	}//EB

	if ( fOptimizeDfa )
	{
		_optimize_dfa< _TyDfa >	optdfa( _rDfa, _rDfaCtxt );
		if ( optdfa.optimize( ) )
		{
			// New dfa.
		}
		else
		{
			// Old was optmimal.
		}
	}

  // Process unsatifiables:
  _rDfaCtxt.ProcessUnsatisfiableTranstitions();
  _rDfa.CompressTransitions();	
}
	
