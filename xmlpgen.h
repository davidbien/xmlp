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
// The parser generator: for the parser of the DTD - see xmlpgen_dtd.cpp. Before the single letter macros below.
#include "_p_inc.h"
#include "_p_lexgen.h"
#include <fstream>
#include <tuple>
#include <vector>
#include <algorithm>
#include <initializer_list>

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

// THis could probably be somewhere within lexang as a utility function.
template < class t_TyFinal, class t_TyDfa, class t_TyDfaCtxt >
void
gen_dfa( t_TyFinal const & _rFinal, t_TyDfa & _rDfa, t_TyDfaCtxt & _rDfaCtxt, uint32_t _grfNFACreationOptions = 0 )
{
__REGEXP_USING_NAMESPACE
__XMLP_USING_NAMESPACE
	bool	fOptimizeDfa = /* false */true;

	typedef t_TyDfa	_TyDfa;
	typedef t_TyDfaCtxt	_TyDfaCtxt;
	typedef _nfa< typename _TyDfa::_TyChar, typename _TyDfa::_TyAllocator >	_TyNfa;
	typedef typename _TyNfa::_TyNfaCtxt _TyNfaCtxt;

	{//B
		_TyNfa	nfa( _grfNFACreationOptions, _rDfa.get_allocator() );
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
	

// The character classes of XML as the specification has them: ranges of code points - once, for the three encodings.
// A production of one of the generators is written with the characters of its encoding: a code point is one of
//  those in UTF-32, one or two in UTF-16 and from one to four in UTF-8. RegexpOfCodePoints() makes of a class of code
//  points the expression which matches one character of the class as it is encoded - so a class is written here as
//  the specification writes it and each generator uses it as it would a range of its own characters:
//    _TyFinal NameStartChar = XmlChars( vrgxcprNameStartChar );
//    _TyFinal CharData = ++XmlChars( vrgxcprChar, U"<&" ) ...   - any Char [2] but '<' and '&'.
// What is not in a class is then no character of a token: a control character, a surrogate which is not half of a
//  pair, #xFFFE, #xFFFF and - in UTF-8 - bytes which are not the encoding of a character.
struct XmlCodePointRange
{
  char32_t m_chFirst;
  char32_t m_chLast;
};
// [2] Char ::= #x9 | #xA | #xD | [#x20-#xD7FF] | [#xE000-#xFFFD] | [#x10000-#x10FFFF]
static const std::initializer_list< XmlCodePointRange > vrgxcprChar = { { 0x9, 0x9 }, { 0xA, 0xA }, { 0xD, 0xD }, { 0x20, 0xD7FF }, { 0xE000, 0xFFFD }, { 0x10000, 0x10FFFF } };
// [4] NameStartChar ::= ":" | [A-Z] | "_" | [a-z] | [#xC0-#xD6] | [#xD8-#xF6] | [#xF8-#x2FF] | [#x370-#x37D] | [#x37F-#x1FFF] | [#x200C-#x200D]
//                        | [#x2070-#x218F] | [#x2C00-#x2FEF] | [#x3001-#xD7FF] | [#xF900-#xFDCF] | [#xFDF0-#xFFFD] | [#x10000-#xEFFFF]
static const std::initializer_list< XmlCodePointRange > vrgxcprNameStartChar = {
  { U':', U':' }, { U'A', U'Z' }, { U'_', U'_' }, { U'a', U'z' }, { 0xC0, 0xD6 }, { 0xD8, 0xF6 }, { 0xF8, 0x2FF }, { 0x370, 0x37D }, { 0x37F, 0x1FFF }, { 0x200C, 0x200D },
  { 0x2070, 0x218F }, { 0x2C00, 0x2FEF }, { 0x3001, 0xD7FF }, { 0xF900, 0xFDCF }, { 0xFDF0, 0xFFFD }, { 0x10000, 0xEFFFF } };
// [4a] NameChar ::= NameStartChar | "-" | "." | [0-9] | #xB7 | [#x0300-#x036F] | [#x203F-#x2040]
static const std::initializer_list< XmlCodePointRange > vrgxcprNameChar = {
  { U':', U':' }, { U'A', U'Z' }, { U'_', U'_' }, { U'a', U'z' }, { 0xC0, 0xD6 }, { 0xD8, 0xF6 }, { 0xF8, 0x2FF }, { 0x370, 0x37D }, { 0x37F, 0x1FFF }, { 0x200C, 0x200D },
  { 0x2070, 0x218F }, { 0x2C00, 0x2FEF }, { 0x3001, 0xD7FF }, { 0xF900, 0xFDCF }, { 0xFDF0, 0xFFFD }, { 0x10000, 0xEFFFF },
  { U'-', U'-' }, { U'.', U'.' }, { U'0', U'9' }, { 0xB7, 0xB7 }, { 0x0300, 0x036F }, { 0x203F, 0x2040 } };

// The ranges of the class without the code points of _pszBut - which may be null - in order and not overlapping.
inline std::vector< XmlCodePointRange > RgXmlCodePointRanges( std::initializer_list< XmlCodePointRange > const & _rilRanges, const char32_t * _pszBut )
{
  std::vector< XmlCodePointRange > rgRanges( _rilRanges );
  std::sort( rgRanges.begin(), rgRanges.end(), []( XmlCodePointRange const & _rl, XmlCodePointRange const & _rr ) { return _rl.m_chFirst < _rr.m_chFirst; } );
  for ( ; !!_pszBut && !!*_pszBut; ++_pszBut )
  {
    const char32_t chBut = *_pszBut;
    for ( size_t n = 0; n < rgRanges.size(); ++n )
    {
      XmlCodePointRange & rxcpr = rgRanges[n];
      if ( ( chBut < rxcpr.m_chFirst ) || ( chBut > rxcpr.m_chLast ) )
        continue;
      if ( rxcpr.m_chFirst == rxcpr.m_chLast )
        rgRanges.erase( rgRanges.begin() + n );
      else
      if ( chBut == rxcpr.m_chFirst )
        ++rxcpr.m_chFirst;
      else
      if ( chBut == rxcpr.m_chLast )
        --rxcpr.m_chLast;
      else
      {
        const XmlCodePointRange xcprAfter{ char32_t( chBut + 1 ), rxcpr.m_chLast };
        rxcpr.m_chLast = chBut - 1;
        rgRanges.insert( rgRanges.begin() + n + 1, xcprAfter );
      }
      break;
    }
  }
  return rgRanges;
}
// A range of code points as sequences of ranges of the characters of an encoding: each sequence is a range for each
//  character of the encoding of a code point - any code point whose characters are in those ranges, one after another,
//  is of the range, and all of the range is in the sequences.
typedef std::vector< std::pair< char32_t, char32_t > > XmlEncodedRangeSequence;
// UTF-32: the code point is the character.
inline void AddEncodedRangeSequences( char32_t, char32_t _chFirst, char32_t _chLast, std::vector< XmlEncodedRangeSequence > & _rrgSequences )
{
  _rrgSequences.push_back( XmlEncodedRangeSequence{ { _chFirst, _chLast } } );
}
// UTF-16: a code point above #xFFFF is a pair of surrogates - #xD800 and the high ten bits, #xDC00 and the low ten.
inline void AddEncodedRangeSequences( char16_t, char32_t _chFirst, char32_t _chLast, std::vector< XmlEncodedRangeSequence > & _rrgSequences )
{
  if ( _chFirst <= 0xFFFF )
  {
    _rrgSequences.push_back( XmlEncodedRangeSequence{ { _chFirst, (std::min)( _chLast, char32_t( 0xFFFF ) ) } } );
    if ( _chLast <= 0xFFFF )
      return;
    _chFirst = 0x10000;
  }
  const char32_t chHighFirst = 0xD800 + ( ( _chFirst - 0x10000 ) >> 10 ), chLowFirst = 0xDC00 + ( ( _chFirst - 0x10000 ) & 0x3FF );
  const char32_t chHighLast = 0xD800 + ( ( _chLast - 0x10000 ) >> 10 ), chLowLast = 0xDC00 + ( ( _chLast - 0x10000 ) & 0x3FF );
  if ( chHighFirst == chHighLast )
  {
    _rrgSequences.push_back( XmlEncodedRangeSequence{ { chHighFirst, chHighFirst }, { chLowFirst, chLowLast } } );
    return;
  }
  // The first high surrogate with what is left of its low ones, those between with all of theirs, the last with its first ones.
  char32_t chHighWholeFirst = chHighFirst, chHighWholeLast = chHighLast;
  if ( 0xDC00 != chLowFirst )
  {
    _rrgSequences.push_back( XmlEncodedRangeSequence{ { chHighFirst, chHighFirst }, { chLowFirst, 0xDFFF } } );
    ++chHighWholeFirst;
  }
  if ( 0xDFFF != chLowLast )
  {
    _rrgSequences.push_back( XmlEncodedRangeSequence{ { chHighLast, chHighLast }, { 0xDC00, chLowLast } } );
    --chHighWholeLast;
  }
  if ( chHighWholeFirst <= chHighWholeLast )
    _rrgSequences.push_back( XmlEncodedRangeSequence{ { chHighWholeFirst, chHighWholeLast }, { 0xDC00, 0xDFFF } } );
}
// UTF-8: from one to four bytes. The range is taken apart until what is left is code points of one length whose
//  bytes - first to last - are each a range: the same leading bits, and every byte after the first which differs
//  between the ends of the range is all that a byte may be.
inline void AddEncodedRangeSequences( char8_t _chEncoding, char32_t _chFirst, char32_t _chLast, std::vector< XmlEncodedRangeSequence > & _rrgSequences )
{
  static const char32_t s_rgchLastOfLength[] = { 0x7F, 0x7FF, 0xFFFF, 0x10FFFF };
  for ( const char32_t chLastOfLength : s_rgchLastOfLength )
  {
    if ( ( _chFirst <= chLastOfLength ) && ( _chLast > chLastOfLength ) )
    {
      AddEncodedRangeSequences( _chEncoding, _chFirst, chLastOfLength, _rrgSequences );
      AddEncodedRangeSequences( _chEncoding, chLastOfLength + 1, _chLast, _rrgSequences );
      return;
    }
  }
  const size_t knBytes = ( _chLast <= 0x7F ) ? 1 : ( ( _chLast <= 0x7FF ) ? 2 : ( ( _chLast <= 0xFFFF ) ? 3 : 4 ) );
  for ( size_t nByte = 1; nByte < knBytes; ++nByte )
  {
    const char32_t chMask = ( char32_t( 1 ) << ( 6 * nByte ) ) - 1; // The bits of the last nByte bytes.
    if ( ( _chFirst & ~chMask ) != ( _chLast & ~chMask ) )
    {
      if ( !!( _chFirst & chMask ) )
      {
        AddEncodedRangeSequences( _chEncoding, _chFirst, _chFirst | chMask, _rrgSequences );
        AddEncodedRangeSequences( _chEncoding, ( _chFirst | chMask ) + 1, _chLast, _rrgSequences );
        return;
      }
      if ( ( _chLast & chMask ) != chMask )
      {
        AddEncodedRangeSequences( _chEncoding, _chFirst, ( _chLast & ~chMask ) - 1, _rrgSequences );
        AddEncodedRangeSequences( _chEncoding, _chLast & ~chMask, _chLast, _rrgSequences );
        return;
      }
    }
  }
  auto ByteOf = [knBytes]( char32_t _ch, size_t _nByte ) -> char32_t
  {
    const char32_t chBits = _ch >> ( 6 * ( knBytes - 1 - _nByte ) );
    if ( 1 == knBytes )
      return chBits;
    if ( !_nByte )
      return ( ( 2 == knBytes ) ? 0xC0 : ( ( 3 == knBytes ) ? 0xE0 : 0xF0 ) ) | chBits;
    return 0x80 | ( chBits & 0x3F );
  };
  XmlEncodedRangeSequence sequence;
  for ( size_t nByte = 0; nByte < knBytes; ++nByte )
    sequence.push_back( { ByteOf( _chFirst, nByte ), ByteOf( _chLast, nByte ) } );
  _rrgSequences.push_back( std::move( sequence ) );
}
// The expression which matches one character of the class as it is encoded in t_TyCTok: see above.
//  _pszBut: code points of the class which are not to match - null for none.
template < class t_TyCTok, class t_TyAllocator >
__REGEXP_NAMESPACE _regexp_final< t_TyCTok, t_TyAllocator >
RegexpOfCodePoints( std::initializer_list< XmlCodePointRange > const & _rilRanges, const char32_t * _pszBut = nullptr )
{
  __REGEXP_USING_NAMESPACE
  __REGEXP_OP_USING_NAMESPACE
  typedef _regexp_final< t_TyCTok, t_TyAllocator > _TyFinal;
  std::vector< XmlEncodedRangeSequence > rgSequences;
  for ( XmlCodePointRange const & rxcpr : RgXmlCodePointRanges( _rilRanges, _pszBut ) )
    AddEncodedRangeSequences( t_TyCTok(), rxcpr.m_chFirst, rxcpr.m_chLast, rgSequences );
  VerifyThrowSz( !rgSequences.empty(), "A class of characters which has none." );
  std::unique_ptr< _TyFinal > upfClass;
  for ( XmlEncodedRangeSequence const & rsequence : rgSequences )
  {
    std::unique_ptr< _TyFinal > upfSequence;
    for ( std::pair< char32_t, char32_t > const & rprRange : rsequence )
    {
      const _TyFinal fRange( litrange< t_TyCTok >( t_TyCTok( rprRange.first ), t_TyCTok( rprRange.second ) ) );
      upfSequence = !upfSequence ? std::make_unique< _TyFinal >( fRange ) : std::make_unique< _TyFinal >( *upfSequence * fRange );
    }
    upfClass = !upfClass ? std::make_unique< _TyFinal >( *upfSequence ) : std::make_unique< _TyFinal >( *upfClass | *upfSequence );
  }
  return *upfClass;
}
