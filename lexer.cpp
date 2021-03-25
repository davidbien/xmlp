
// lexer.cpp
// This is a simple test platform for a lexical analyzer.
// dbien
// 1999->present.

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
#include <exception>
typedef std::allocator< char >	_TyDefaultAllocator;

#include "_compat.h"
#include "_heapchk.h"

// Include lexical analyzer:
#include <string>
#include <iostream>
#include <fstream>
#include "syslogmgr.inl"
#include "_compat.inl"
#include "xml_traits.h"
#include "_xmlplex_utf32.h"
#include "_xmlplex_utf16.h"
#include "_xmlplex_utf8.h"

__BIENUTIL_USING_NAMESPACE

std::string g_strProgramName;
int TryMain( int argc, char ** argv );

template < class t_TyAnalyzer, class t_PtrReturned >
bool FTryGetToken( t_TyAnalyzer & _rA, t_PtrReturned & _rptrReturned );

int
main( int argc, char **argv )
{
#ifdef WIN32
	_set_error_mode( _OUT_TO_MSGBOX );	// Allow debugging after assert(). We use Assert() instead so this doesn't matter much.
	_CrtSetDbgFlag( _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG ) | _CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF );
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

int
TryMain( int argc, char ** argv )
{
	g_strProgramName = *argv;
	n_SysLog::InitSysLog( argv[0],
		 LOG_PERROR, LOG_USER
	);

	__XMLP_USING_NAMESPACE
	__XMLPLEX_USING_NAMESPACE

	typedef _l_transport_fixedmem< char32_t > _TyTransport;
	typedef xml_traits< _TyTransport, false, false > _TyXmlTraits;
	typedef typename _TyXmlTraits::_TyLexTraits _TyLexTraits;
	typedef _TyLexTraits _TyLexT; // shorthand for below.
	typedef TGetLexicalAnalyzer< _TyLexTraits > _TyMDAnalyzer;
	typedef _TyMDAnalyzer::_TyToken _TyToken;
	_TyMDAnalyzer	analyzer;

  typedef basic_string< char, char_traits< char >, _TyDefaultAllocator > _TyStringChar;

  typedef typename _Alloc_traits< _TyMDAnalyzer::_TyChar, _TyDefaultAllocator >::allocator_type _TyStringCvtAllocator;
  typedef basic_string< _TyMDAnalyzer::_TyChar, 
                        char_traits< _TyMDAnalyzer::_TyChar >,
                        _TyStringCvtAllocator > _TyStringCvt;

	unique_ptr< _TyToken > upToken;
  const int kiBufLen = 16384;
	_TyStringChar	bs( kiBufLen, 0 );
	while ( !cin.eof() )
	{
		cout << ">";
		cout.flush();
		bs.resize(kiBufLen, 0);
		cin.getline( bs.data(), kiBufLen-1 );
		bs.resize( StrNLen( bs.c_str() ) );

		// Convert to the type we need it to be:
		_TyStringCvt bsConvert( bs.begin(), bs.end() );

		analyzer.emplaceTransport( bsConvert.c_str(), bsConvert.length() );
		while( FTryGetToken( analyzer, upToken ) )
		{
			upToken->GetValue().ProcessStrings< char >( *upToken );
			JsoValue< char > jv;
			upToken->GetValue().ToJsoValue( jv );
			cout << "Got token: [" << upToken->GetTokenId() << "]\n" ;
			cout << jv;
			cout << "\n";
		}
	}
	
	return 0;
}

template < class t_TyAnalyzer, class t_PtrReturned >
bool FTryGetToken( t_TyAnalyzer & _rA, t_PtrReturned & _rptrReturned )
{
	__LEXOBJ_USING_NAMESPACE
	try
	{
		return _rA.FGetToken( _rptrReturned );
	}
	catch( const _l_no_token_found_exception & rexc )
	{
		n_SysLog::Log( eslmtError, "%s: *** Exception: [%s]", g_strProgramName.c_str(), rexc.what() );
		std::cerr << "Bogus token:" << rexc.what() << "\n";
		_rA.ClearTokenData();
		return false;
	}
	catch( const std::exception & rexc )
	{
		n_SysLog::Log( eslmtError, "%s: *** Exception: [%s]", g_strProgramName.c_str(), rexc.what() );
		std::cerr << "Other exception:" << rexc.what() << "\n";
		_rA.ClearTokenData();
		return false;
	}
}