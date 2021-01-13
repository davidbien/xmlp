
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

	typedef char32_t _TyCTok;
  typedef _TyDefaultAllocator  _TyAllocator;
	typedef _regexp_final< _TyCTok, _TyAllocator > _TyFinal;
	typedef _regexp_trigger< _TyCTok, _TyAllocator > _TyTrigger;
	typedef _regexp_action< _TyCTok, _TyAllocator > _TyFreeAction;

// Define some simple macros to make the productions more readable.
#define l(x)	literal<_TyCTok>(x)
#define ls(x)	litstr<_TyCTok>(x)
#define lr(x,y)	litrange<_TyCTok>(x,y)
#define lnot(x) litnotset_no_surrogates<_TyCTok>(x)
#define lanyof(x) litanyinset<_TyCTok>(x)
#define u(n) unsatisfiable<_TyCTok>(n)
#define t(a) _TyTrigger(a)
#define a(a) _TyFreeAction(a)

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
	_TyFinal	QName = t(TyGetTriggerPrefixBegin<_TyCTok>()) * Prefix * t(TyGetTriggerPrefixEnd<_TyCTok>()) * --( l(U':')	* t( TyGetTriggerLocalPartBegin<_TyCTok>() ) * LocalPart * t( TyGetTriggerLocalPartEnd<_TyCTok>() ) );
#if 0 // We have to rid these because they match the QName production and therefore they cause trigger confusion. We'll have to check in post-processing for xmlns as the prefix.
	_TyFinal	DefaultAttName = ls(U"xmlns") /* * t( TyGetTriggerDefaultAttName<_TyCTok>() ) */;
	_TyFinal	PrefixedAttName = ls(U"xmlns:") * t( TyGetTriggerPrefixedAttNameBegin<_TyCTok>() ) * NCName * t( TyGetTriggerPrefixedAttNameEnd<_TyCTok>() );
	_TyFinal	NSAttName = PrefixedAttName | DefaultAttName;
#endif //0

// Attribute value and character data productions:
	_TyFinal	EntityRef = l(U'&') * t( TyGetTriggerEntityRefBegin<_TyCTok>() ) 	// [49]
																* Name * t( TyGetTriggerEntityRefEnd<_TyCTok>() ) * l(U';');
	_TyFinal	CharRef = ( ls(U"&#") * t( TyGetTriggerCharDecRefBegin<_TyCTok>() )  * ++lr(U'0',U'9') * t( TyGetTriggerCharDecRefEnd<_TyCTok>()  ) * l(U';') )	// [66]
							| ( ls(U"&#x") * t( TyGetTriggerCharHexRefBegin<_TyCTok>() ) * ++( lr(U'0',U'9') | lr(U'A',U'F') | lr(U'a',U'f') ) * t( TyGetTriggerCharHexRefEnd<_TyCTok>() ) * l(U';') );
	_TyFinal	Reference = EntityRef | CharRef;	// [67]
	// We use the same triggers for these just to save implementation space and because we know by the final trigger
	//	whether a single or double quote was present.
	_TyFinal _NotAmperLessDouble = lr(1,U'!') |  lr(U'#',U'%') | lr(U'\'',U';') | lr(U'=',0xD7FF) | lr(0xE000,0x10ffff); // == lnot("&<\"")
	_TyFinal _NotAmperLessSingle = lr(1,U'%') |  lr(U'(',U';') | lr(U'=',0xD7FF) | lr(0xE000,0x10ffff); // == lnot("&<\'")
	_TyFinal	_AVCharRangeNoAmperLessDouble =	t(TyGetTriggerCharDataBegin<_TyCTok>()) * ++_NotAmperLessDouble * t(TyGetTriggerCharDataEnd<_TyCTok>());
	_TyFinal	_AVCharRangeNoAmperLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyCTok>()) * ++_NotAmperLessSingle * t(TyGetTriggerCharDataSingleQuoteEnd<_TyCTok>());
	// We need only record whether single or double quotes were used as a convenience to any reader/writer system that may want to duplicate the current manner of quoting.
	_TyFinal	AttValue =	l(U'\"') * /* t( TyGetTriggerAttValueDoubleQuote<_TyCTok>() )  * */ ~( _AVCharRangeNoAmperLessDouble | Reference )  * l(U'\"') |	// [10]
												l(U'\'') * ~( _AVCharRangeNoAmperLessSingle | Reference ) * l(U'\''); // No need to record a single quote trigger as the lack of double quote is adequate.
	_TyFinal	Attribute = /* NSAttName * Eq * AttValue | */ // [41]
												QName * Eq * AttValue;

