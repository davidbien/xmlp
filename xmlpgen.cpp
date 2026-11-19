
// xmlpgen.cpp
// dbien 25DEC2020
// Generate the lexicographical analyzer for the xmlparser implementation using the lexang templates.

// TODO: precompiled headers? probably wouldn't gain too much but it is easy.
#include "xmlpgen.h"

__REGEXP_USING_NAMESPACE
__XMLP_USING_NAMESPACE

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
// Each gives the modes of its lexical analyzer for the parser of the DTD: its DTD DFAs and the tokens of each.
typedef ns_pg::LexerModes< vtyTokenIdent > _TyDtdLexerModes;
void GenerateUTF32XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp, _TyDtdLexerModes & _rmodesDtd );
void GenerateUTF16XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp, _TyDtdLexerModes & _rmodesDtd );
void GenerateUTF8XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp, _TyDtdLexerModes & _rmodesDtd );
// The parser of the DTD - xmlpgen_dtd.cpp. The one parser is used with each of the lexical analyzers.
void GenerateDtdParser( _TyDtdLexerModes const & _rmodes );

int
TryMain( int argc, char ** argv )
{
	g_strProgramName = *argv;
	n_SysLog::InitSysLog( argv[0],  
		/*LOG_PERROR*/0, LOG_USER
	);

	// The "base" generator generates the lexicographical analyzer template as well as the state machine.
	// The "specialized" generators are only state machines and they share the lexicographical analyzer template.
	_TyDtdLexerModes modesDtdUTF32, modesDtdUTF16, modesDtdUTF8;
	GenerateUTF32XmlLex( egfdBaseGenerator, modesDtdUTF32 );
	GenerateUTF16XmlLex( egfdSpecializedGenerator, modesDtdUTF16 );
	GenerateUTF8XmlLex( egfdSpecializedGenerator, modesDtdUTF8 );
	// The parser of the DTD is generated once: the modes - and the tokens of each - must be the same for each character type.
	for ( const _TyDtdLexerModes * pmodes : { &modesDtdUTF16, &modesDtdUTF8 } )
	{
		VerifyThrowSz( pmodes->RgModeNames() == modesDtdUTF32.RgModeNames(), "The DTD modes of the lexical analyzers differ." );
		for ( uint32_t nMode = 0; nMode < uint32_t( modesDtdUTF32.NModes() ); ++nMode )
			VerifyThrowSz( pmodes->RGetTokensOfMode( nMode ) == modesDtdUTF32.RGetTokensOfMode( nMode ), "The tokens of DTD mode [%s] differ between the lexical analyzers.", modesDtdUTF32.RgModeNames()[ nMode ].c_str() );
	}
	GenerateDtdParser( modesDtdUTF32 );
	return 0;
}
