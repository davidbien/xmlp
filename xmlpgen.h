#pragma once

// xmlpgen.h
// Include file for XML lex.
// dbien
// 18FEB2021
// Pulling this stuff out so I can put each generator in a different file merely so I can easily diff the generators.

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
#include "_heapchk.h"

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

#define l(x)	literal<_TyCTok>(x)
#define ls(x)	litstr<_TyCTok>(x)
#define lr(x,y)	litrange<_TyCTok>(x,y)
#define lanyof(x) litanyinset<_TyCTok>(x)
#define u(n) unsatisfiable<_TyLexT>(n)
#define t(a) _TyTrigger(a)
#define a(a) _TyFreeAction(a)

template < class t_TyFinal, class t_TyDfa, class t_TyDfaCtxt >
void
gen_dfa( t_TyFinal const & _rFinal, t_TyDfa & _rDfa, t_TyDfaCtxt & _rDfaCtxt )
{
__REGEXP_USING_NAMESPACE
__XMLP_USING_NAMESPACE
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
	
