
// xnltestsame.cpp
// Unit test for XML Parser to check that we can exactly copy an XML file.
// dbien
// 23JAN2021

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
#include "xml_inc.h"
#include "xml_tag.h"
#include "xml_tag_var.h"
#include "gtest/gtest.h"

__BIENUTIL_USING_NAMESPACE

std::string g_strProgramName;
int TryMain( int argc, char ** argv );

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
  #define USAGE "%s: Usage:\n%s <xml-file>\n"
	g_strProgramName = *argv;
	n_SysLog::InitSysLog( argv[0],  
		/* LOG_PERROR */0, LOG_USER
	);

  if ( argc < 2 )
  {
    fprintf( stderr, USAGE, g_strProgramName.c_str(), g_strProgramName.c_str() );
    return -1;
  }

  if ( !FFileExists( argv[1] ) )
  {
    fprintf( stderr, "%s: File does not exist[%s].\n", g_strProgramName.c_str(), argv[1] );
    fprintf( stderr, USAGE, g_strProgramName.c_str(), g_strProgramName.c_str() );
    return -2;
  }

	typedef char8_t _TyCharTest;
	__XMLP_USING_NAMESPACE
	__XMLPLEX_USING_NAMESPACE

	typedef xml_parser_var< xml_var_get_var_transport_t, tuple< tuple< char8_t >, tuple< char16_t >/*, tuple< char32_t >*/ > > _TyXmlParser;
	typedef _TyXmlParser::_TyReadCursorVar _TyXmlReadCursor;
  _TyXmlParser xmlParser;

  // Open the file:
	_TyXmlReadCursor xmlReadCursor = xmlParser.OpenFileVar< _l_transport_file >( argv[1] );
  typedef xml_document_var< _TyXmlParser::_TyTpTransports > _TyXmlDocument;
  auto prXmlDoc = _TyXmlDocument::PrCreateXmlDocument();
  _TyXmlDocument * pXmlDoc = prXmlDoc.first; // We store this separately for now - need to come up with a class infrastructure to store this paradigm.
  pXmlDoc->FromXmlStream( xmlReadCursor, prXmlDoc.second );

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