// The various types of tags:
  _TyFinal	STag = l(U'<') * QName * t(TyGetTriggerSaveTagName<_TyCTok>()) * ~( S * Attribute * t(TyGetTriggerSaveAttributes<_TyCTok>()) ) * --S * l(U'>'); // [12]
	_TyFinal	ETag = ls(U"</") * QName * --S * l(U'>');// [13]
  _TyFinal	EmptyElemTag = l(U'<') * QName * t(TyGetTriggerSaveTagName<_TyCTok>()) * ~( S * Attribute * t(TyGetTriggerSaveAttributes<_TyCTok>()) ) * --S * ls(U"/>");// [14]								

// Processsing instruction:
	_TyFinal PITarget = Name - ( ( ( l(U'x') | l(U'X') ) * ( l(U'm') | l(U'M') ) * ( l(U'U') | l(U'U') ) ) ); // This produces an anti-accepting state for "xmU" or "XMU".
	_TyFinal	PI = ls(U"<?")	* t( TyGetTriggerPITargetStart<_TyCTok>() ) * PITarget * t( TyGetTriggerPITargetEnd<_TyCTok>() )
			* ( ls(U"?>") | ( S * t( TyGetTriggerPITargetMeatBegin<_TyCTok>() ) * ( ~Char + ls(U"?>") ) ) ) * t( TyGetTriggerPITargetMeatEnd<_TyCTok>() );

// Comment:
	_TyFinal	_CharNoMinus =	l(0x09) | l(0x0a) | l(0x0d) | // [2].
										lr(0x020,0x02c) | lr(0x02e,0xd7ff) | lr(0xe000,0xfffd) | lr(0x10000,0x10ffff);
	// For a comment the end trigger was messing with the production since it kept the second '-'
	_TyFinal Comment = ls(U"<!--") /* * t(TyGetTriggerCommentBegin<_TyCTok>())  */* ~(_CharNoMinus | (l(U'-') * _CharNoMinus))/*  * t(TyGetTriggerCommentEnd<_TyCTok>()) */ * ls(U"-->");

// CDataSection:
	// As with Comments we cannot use triggers in this production since the characters in "]]>" are also present in the production Char.
	// This is not transtion from character classes where the trigger can fire.
	_TyFinal CDStart = ls(U"<![CDATA["); //[18]
	_TyFinal CDEnd = ls(U"]]>"); //[21]
	_TyFinal CDSect = CDStart * ~Char + CDEnd; 

// prolog stuff:
// XMLDecl and TextDecl(external DTDs):
	// This makes things default to "standalone='no'" when no standalone statement is present.
	// This is the statement of the specification that reflects this:
	// "If there are external markup declarations but there is no standalone document declaration, the value 'no' is assumed."
	// So we assume no and then the value is meaningless if there are no external declarations.
	_TyFinal	_YesSDDecl = ls(U"yes") * t(TyGetTriggerStandaloneYes<_TyCTok>());
	_TyFinal	_NoSDDecl = ls(U"no"); // Don't need to record when we get no.
	_TyFinal	SDDecl = S * ls(U"standalone") * Eq *		// [32]
		( ( l(U'\"') * t(TyGetTriggerStandaloneDoubleQuote<_TyCTok>()) * ( _YesSDDecl | _NoSDDecl ) * l(U'\"') ) |
			( l(U'\'') * ( _YesSDDecl | _NoSDDecl ) * l(U'\'') ) );
	_TyFinal	EncName = ( lr(U'A',U'Z') | lr(U'a',U'z') ) * // [81]
											~(	lr(U'A',U'Z') | lr(U'a',U'z') | 
													lr(U'0',U'9') | l(U'.') | l(U'_') | l(U'-') );
	_TyFinal	_TrEncName =	t(TyGetTriggerEncodingNameBegin<_TyCTok>()) *  EncName * t(TyGetTriggerEncodingNameEnd<_TyCTok>());
	_TyFinal	EncodingDecl = S * ls(U"encoding") * Eq *			// [80].
			( l(U'\"') * t( TyGetTriggerEncDeclDoubleQuote<_TyCTok>()) * _TrEncName * l(U'\"') | 
				l(U'\'') * _TrEncName * l(U'\'') );

	_TyFinal	VersionNum = ls(U"1.") * t(TyGetTriggerVersionNumBegin<_TyCTok>()) * ++lr(U'0',U'9') * t(TyGetTriggerVersionNumEnd<_TyCTok>());
	_TyFinal	VersionInfo = S * ls(U"version") * Eq * // [24]
													(	l(U'\"') * t(TyGetTriggerVersionNumDoubleQuote<_TyCTok>()) * VersionNum * l(U'\"') |
														l(U'\'') * VersionNum * l(U'\'') );
	_TyFinal	XMLDecl = ls(U"<?xmU") * VersionInfo * --EncodingDecl * --SDDecl * --S * ls(U"?>"); // [23]
 	_TyFinal	TextDecl = ls(U"<?xmU") * --VersionInfo * EncodingDecl * --S * ls(U"?>"); //[77]

