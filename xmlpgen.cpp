
// xmlpgen.cpp
// dbien 25DEC2020
// Generate the lexicographical analyzer for the xmlparser implementation using the lexang templates.

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

#ifdef __DGRAPH_COUNT_EL_ALLOC_LIFETIME
int gs_iNodesAllocated = 0;
int gs_iLinksAllocated = 0;
int gs_iNodesConstructed = 0;
int gs_iLinksConstructed = 0;
#endif //__DGRAPH_COUNT_EL_ALLOC_LIFETIME

__REGEXP_USING_NAMESPACE

template < class t_TyFinal, class t_TyDfa, class t_TyDfaCtxt >
void
gen_dfa( t_TyFinal const & _rFinal, t_TyDfa & _rDfa, t_TyDfaCtxt & _rDfaCtxt );

std::string g_strProgramName;
int TryMain( int argc, char ** argv );

int
main( int argc, char **argv )
{
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

__REGEXP_OP_USING_NAMESPACE

	typedef char32_t	_TyCharTokens;
  typedef _TyDefaultAllocator  _TyAllocator;
	typedef _regexp_final< _TyCharTokens, _TyAllocator >		_TyFinal;
	typedef _regexp_trigger< _TyCharTokens, _TyAllocator >	_TyTrigger;
	typedef _regexp_action< _TyCharTokens, _TyAllocator >		_TyFreeAction;

// Define some simple macros to make the productions more readable.
#define l(x)	literal< _TyCharTokens >(x)
#define ls(x)	litstr< _TyCharTokens >(x)
#define lr(x,y)	litrange< _TyCharTokens >(x,y)
#define lnot(x) litnotset_no_surrogates< _TyCharTokens >(x)
#define lanyof(x) litanyinset< _TyCharTokens >(x)
#define u(n) unsatisfiable< _TyCharTokens >(n)
#define t(a) _TyTrigger(a)
#define a(a) _TyFreeAction(a)

	static const vtyActionIdent s_knTriggerPITargetStart = 1;
	static const vtyActionIdent s_knTriggerPITargetEnd = 2;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerPITargetStart > _TyTriggerPITargetStart;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerPITargetEnd, s_knTriggerPITargetStart > _TyTriggerPITargetEnd;
	static const vtyActionIdent s_knTriggerPITargetMeatBegin = 3;
	static const vtyActionIdent s_knTriggerPITargetMeatEnd = 4;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerPITargetMeatBegin > _TyTriggerPITargetMeatBegin;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerPITargetMeatEnd, s_knTriggerPITargetMeatBegin > _TyTriggerPITargetMeatEnd;

	static const vtyActionIdent s_knTriggerMixedNameBegin = 5;
	static const vtyActionIdent s_knTriggerMixedNameEnd = 6;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerMixedNameBegin > _TyTriggerMixedNameBegin;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerMixedNameEnd, s_knTriggerMixedNameBegin > _TyTriggerMixedNameEnd;
	
	// For now we will lump together everything that is a subset of CHARDATA (i.e. attr values in which can be no ' or ", etc)
	//	into the same triggers and resultant token data. This should be perfectly fine during input of XML - also
	//	it saves us implementation space.
	static const vtyActionIdent s_knTriggerCharDataBegin = 7;
	static const vtyActionIdent s_knTriggerCharDataEnd = 8;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerCharDataBegin > _TyTriggerCharDataBegin;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerCharDataEnd, s_knTriggerCharDataBegin > _TyTriggerCharDataEnd;

	// We need to have a different set of triggers for the single-quoted CharData since the actions become ambiguous with each other when the are the same - which makes sense.
	static const vtyActionIdent s_knTriggerCharDataSingleQuoteBegin = 9;
	static const vtyActionIdent s_knTriggerCharDataSingleQuoteEnd = 10;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerCharDataSingleQuoteBegin > _TyTriggerCharDataSingleQuoteBegin;
	typedef _l_trigger_string_typed_range< s_kdtPlainText, _TyTriggerCharDataEnd, s_knTriggerCharDataSingleQuoteEnd, s_knTriggerCharDataSingleQuoteBegin > _TyTriggerCharDataSingleQuoteEnd;

	static const vtyActionIdent s_knTriggerEntityRefBegin = 11;
	static const vtyActionIdent s_knTriggerEntityRefEnd = 12;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerEntityRefBegin > _TyTriggerEntityRefBegin;
	typedef _l_trigger_string_typed_range< s_kdtEntityRef, _TyTriggerCharDataEnd, s_knTriggerEntityRefEnd, s_knTriggerEntityRefBegin > _TyTriggerEntityRefEnd;

	static const vtyActionIdent s_knTriggerCharDecRefBegin = 13;
	static const vtyActionIdent s_knTriggerCharDecRefEnd = 14;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerCharDecRefBegin > _TyTriggerCharDecRefBegin;
	typedef _l_trigger_string_typed_range< s_kdtCharDecRef, _TyTriggerCharDataEnd, s_knTriggerCharDecRefEnd, s_knTriggerCharDecRefBegin > _TyTriggerCharDecRefEnd;

	static const vtyActionIdent s_knTriggerCharHexRefBegin = 15;
	static const vtyActionIdent s_knTriggerCharHexRefEnd = 16;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerCharHexRefBegin > _TyTriggerCharHexRefBegin;
	typedef _l_trigger_string_typed_range< s_kdtCharHexRef, _TyTriggerCharDataEnd, s_knTriggerCharHexRefEnd, s_knTriggerCharHexRefBegin > _TyTriggerCharHexRefEnd;

	// Not necessary because we can look at the trigger id within the token's constituent _l_data_typed_range objects.
	// static const vtyActionIdent s_knTriggerAttValueDoubleQuote = 17;
	// typedef _l_trigger_bool< _TyCharTokens, s_knTriggerAttValueDoubleQuote > _TyTriggerAttValueDoubleQuote;

	static const vtyActionIdent s_knTriggerPrefixBegin = 18;
	static const vtyActionIdent s_knTriggerPrefixEnd = 19;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerPrefixBegin > _TyTriggerPrefixBegin;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerPrefixEnd, s_knTriggerPrefixBegin > _TyTriggerPrefixEnd;
	
	static const vtyActionIdent s_knTriggerLocalPartBegin = 20;
	static const vtyActionIdent s_knTriggerLocalPartEnd = 21;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerLocalPartBegin > _TyTriggerLocalPartBegin;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerLocalPartEnd, s_knTriggerLocalPartBegin > _TyTriggerLocalPartEnd;

#if 0 // This trigger is ambiguous due to the nature of attribute names and xmlns attribute names - they match each other - so this trigger fires all the time erroneously.
	static const vtyActionIdent s_knTriggerDefaultAttName = 20;
	typedef _l_trigger_bool< _TyCharTokens, s_knTriggerDefaultAttName > _TyTriggerDefaultAttName;
#endif //0

#if 0 // These triggers also cause ambiguity since PrefixedAttName matches a QName.
	static const vtyActionIdent s_knTriggerPrefixedAttNameBegin = 21;
	static const vtyActionIdent s_knTriggerPrefixedAttNameEnd = 22;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerPrefixedAttNameBegin > _TyTriggerPrefixedAttNameBegin;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerPrefixedAttNameEnd, s_knTriggerPrefixedAttNameBegin > _TyTriggerPrefixedAttNameEnd;
#endif //0

	static const vtyActionIdent s_knTriggerSaveTagName = 23;
	typedef _l_action_save_data_single< s_knTriggerSaveTagName, true, _TyTriggerPrefixEnd, _TyTriggerLocalPartEnd > _TyTriggerSaveTagName;
	
	static const vtyActionIdent s_knTriggerSaveAttributes = 24;
	typedef _l_action_save_data_multiple< s_knTriggerSaveAttributes, true, 
		_TyTriggerPrefixEnd, _TyTriggerLocalPartEnd, /* _TyTriggerDefaultAttName, _TyTriggerPrefixedAttNameEnd,  */
		/* _TyTriggerAttValueDoubleQuote,  */_TyTriggerCharDataEnd > _TyTriggerSaveAttributes;

#if 0 // The comment production has a problem with triggers due to its nature. It is easy to manage without triggers.
	static const vtyActionIdent s_knTriggerCommentBegin = 25;
	static const vtyActionIdent s_knTriggerCommentEnd = 26;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerCommentBegin > _TyTriggerCommentBegin;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerCommentEnd, s_knTriggerCommentBegin > _TyTriggerCommentEnd;
#endif //0

	static const vtyActionIdent s_knTriggerStandaloneYes = 27;
	typedef _l_trigger_bool< _TyCharTokens, s_knTriggerStandaloneYes > _TyTriggerStandaloneYes;
	static const vtyActionIdent s_knTriggerStandaloneDoubleQuote = 28;
	typedef _l_trigger_bool< _TyCharTokens, s_knTriggerStandaloneDoubleQuote > _TyTriggerStandaloneDoubleQuote;

	static const vtyActionIdent s_knTriggerEncodingNameBegin = 29;
	static const vtyActionIdent s_knTriggerEncodingNameEnd = 30;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerEncodingNameBegin > _TyTriggerEncodingNameBegin;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerEncodingNameEnd, s_knTriggerEncodingNameBegin > _TyTriggerEncodingNameEnd;
	static const vtyActionIdent s_knTriggerEncDeclDoubleQuote = 31;
	typedef _l_trigger_bool< _TyCharTokens, s_knTriggerEncDeclDoubleQuote > _TyTriggerEncDeclDoubleQuote;

	static const vtyActionIdent s_knTriggerVersionNumBegin = 32;
	static const vtyActionIdent s_knTriggerVersionNumEnd = 33;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerVersionNumBegin > _TyTriggerVersionNumBegin;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerVersionNumEnd, s_knTriggerVersionNumBegin > _TyTriggerVersionNumEnd;
	static const vtyActionIdent s_knTriggerVersionNumDoubleQuote = 34;
	typedef _l_trigger_bool< _TyCharTokens, s_knTriggerVersionNumDoubleQuote > _TyTriggerVersionNumDoubleQuote;
	
	static const vtyActionIdent s_knTriggerPEReferenceBegin = 35;
	static const vtyActionIdent s_knTriggerPEReferenceEnd = 36;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerPEReferenceBegin > _TyTriggerPEReferenceBegin;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerPEReferenceEnd, s_knTriggerPEReferenceBegin > _TyTriggerPEReferenceEnd;

	static const vtyActionIdent s_knTriggerSystemLiteralBegin = 37;
	static const vtyActionIdent s_knTriggerSystemLiteralDoubleQuoteEnd = 38;
	static const vtyActionIdent s_knTriggerSystemLiteralSingleQuoteEnd = 39;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerSystemLiteralBegin > _TyTriggerSystemLiteralBegin;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerSystemLiteralDoubleQuoteEnd, s_knTriggerSystemLiteralBegin > _TyTriggerSystemLiteralDoubleQuoteEnd;
	typedef _l_trigger_string_typed_range< s_kdtPlainText, _TyTriggerSystemLiteralDoubleQuoteEnd, s_knTriggerSystemLiteralSingleQuoteEnd, s_knTriggerSystemLiteralBegin > _TyTriggerSystemLiteralSingleQuoteEnd;

	static const vtyActionIdent s_knTriggerPubidLiteralBegin = 40;
	static const vtyActionIdent s_knTriggerPubidLiteralDoubleQuoteEnd = 41;
	static const vtyActionIdent s_knTriggerPubidLiteralSingleQuoteEnd = 42;
	typedef _l_trigger_position< _TyCharTokens, s_knTriggerPubidLiteralBegin > _TyTriggerPubidLiteralBegin;
	typedef _l_trigger_string< _TyCharTokens, s_knTriggerPubidLiteralDoubleQuoteEnd, s_knTriggerPubidLiteralBegin > _TyTriggerPubidLiteralDoubleQuoteEnd;
	typedef _l_trigger_string_typed_range< s_kdtPlainText, _TyTriggerPubidLiteralDoubleQuoteEnd, s_knTriggerPubidLiteralSingleQuoteEnd, s_knTriggerPubidLiteralBegin > _TyTriggerPubidLiteralSingleQuoteEnd;

// Tokens:
	static const vtyActionIdent s_knTokenSTag = 1000;
	typedef _l_action_token< _l_action_save_data_single< s_knTokenSTag, true, _TyTriggerSaveTagName, _TyTriggerSaveAttributes > > _TyTokenSTag;

	static const vtyActionIdent s_knTokenETag = 1001;
	typedef _l_action_token< _l_action_save_data_single< s_knTokenETag, true, _TyTriggerPrefixEnd, _TyTriggerLocalPartEnd > >_TyTokenETag;

	static const vtyActionIdent s_knTokenEmptyElemTag = 1002;
	typedef _l_action_token< _l_action_save_data_single< s_knTokenEmptyElemTag, true, _TyTriggerSaveTagName, _TyTriggerSaveAttributes > > _TyTokenEmptyElemTag;

	static const vtyActionIdent s_knTokenComment = 1003;
	typedef _l_action_token< _l_trigger_noop< _TyCharTokens, s_knTokenComment, true > > _TyTokenComment;

	static const vtyActionIdent s_knTokenXMLDecl = 1004;
	typedef _l_action_token< _l_action_save_data_single< s_knTokenXMLDecl, true, _TyTriggerStandaloneYes, _TyTriggerStandaloneYes, 
		_TyTriggerEncodingNameEnd, _TyTriggerEncDeclDoubleQuote, _TyTriggerVersionNumEnd, _TyTriggerVersionNumDoubleQuote > > _TyTokenXMLDecl;

	static const vtyActionIdent s_knTokenCDataSection = 1004;
	typedef _l_action_token< _l_trigger_noop< _TyCharTokens, s_knTokenCDataSection, true > > _TyTokenCDataSection;

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
	_TyFinal	QName = t(_TyTriggerPrefixBegin()) * Prefix * t(_TyTriggerPrefixEnd()) * --( l(U':')	* t( _TyTriggerLocalPartBegin() ) * LocalPart * t( _TyTriggerLocalPartEnd() ) );
#if 0 // We have to rid these because they match the QName production and therefore they cause trigger confusion. We'll have to check in post-processing for xmlns as the prefix.
	_TyFinal	DefaultAttName = ls(U"xmlns") /* * t( _TyTriggerDefaultAttName() ) */;
	_TyFinal	PrefixedAttName = ls(U"xmlns:") * t( _TyTriggerPrefixedAttNameBegin() ) * NCName * t( _TyTriggerPrefixedAttNameEnd() );
	_TyFinal	NSAttName = PrefixedAttName | DefaultAttName;
#endif //0

// Attribute value and character data productions:
	_TyFinal	EntityRef = l(U'&') * t( _TyTriggerEntityRefBegin() ) 	// [49]
																* Name * t( _TyTriggerEntityRefEnd() ) * l(U';');
	_TyFinal	CharRef = ( ls(U"&#") * t( _TyTriggerCharDecRefBegin() )  * ++lr(U'0',U'9') * t( _TyTriggerCharDecRefEnd()  ) * l(U';') )	// [66]
							| ( ls(U"&#x") * t( _TyTriggerCharHexRefBegin() ) * ++( lr(U'0',U'9') | lr(U'A',U'F') | lr(U'a',U'f') ) * t( _TyTriggerCharHexRefEnd() ) * l(U';') );
	_TyFinal	Reference = EntityRef | CharRef;	// [67]
	// We use the same triggers for these just to save implementation space and because we know by the final trigger
	//	whether a single or double quote was present.
	_TyFinal _NotAmperLessDouble = lr(1,U'!') |  lr(U'#',U'%') | lr(U'\'',U';') | lr(U'=',0xD7FF) | lr(0xE000,0x10ffff); // == lnot("&<\"")
	_TyFinal _NotAmperLessSingle = lr(1,U'%') |  lr(U'(',U';') | lr(U'=',0xD7FF) | lr(0xE000,0x10ffff); // == lnot("&<\'")
	_TyFinal	_AVCharRangeNoAmperLessDouble =	t(_TyTriggerCharDataBegin()) * ++_NotAmperLessDouble * t(_TyTriggerCharDataEnd());
	_TyFinal	_AVCharRangeNoAmperLessSingle =	t(_TyTriggerCharDataSingleQuoteBegin()) * ++_NotAmperLessSingle * t(_TyTriggerCharDataSingleQuoteEnd());
	// We need only record whether single or double quotes were used as a convenience to any reader/writer system that may want to duplicate the current manner of quoting.
	_TyFinal	AttValue =	l(U'\"') * /* t( _TyTriggerAttValueDoubleQuote() )  * */ ~( _AVCharRangeNoAmperLessDouble | Reference )  * l(U'\"') |	// [10]
												l(U'\'') * ~( _AVCharRangeNoAmperLessSingle | Reference ) * l(U'\''); // No need to record a single quote trigger as the lack of double quote is adequate.
	_TyFinal	Attribute = /* NSAttName * Eq * AttValue | */ // [41]
												QName * Eq * AttValue;

// The various types of tags:
  _TyFinal	STag = l(U'<') * QName * t(_TyTriggerSaveTagName()) * ~( S * Attribute * t(_TyTriggerSaveAttributes()) ) * --S * l(U'>'); // [12]
	_TyFinal	ETag = ls(U"</") * QName * --S * l(U'>');// [13]
  _TyFinal	EmptyElemTag = l(U'<') * QName * t(_TyTriggerSaveTagName()) * ~( S * Attribute * t(_TyTriggerSaveAttributes()) ) * --S * ls(U"/>");// [14]								

// Processsing instruction:
	_TyFinal PITarget = Name - ( ( ( l(U'x') | l(U'X') ) * ( l(U'm') | l(U'M') ) * ( l(U'U') | l(U'U') ) ) ); // This produces an anti-accepting state for "xmU" or "XMU".
	_TyFinal	PI = ls(U"<?")	* t( _TyTriggerPITargetStart() ) * PITarget * t( _TyTriggerPITargetEnd() )
			* ( ls(U"?>") | ( S * t( _TyTriggerPITargetMeatBegin() ) * ( ~Char + ls(U"?>") ) ) ) * t( _TyTriggerPITargetMeatEnd() );

// Comment:
	_TyFinal	_CharNoMinus =	l(0x09) | l(0x0a) | l(0x0d) | // [2].
										lr(0x020,0x02c) | lr(0x02e,0xd7ff) | lr(0xe000,0xfffd) | lr(0x10000,0x10ffff);
	// For a comment the end trigger was messing with the production since it kept the second '-'
	_TyFinal Comment = ls(U"<!--") /* * t(_TyTriggerCommentBegin())  */* ~(_CharNoMinus | (l(U'-') * _CharNoMinus))/*  * t(_TyTriggerCommentEnd()) */ * ls(U"-->");

// CDataSection:
	// As with Comments we cannot use triggers in this production since the characters in "]]>" are also present in the production Char.
	// This is not transtion from character classes where the trigger can fire.
	_TyFinal CDStart = ls(U"<![CDATA["); //[18]
	_TyFinal CDEnd = ls(U"]]>"); //[21]
	_TyFinal CDSect = CDStart * ~Char + CDEnd; 

// prolog stuff:
// XMLDecl and TextDecl(external DTDs):
	_TyFinal	_YesSDDecl = ls(U"yes") * t(_TyTriggerStandaloneYes());
	_TyFinal	_NoSDDecl = ls(U"no"); // Don't need to record when we get no.
	_TyFinal	SDDecl = S * ls(U"standalone") * Eq *		// [32]
		( ( l(U'\"') * t(_TyTriggerStandaloneDoubleQuote()) * ( _YesSDDecl | _NoSDDecl ) * l(U'\"') ) |
			( l(U'\'') * ( _YesSDDecl | _NoSDDecl ) * l(U'\'') ) );
	_TyFinal	EncName = ( lr(U'A',U'Z') | lr(U'a',U'z') ) * // [81]
											~(	lr(U'A',U'Z') | lr(U'a',U'z') | 
													lr(U'0',U'9') | l(U'.') | l(U'_') | l(U'-') );
	_TyFinal	_TrEncName =	t(_TyTriggerEncodingNameBegin()) *  EncName * t(_TyTriggerEncodingNameEnd());
	_TyFinal	EncodingDecl = S * ls(U"encoding") * Eq *			// [80].
			( l(U'\"') * t( _TyTriggerEncDeclDoubleQuote()) * _TrEncName * l(U'\"') | 
				l(U'\'') * _TrEncName * l(U'\'') );

	_TyFinal	VersionNum = ls(U"1.") * t(_TyTriggerVersionNumBegin()) * ++lr(U'0',U'9') * t(_TyTriggerVersionNumEnd());
	_TyFinal	VersionInfo = S * ls(U"version") * Eq * // [24]
													(	l(U'\"') * t(_TyTriggerVersionNumDoubleQuote()) * VersionNum * l(U'\"') |
														l(U'\'') * VersionNum * l(U'\'') );
	_TyFinal	XMLDecl = ls(U"<?xmU") * VersionInfo * --EncodingDecl * --SDDecl * --S * ls(U"?>"); // [23]
 	_TyFinal	TextDecl = ls(U"<?xmU") * --VersionInfo * EncodingDecl * --S * ls(U"?>"); //[77]

// DTD stuff:
	_TyFinal PEReference = l(U'%') * t(_TyTriggerPEReferenceBegin()) * Name * t(_TyTriggerPEReferenceEnd()) * l(U';');
// Mixed:
	_TyFinal	_MixedBegin = l(U'(') * --S * ls(U"#PCDATA");
	_TyFinal	_TriggeredMixedName = t( _TyTriggerMixedNameBegin() ) * Name  * t( _TyTriggerMixedNameEnd() );
	_TyFinal	Mixed = _MixedBegin * ~( --S * l(U'|') * --S * _TriggeredMixedName )
														* --S * ls(U")*") |
										_MixedBegin * --S * l(U')'); // [51].
// EntityDecl stuff:
  _TyFinal	SystemLiteral =	l(U'\"') * t(_TyTriggerSystemLiteralBegin()) * ~lnot(U"\"") * t(_TyTriggerSystemLiteralDoubleQuoteEnd()) * l(U'\"') | //[11]
														l(U'\'') * t(_TyTriggerSystemLiteralBegin()) * ~lnot(U"\'") * t(_TyTriggerSystemLiteralSingleQuoteEnd()) * l(U'\'');
	_TyFinal	PubidCharLessSingleQuote = l(0x20) | l(0xD) | l(0xA) | lr(U'a',U'z') | lr(U'A',U'Z') | lr(U'0',U'9') | lanyof(U"-()+,./:=?;!*#@$_%");
	_TyFinal	PubidChar = PubidCharLessSingleQuote | l('\''); //[13]
	_TyFinal	PubidLiteral = l(U'\"') * t(_TyTriggerPubidLiteralBegin()) * ~PubidChar * t(_TyTriggerPubidLiteralDoubleQuoteEnd()) * l(U'\"') | //[12]
														l(U'\'') * t(_TyTriggerPubidLiteralBegin()) * ~PubidCharLessSingleQuote * t(_TyTriggerPubidLiteralSingleQuoteEnd()) * l(U'\'');
	_TyFinal	ExternalID = ls(U"SYSTEM") * S * SystemLiteral | ls(U"PUBLIC") * S * PubidLiteral * S * SystemLiteral; //[75]
  _TyFinal	NDataDecl = S * ls(U"NDATA") * S * Name; //[76]

	// Because this production is sharing triggers with AttValue they cannot appear in the same resultant production (token). That's fine because that isn't possible.
	_TyFinal	_EVCharRangeNoPercentLessDouble =	t(_TyTriggerCharDataBegin()) * ++lnot(U"%&\"") * t(_TyTriggerCharDataEnd());
	_TyFinal	_EVCharRangeNoPercentLessSingle =	t(_TyTriggerCharDataSingleQuoteBegin()) * ++lnot(U"%&\'") * t(_TyTriggerCharDataSingleQuoteEnd());
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
	// _TyFinal Test = ++l('a') * t(_TyTriggerStandaloneYes()) * ++l('b');
	// static const vtyActionIdent s_knTokenTest = 2000;
	// typedef _l_action_token< _l_action_save_data_single< s_knTokenTest, true, _TyTriggerStandaloneYes > > _TyTokenTest;
	// Test.SetAction( _TyTokenTest() );
	STag.SetAction( _TyTokenSTag() );
	ETag.SetAction( _TyTokenETag() );
	EmptyElemTag.SetAction( _TyTokenEmptyElemTag() );
	Comment.SetAction( _TyTokenComment() );
	XMLDecl.SetAction( _TyTokenXMLDecl() );
	CDSect.SetAction( _TyTokenCDataSection() );
#else //!REGEXP_NO_TRIGGERS
	STag.SetAction( _l_action_token< _TyCharTokens, 1 >() );
	ETag.SetAction( _l_action_token< _TyCharTokens, 2 >() );
	EmptyElemTag.SetAction( _l_action_token< _TyCharTokens, 3 >() );
	Comment.SetAction( _l_action_token< _TyCharTokens, 4 >() );
	XMLDecl.SetAction( _l_action_token< _TyCharTokens, 5 >() );
#endif //!REGEXP_NO_TRIGGERS

	// _TyFinal	All( Test );
	_TyFinal	All( STag );
	All.AddRule( ETag );
	All.AddRule( EmptyElemTag );
	All.AddRule( Comment );
	All.AddRule( XMLDecl );
	All.AddRule( CDSect );

	// We will separate out "prolog" processing from the rest of the document.
	// We should be able to have just two DFAs for the XML parsing: prolog and element.
	// We will have to check on content (of course and as we would anyway) to make sure
	//	that things appear in the right order, but that's easy.
	
	typedef _dfa< _TyCharTokens, _TyAllocator >	_TyDfa;
	typedef _TyDfa::_TyDfaCtxt _TyDfaCtxt;

  try
  {
    _TyDfa dfaAll;
    _TyDfaCtxt	dctxtAll(dfaAll);
    gen_dfa(All, dfaAll, dctxtAll);

    _l_generator< _TyDfa, char >
      gen("_xmlplex.h",
        "XMLPLEX", true, "ns_xmlplex",
        "state", "char32_t", "U'", "'");
    gen.add_dfa(dfaAll, dctxtAll, "startAll");
    gen.generate();
  }
  catch (exception const & _rex)
  {
    cerr << "xmlpgen: Caught exception [" << _rex.what() << "].\n";
  }

	return 0;
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
	
