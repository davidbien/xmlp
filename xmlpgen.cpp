
// xmlpgen.cpp
// dbien 25DEC2020
// Generate the lexicographical analyzer for the xmlparser implementation using the lexang templates.

// TODO: precompiled headers? probably wouldn't gain too much but it is easy.
#include "xmlpgen.h"

#ifdef __DGRAPH_COUNT_EL_ALLOC_LIFETIME
int gs_iNodesAllocated = 0;
int gs_iLinksAllocated = 0;
int gs_iNodesConstructed = 0;
int gs_iLinksConstructed = 0;
#endif //__DGRAPH_COUNT_EL_ALLOC_LIFETIME

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
void GenerateUTF32XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp );
void GenerateUTF16XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp );
void GenerateUTF8XmlLex( EGeneratorFamilyDisposition _egfdFamilyDisp );

int
TryMain( int argc, char ** argv )
{
	g_strProgramName = *argv;
	n_SysLog::InitSysLog( argv[0],  
		/*LOG_PERROR*/0, LOG_USER
	);

	// The "base" generator generates the lexicographical analyzer template as well as the state machine.
	// The "specialized" generators are only state machines and they share the lexicographical analyzer template.
	GenerateUTF32XmlLex( egfdBaseGenerator );
	GenerateUTF16XmlLex( egfdSpecializedGenerator );
	GenerateUTF8XmlLex( egfdSpecializedGenerator );
	return 0;
}