// DTD stuff:
	_TyFinal PEReference = l(U'%') * t(TyGetTriggerPEReferenceBegin<_TyCTok>()) * Name * t(TyGetTriggerPEReferenceEnd<_TyCTok>()) * l(U';');
// Mixed:
	_TyFinal	_MixedBegin = l(U'(') * --S * ls(U"#PCDATA");
	_TyFinal	_TriggeredMixedName = t( TyGetTriggerMixedNameBegin<_TyCTok>() ) * Name  * t( TyGetTriggerMixedNameEnd<_TyCTok>() );
	_TyFinal	Mixed = _MixedBegin * ~( --S * l(U'|') * --S * _TriggeredMixedName )
														* --S * ls(U")*") |
										_MixedBegin * --S * l(U')'); // [51].
// EntityDecl stuff:
  _TyFinal	SystemLiteral =	l(U'\"') * t(TyGetTriggerSystemLiteralBegin<_TyCTok>()) * ~lnot(U"\"") * t(TyGetTriggerSystemLiteralDoubleQuoteEnd<_TyCTok>()) * l(U'\"') | //[11]
														l(U'\'') * t(TyGetTriggerSystemLiteralBegin<_TyCTok>()) * ~lnot(U"\'") * t(TyGetTriggerSystemLiteralSingleQuoteEnd<_TyCTok>()) * l(U'\'');
	_TyFinal	PubidCharLessSingleQuote = l(0x20) | l(0xD) | l(0xA) | lr(U'a',U'z') | lr(U'A',U'Z') | lr(U'0',U'9') | lanyof(U"-()+,./:=?;!*#@$_%");
	_TyFinal	PubidChar = PubidCharLessSingleQuote | l('\''); //[13]
	_TyFinal	PubidLiteral = l(U'\"') * t(TyGetTriggerPubidLiteralBegin<_TyCTok>()) * ~PubidChar * t(TyGetTriggerPubidLiteralDoubleQuoteEnd<_TyCTok>()) * l(U'\"') | //[12]
														l(U'\'') * t(TyGetTriggerPubidLiteralBegin<_TyCTok>()) * ~PubidCharLessSingleQuote * t(TyGetTriggerPubidLiteralSingleQuoteEnd<_TyCTok>()) * l(U'\'');
	_TyFinal	ExternalID = ls(U"SYSTEM") * S * SystemLiteral | ls(U"PUBLIC") * S * PubidLiteral * S * SystemLiteral; //[75]
  _TyFinal	NDataDecl = S * ls(U"NDATA") * S * Name; //[76]

	// Because this production is sharing triggers with AttValue they cannot appear in the same resultant production (token). That's fine because that isn't possible.
	_TyFinal	_EVCharRangeNoPercentLessDouble =	t(TyGetTriggerCharDataBegin<_TyCTok>()) * ++lnot(U"%&\"") * t(TyGetTriggerCharDataEnd<_TyCTok>());
	_TyFinal	_EVCharRangeNoPercentLessSingle =	t(TyGetTriggerCharDataSingleQuoteBegin<_TyCTok>()) * ++lnot(U"%&\'") * t(TyGetTriggerCharDataSingleQuoteEnd<_TyCTok>());
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
	// _TyFinal Test = ++l('a') * t(TyGetTriggerStandaloneYes<_TyCTok>()) * ++l('b');
	// static const vtyActionIdent s_knTokenTest = 2000;
	// typedef _l_action_token< _l_action_save_data_single< s_knTokenTest, true, TyGetTriggerStandaloneYes > > TyGetTokenTest;
	// Test.SetAction( TyGetTokenTest<_TyCTok>() );
	STag.SetAction( TyGetTokenSTag<_TyCTok>() );
	ETag.SetAction( TyGetTokenETag<_TyCTok>() );
	EmptyElemTag.SetAction( TyGetTokenEmptyElemTag<_TyCTok>() );
	Comment.SetAction( TyGetTokenComment<_TyCTok>() );
	XMLDecl.SetAction( TyGetTokenXMLDecl<_TyCTok>() );
	CDSect.SetAction( TyGetTokenCDataSection<_TyCTok>() );
#else //!REGEXP_NO_TRIGGERS
	STag.SetAction( _l_action_token< _TyCTok, 1 >() );
	ETag.SetAction( _l_action_token< _TyCTok, 2 >() );
	EmptyElemTag.SetAction( _l_action_token< _TyCTok, 3 >() );
	Comment.SetAction( _l_action_token< _TyCTok, 4 >() );
	XMLDecl.SetAction( _l_action_token< _TyCTok, 5 >() );
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
	
	typedef _dfa< _TyCTok, _TyAllocator >	_TyDfa;
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
	
