// xmlpgtest_writer.cpp
// Google test stuff for XML Parser.
// The writer and what it makes - xml_writer<>::StartTag( name ), WriteCharData(), WriteComment() and the rest: a document
//  which isn't read but made. What is written is what is expected - in each encoding - and it is read again: it is a
//  document, and it has what was given. And what can't be written throws: a name which isn't one, "--" within a comment,
//  a second document element, a reference to an entity which isn't declared...
// 20NOV2026

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

#include <string>
#include <vector>
#include <cstring>
#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include "syslogmgr.inl"
#include "_compat.inl"
#include "xml_inc.h"
#include "xml_tag.h"
#include "xml_transport.h"
#include "xml_writer.h"
#include "xml_dtd.h"
#include "gtest/gtest.h"

__BIENUTIL_USING_NAMESPACE
std::string g_strProgramName;

namespace ns_XMLGTestWriter
{
__XMLP_USING_NAMESPACE
__XMLPLEX_USING_NAMESPACE

template < class t_TyChar >
class WriterTest : public testing::Test
{
};
typedef testing::Types< char8_t, char16_t, char32_t > TyWriterCharTypes;
struct WriterCharTypeNames
{
  template < class t_TyChar >
  static std::string GetName( int )
  {
    return std::is_same_v< t_TyChar, char8_t > ? "UTF8" : ( std::is_same_v< t_TyChar, char16_t > ? "UTF16" : "UTF32" );
  }
};
TYPED_TEST_SUITE( WriterTest, TyWriterCharTypes, WriterCharTypeNames );

template < class t_TyChar >
std::string StrNarrow( std::basic_string< t_TyChar > const & _rstr )
{
  std::string str;
  for ( t_TyChar ch : _rstr )
    str += ( ( ( ch >= 0x20 ) && ( ch < 0x7f ) ) || ( 0xa == ch ) || ( 0x9 == ch ) ) ? char( ch ) : '?';
  return str;
}
// The string in the type of character: from UTF-32.
template < class t_TyChar >
std::basic_string< t_TyChar > StrOf( std::u32string_view _sv )
{
  std::basic_string< t_TyChar > str;
  ConvertString( str, _sv.data(), _sv.length() );
  return str;
}

// The writer of the tests: to memory, in the type of character - without a byte order mark or an XML declaration unless
//  they are asked for.
template < class t_TyChar >
using TyTestXmlWriter = xml_writer< typename map_input_to_any_output_transport< _l_transport_fixedmem< t_TyChar >, t_TyChar, false_type >::_TyXmlWriteTransport >;
// What a writer has written to memory.
template < class t_TyChar, class t_TyXmlWriter >
std::basic_string< t_TyChar > StrWrittenBy( t_TyXmlWriter & _rxw )
{
  auto & rmsOut = _rxw.GetTransportOut().GetMemStream();
  const size_t knbyWritten = (size_t)rmsOut.GetEndPos();
  std::basic_string< t_TyChar > str( knbyWritten / sizeof( t_TyChar ), 0 );
  rmsOut.Seek( 0, vkSeekBegin );
  rmsOut.Read( str.data(), knbyWritten );
  return str;
}
template < class t_TyChar >
struct TestWriter
{
  TyTestXmlWriter< t_TyChar > m_xw;
  explicit TestWriter( bool _fUseNamespaces = true, bool _fWriteXMLDecl = false, const _xml_output_format * _pxof = nullptr )
  {
    m_xw.SetWriteBOM( false );
    m_xw.SetWriteXMLDecl( _fWriteXMLDecl );
    m_xw.SetUseNamespaces( _fUseNamespaces );
    if ( !!_pxof )
      m_xw.SetOutputFormat( *_pxof );
    m_xw.OpenMemFile();
  }
  TyTestXmlWriter< t_TyChar > * operator ->()
  {
    return &m_xw;
  }
  std::basic_string< t_TyChar > StrWritten()
  {
    return StrWrittenBy< t_TyChar >( m_xw );
  }
  std::string StrNarrowWritten()
  {
    return StrNarrow( StrWritten() );
  }
};

// What was written read again - with the DTD supported, with namespaces or without.
template < class t_TyChar >
struct ReadDocument
{
  typedef xml_traits< _l_transport_fixedmem< t_TyChar >, true, false > _TyXmlTraits;
  typedef xml_parser< _TyXmlTraits > _TyXmlParser;
  typedef xml_read_cursor< _TyXmlTraits > _TyCursor;
  std::basic_string< t_TyChar > m_strText;
  _TyXmlParser m_xmlParser;
  _TyCursor m_cursor;
  std::string m_strError;

  ReadDocument( std::basic_string< t_TyChar > const & _rstrText, bool _fUseXMLNamespaces = true )
  {
    if ( sizeof( t_TyChar ) > 1 )
      m_strText += t_TyChar( 0xFEFF ); // How the encoding of what is in memory is known.
    m_strText += _rstrText;
    try
    {
      m_xmlParser.SetUseXMLNamespaces( _fUseXMLNamespaces );
      m_cursor = m_xmlParser.OpenMemory( m_strText.data(), m_strText.length() * sizeof( t_TyChar ) );
    }
    catch ( std::exception const & rexc )
    {
      m_strError = rexc.what();
      if ( m_strError.empty() )
        m_strError = "error";
    }
  }
  // Read all of it - into a tree: the error, if it isn't a document.
  std::string StrReadAll()
  {
    if ( !m_strError.empty() )
      return m_strError;
    try
    {
      auto prXmlDoc = xml_document< _TyXmlTraits >::PrCreateXmlDocument();
      prXmlDoc.first->FromXmlStream( m_cursor, prXmlDoc.second );
    }
    catch ( std::exception const & rexc )
    {
      std::string str = rexc.what();
      return str.empty() ? std::string( "error" ) : str;
    }
    return std::string();
  }
  // The character data of the current tag - up to its first child - with its references replaced.
  std::basic_string< t_TyChar > StrContent()
  {
    std::basic_string< t_TyChar > str;
    m_cursor.GetContextCur().GetContentString( str );
    return str;
  }
  // The value of the attribute of the current tag - as it is read.
  std::basic_string< t_TyChar > StrAttrValue( size_t _nAttr )
  {
    auto & rltokTag = m_cursor.GetTagCur().GetLexToken();
    auto & rrgAttrs = rltokTag.GetValue()[ vknAttributesIdx ];
    std::basic_string_view< t_TyChar > svValue;
    rrgAttrs[ _nAttr ][ vknAttr_ValueIdx ].GetStringView( rltokTag, svValue );
    return std::basic_string< t_TyChar >( svValue );
  }
};
// Read the text: it is to be a document.
template < class t_TyChar >
void ExpectDocument( std::basic_string< t_TyChar > const & _rstrText, bool _fUseXMLNamespaces = true )
{
  ReadDocument< t_TyChar > doc( _rstrText, _fUseXMLNamespaces );
  EXPECT_EQ( doc.StrReadAll(), "" ) << StrNarrow( _rstrText );
}

// A document with each thing which may be made - and its strings escaped as they are to be.
TYPED_TEST( WriterTest, MadeDocument )
{
  typedef TypeParam TyChar;
  TestWriter< TyChar > tw;
  {
    auto xwtDoc = tw->StartTag( "doc" );
    xwtDoc.AddAttribute( "a", "1" );
    xwtDoc.AddAttribute( "n", 42 );
    xwtDoc.AddAttribute( "neg", -7 );
    xwtDoc.AddAttribute( "f", 1.5 );
    xwtDoc.AddAttribute( "b", true );
    xwtDoc.AddAttribute( "esc", "q\"<&>'\tx\ny" ); // The quote in use, the markup, and what would be read as a space.
    xwtDoc.FormatAttribute( "fmt", "%d-%s", 3, "three" );
    tw->WriteCharData( "a < b & c ]]> d" );
    {
      auto xwtChild = tw->StartTag( "child" );
    }
    tw->WriteComment( " comment " );
    tw->WriteComment( "" );
    tw->WriteProcessingInstruction( "pi", "data" );
    tw->WriteProcessingInstruction( "empty", "" );
    tw->WriteCDataSection( "x < ]]> y" );
    tw->WriteCharReference( 0x2028 );
    tw->WriteEntityReference( "amp" );
    tw->WriteElement( "text", "in <text>" );
    tw->WriteElement( "none", "" );
    {
      auto xwtFull = tw->StartTag( "full" );
      xwtFull.SetFullEnd();
    }
    tw->WriteCharData( "\r" ); // Written as a reference: it would be read as the end of a line.
  }
  tw->WriteCharData( "\n" ); // White space after the document element.
  tw->WriteComment( "after" );
  tw->EndDocument();
  const std::basic_string< TyChar > strWritten = tw.StrWritten();
  EXPECT_EQ( StrNarrow( strWritten ),
    "<doc a=\"1\" n=\"42\" neg=\"-7\" f=\"1.5\" b=\"true\" esc=\"q&quot;&lt;&amp;>'&#9;x&#10;y\" fmt=\"3-three\">"
    "a &lt; b &amp; c ]]&gt; d<child/><!-- comment --><!----><?pi data?><?empty?>"
    "<![CDATA[x < ]]]]><![CDATA[> y]]>&#x2028;&amp;<text>in &lt;text></text><none/><full></full>&#13;</doc>\n<!--after-->" );
  // Read again it is what was given.
  ReadDocument< TyChar > doc( strWritten );
  ASSERT_EQ( doc.m_strError, "" );
  ASSERT_TRUE( doc.m_cursor.FMoveDown() );
  EXPECT_EQ( StrNarrow( doc.StrAttrValue( 5 ) ), "q\"<&>'\tx\ny" );
  EXPECT_EQ( StrNarrow( doc.StrContent() ), "a < b & c ]]> d" );
  ReadDocument< TyChar > docAll( strWritten );
  EXPECT_EQ( docAll.StrReadAll(), "" );
}

// The strings may be of any type of character - and a character which isn't ASCII is written in the encoding.
TYPED_TEST( WriterTest, StringsOfEachType )
{
  typedef TypeParam TyChar;
  TestWriter< TyChar > tw;
  {
    auto xwtDoc = tw->StartTag( u"d\U000000E9" );
    xwtDoc.AddAttribute( std::string( "s" ), std::u32string( U"\U0001F600" ) );
    xwtDoc.AddAttribute( std::wstring( L"w" ), std::wstring_view( L"\U000000FC" ) );
    xwtDoc.AddAttribute( std::u8string_view( u8"u8" ), u8"\U00004E2D" );
    const char * pszNull = nullptr;
    xwtDoc.AddAttribute( "null", pszNull );
    tw->WriteCharData( U"\U000000E9\U0001F600" );
  }
  tw->EndDocument();
  const std::basic_string< TyChar > strWritten = tw.StrWritten();
  EXPECT_TRUE( strWritten == StrOf< TyChar >( U"<d\U000000E9 s=\"\U0001F600\" w=\"\U000000FC\" u8=\"\U00004E2D\" null=\"\">\U000000E9\U0001F600</d\U000000E9>" ) ) << StrNarrow( strWritten );
  ExpectDocument( strWritten );
}

// The XML declaration names the encoding which is written.
TYPED_TEST( WriterTest, XMLDecl )
{
  typedef TypeParam TyChar;
  TestWriter< TyChar > tw( true, true );
  tw->WriteElement( "doc", "" );
  tw->EndDocument();
  const std::string strWritten = tw.StrNarrowWritten();
  const char * pszEncoding = std::is_same_v< TyChar, char8_t > ? "UTF-8" : ( std::is_same_v< TyChar, char16_t > ? "UTF-16" : "UTF-32" );
  EXPECT_EQ( strWritten.substr( 0, strWritten.find( "?>" ) + 2 ).find( pszEncoding ) != std::string::npos, true ) << strWritten;
  ExpectDocument( tw.StrWritten() );
}

// Namespaces: declared where they are to be - and not where the one in scope is the same.
TYPED_TEST( WriterTest, Namespaces )
{
  typedef TypeParam TyChar;
  TestWriter< TyChar > tw;
  {
    auto xwtDoc = tw->StartTag( "doc", "", "urn:d" ); // The default namespace.
    auto xnvwX = xwtDoc.AddNamespace( "x", "urn:x" );
    xwtDoc.AddAttribute( xnvwX, "a", "1" ); // x:a
    xwtDoc.AddAttribute( "plain", "2" ); // In no namespace.
    {
      auto xwtItem = tw->StartTag( xnvwX, "item" ); // x:item
    }
    {
      auto xwtItem = tw->StartTag( "x:item2" );
      xwtItem.AddNamespace( "x", "urn:x" ); // In scope already: nothing is declared.
    }
    {
      auto xwtK = tw->StartTag( "y:k", "y", "urn:y" );
      xwtK.AddAttribute( "y:at", "3" );
    }
    {
      auto xwtK = tw->StartTag( "x:k" );
      xwtK.AddAttribute( "x:at", "4" );
      xwtK.AddNamespace( "x", "urn:other" ); // The tag and its attribute are in it.
    }
    {
      auto xwtE = tw->StartTag( "e" );
      xwtE.AddNamespace( "", "" ); // No default namespace within.
      tw->StartTag( "f" ).EndTag();
    }
    {
      auto xwtG = tw->StartTag( "g" );
      xwtG.SetDefaultAttributeNamespace( xnvwX );
      xwtG.AddAttribute( "b", "5" ); // x:b
    }
    tw->StartTag( "xml:space-free" ).EndTag(); // The prefix xml is always in scope.
  }
  tw->EndDocument();
  const std::basic_string< TyChar > strWritten = tw.StrWritten();
  EXPECT_EQ( StrNarrow( strWritten ),
    "<doc xmlns=\"urn:d\" xmlns:x=\"urn:x\" x:a=\"1\" plain=\"2\"><x:item/><x:item2/><y:k xmlns:y=\"urn:y\" y:at=\"3\"/>"
    "<x:k x:at=\"4\" xmlns:x=\"urn:other\"/><e xmlns=\"\"><f/></e><g x:b=\"5\"/><xml:space-free/></doc>" );
  ExpectDocument( strWritten );
  // Read again, each tag is in its namespace.
  ReadDocument< TyChar > doc( strWritten );
  ASSERT_EQ( doc.m_strError, "" );
  ASSERT_TRUE( doc.m_cursor.FMoveDown() );
  auto StrUriCur = [&doc]() -> std::string
  {
    auto const & rxtTag = doc.m_cursor.GetTagCur();
    if ( !rxtTag.FHasNamespace() )
      return "";
    return StrNarrow( std::basic_string< TyChar >( rxtTag.GetNamespaceReference( enrtStandaloneReference ).RStringUri() ) );
  };
  EXPECT_EQ( StrUriCur(), "urn:d" );
  ASSERT_TRUE( doc.m_cursor.FMoveDown() );
  EXPECT_EQ( StrUriCur(), "urn:x" );
  ASSERT_TRUE( doc.m_cursor.FNextTag() );
  EXPECT_EQ( StrUriCur(), "urn:x" );
  ASSERT_TRUE( doc.m_cursor.FNextTag() );
  EXPECT_EQ( StrUriCur(), "urn:y" );
  ASSERT_TRUE( doc.m_cursor.FNextTag() );
  EXPECT_EQ( StrUriCur(), "urn:other" );
  ASSERT_TRUE( doc.m_cursor.FNextTag() );
  EXPECT_EQ( StrUriCur(), "" ); // e
}

// What isn't to be with namespaces - each throws, and the writer goes on as it was.
TYPED_TEST( WriterTest, NamespaceErrors )
{
  typedef TypeParam TyChar;
  TestWriter< TyChar > tw;
  {
    auto xwtDoc = tw->StartTag( "doc" );
    EXPECT_ANY_THROW( tw->StartTag( "z:q" ) ); // Not in scope.
    EXPECT_ANY_THROW( tw->StartTag( "a:b:c" ) ); // Not a qualified name.
    EXPECT_ANY_THROW( tw->StartTag( "x:q", "y", "urn:y" ) ); // Not the prefix of the name.
    EXPECT_ANY_THROW( tw->StartTag( "q", "y", "" ) ); // A prefix is bound to a URI.
    EXPECT_ANY_THROW( tw->StartTag( "xmlns:q" ) );
    {
      auto xwtT = tw->StartTag( "t" );
      EXPECT_ANY_THROW( xwtT.AddAttribute( "xmlns:q", "urn:q" ) ); // By AddNamespace().
      EXPECT_ANY_THROW( xwtT.AddAttribute( "xmlns", "urn:q" ) );
      EXPECT_ANY_THROW( xwtT.AddAttribute( "z:a", "1" ) ); // Not in scope.
      EXPECT_ANY_THROW( xwtT.AddNamespace( "xmlns", "urn:q" ) );
      EXPECT_ANY_THROW( xwtT.AddNamespace( "q", "http://www.w3.org/2000/xmlns/" ) );
      EXPECT_ANY_THROW( xwtT.AddNamespace( "q", "http://www.w3.org/XML/1998/namespace" ) );
      EXPECT_ANY_THROW( xwtT.AddNamespace( "xml", "urn:q" ) );
      EXPECT_ANY_THROW( xwtT.AddNamespace( "1q", "urn:q" ) );
      auto xnvwDefault = xwtT.AddNamespace( "", "urn:default" );
      EXPECT_ANY_THROW( xwtT.AddAttribute( xnvwDefault, "a", "1" ) ); // An attribute isn't in the default namespace.
      xwtT.AddNamespace( "q", "urn:q" );
      EXPECT_ANY_THROW( xwtT.AddNamespace( "q", "urn:q2" ) ); // Declared on this tag already.
      xwtT.AddAttribute( "q:a", "1" );
    }
    {
      auto xwtLater = tw->StartTag( "later" );
      xwtLater.Commit();
      EXPECT_ANY_THROW( xwtLater.AddAttribute( "a", "1" ) ); // It was written.
      EXPECT_ANY_THROW( xwtLater.AddNamespace( "p", "urn:p" ) );
    }
  }
  tw->EndDocument();
  EXPECT_EQ( tw.StrNarrowWritten(), "<doc><t xmlns=\"urn:default\" xmlns:q=\"urn:q\" q:a=\"1\"/><later/></doc>" );
  ExpectDocument( tw.StrWritten() );
  // The same attribute twice - by its name, or by its namespace and local name: when the tag is written. ( A tag which
  //  can't be written throws again as it is ended - here as the exception unwinds, when it doesn't. )
  EXPECT_ANY_THROW(
  {
    TestWriter< TyChar > twDup;
    auto xwtDoc = twDup->StartTag( "doc" );
    xwtDoc.AddAttribute( "a", "1" );
    xwtDoc.AddAttribute( "a", "2" );
    xwtDoc.Commit();
  } );
  EXPECT_ANY_THROW(
  {
    TestWriter< TyChar > twDup;
    auto xwtDoc = twDup->StartTag( "doc" );
    xwtDoc.AddNamespace( "p", "urn:same" );
    xwtDoc.AddNamespace( "q", "urn:same" );
    xwtDoc.AddAttribute( "p:a", "1" );
    xwtDoc.AddAttribute( "q:a", "2" );
    xwtDoc.Commit();
  } );
  // A value which can't be written: nothing of the tag is.
  {
    TestWriter< TyChar > twBad;
    EXPECT_ANY_THROW(
    {
      auto xwtDoc = twBad->StartTag( "doc" );
      xwtDoc.AddAttribute( "a", "1" );
      xwtDoc.AddAttribute( "b", U"\u0001" );
      xwtDoc.Commit();
    } );
    EXPECT_EQ( twBad.StrNarrowWritten(), "" );
  }
}

// Without namespaces a name is a Name: its colons are characters of it.
TYPED_TEST( WriterTest, NoNamespaces )
{
  typedef TypeParam TyChar;
  TestWriter< TyChar > tw( false );
  {
    auto xwtDoc = tw->StartTag( "a:b:c" );
    xwtDoc.AddAttribute( ":x", "1" );
    xwtDoc.AddAttribute( "y:-1", "2" ); // After a colon a NameChar.
    EXPECT_ANY_THROW( xwtDoc.AddNamespace( "p", "urn:p" ) );
    EXPECT_ANY_THROW( tw->StartTag( "-a" ) );
    tw->WriteProcessingInstruction( "p:i", "" );
  }
  tw->EndDocument();
  EXPECT_EQ( tw.StrNarrowWritten(), "<a:b:c :x=\"1\" y:-1=\"2\"><?p:i?></a:b:c>" );
  ExpectDocument( tw.StrWritten(), false );
}

// What can't be written - each throws.
TYPED_TEST( WriterTest, Errors )
{
  typedef TypeParam TyChar;
  // Before the document element.
  {
    TestWriter< TyChar > tw;
    EXPECT_ANY_THROW( tw->WriteCharData( "text" ) ); // Only white space.
    tw->WriteCharData( " \n" );
    EXPECT_ANY_THROW( tw->WriteCDataSection( "x" ) );
    EXPECT_ANY_THROW( tw->WriteEntityReference( "amp" ) );
    EXPECT_ANY_THROW( tw->WriteCharReference( 0x41 ) );
    EXPECT_ANY_THROW( tw->EndDocument() ); // There is no document element.
    EXPECT_ANY_THROW( tw->StartTag( "1doc" ) );
    EXPECT_ANY_THROW( tw->StartTag( "a b" ) );
    EXPECT_ANY_THROW( tw->StartTag( "" ) );
    tw->WriteComment( "fine" );
    EXPECT_ANY_THROW( tw->WriteComment( "a--b" ) );
    EXPECT_ANY_THROW( tw->WriteComment( "a-" ) );
    EXPECT_ANY_THROW( tw->WriteProcessingInstruction( "xml", "" ) );
    EXPECT_ANY_THROW( tw->WriteProcessingInstruction( "XmL", "" ) );
    EXPECT_ANY_THROW( tw->WriteProcessingInstruction( "p", "a?>b" ) );
    EXPECT_ANY_THROW( tw->WriteProcessingInstruction( "p:i", "" ) ); // No colon with namespaces.
    EXPECT_ANY_THROW( tw->WriteProcessingInstruction( "", "" ) );
    tw->WriteProcessingInstruction( "xml-stylesheet", "href='a.css'" );
    {
      auto xwtDoc = tw->StartTag( "doc" );
      EXPECT_ANY_THROW( tw->WriteCharData( U"\u0001" ) ); // Not a character of XML.
      EXPECT_ANY_THROW( tw->WriteCharData( U"\U0000FFFE" ) );
      EXPECT_ANY_THROW( tw->WriteCharReference( 0x1 ) );
      EXPECT_ANY_THROW( tw->WriteCharReference( 0xD800 ) );
      EXPECT_ANY_THROW( tw->WriteCharReference( 0x110000 ) );
      EXPECT_ANY_THROW( tw->WriteEntityReference( "e" ) ); // Not declared: there is no DTD.
      EXPECT_ANY_THROW( tw->WriteEntityReference( "a:b" ) );
      EXPECT_ANY_THROW( tw->WriteDoctypeDecl( "<!DOCTYPE doc>" ) ); // Not after the first tag.
      EXPECT_ANY_THROW( tw->EndDocument() ); // A tag is open.
    }
    EXPECT_ANY_THROW( tw->StartTag( "second" ) ); // One document element.
    EXPECT_ANY_THROW( tw->WriteCharData( "text" ) ); // Only white space.
    tw->WriteCharData( "\n" );
    tw->EndDocument();
    EXPECT_ANY_THROW( tw->WriteComment( "after the end" ) );
    EXPECT_ANY_THROW( tw->WriteCharData( "\n" ) );
    EXPECT_ANY_THROW( tw->EndDocument() );
    // Nothing of what threw was written - nor was the tag which it was within, for it: nothing was written within it.
    EXPECT_EQ( tw.StrNarrowWritten(), " \n<!--fine--><?xml-stylesheet href='a.css'?><doc/>\n" );
    ExpectDocument( tw.StrWritten() );
  }
}

// The document type declaration: its text is that of one - and the entities which it declares may be referred to.
TYPED_TEST( WriterTest, DoctypeDecl )
{
  typedef TypeParam TyChar;
  {
    TestWriter< TyChar > tw;
    EXPECT_ANY_THROW( tw->WriteDoctypeDecl( "<!DOCTYPE doc [<!ELEMENT doc>]>" ) ); // Not one.
    EXPECT_ANY_THROW( tw->WriteDoctypeDecl( "<!DOCTYPE doc> " ) ); // Something after it.
    EXPECT_ANY_THROW( tw->WriteDoctypeDecl( "doc", "-//pub", "", "" ) ); // A public identifier has a system one.
    tw->WriteDoctypeDecl( "doc", "", "", "<!ENTITY e 'v'><!ENTITY q '\"'>" );
    EXPECT_ANY_THROW( tw->WriteDoctypeDecl( "<!DOCTYPE doc>" ) ); // One.
    {
      auto xwtDoc = tw->StartTag( "doc" );
      tw->WriteEntityReference( "e" );
      tw->WriteEntityReference( "q" );
      EXPECT_ANY_THROW( tw->WriteEntityReference( "f" ) ); // Every declaration was read: it isn't declared.
    }
    tw->EndDocument();
    const std::basic_string< TyChar > strWritten = tw.StrWritten();
    EXPECT_EQ( StrNarrow( strWritten ), "<!DOCTYPE doc [<!ENTITY e 'v'><!ENTITY q '\"'>]><doc>&e;&q;</doc>" );
    ReadDocument< TyChar > doc( strWritten );
    ASSERT_EQ( doc.m_strError, "" );
    ASSERT_TRUE( doc.m_cursor.FMoveDown() );
    EXPECT_EQ( StrNarrow( doc.StrContent() ), "v\"" );
  }
  // With an external subset an entity may be declared where it wasn't read.
  {
    TestWriter< TyChar > tw;
    tw->WriteDoctypeDecl( "doc", "-//test//doc", "doc.dtd", "" );
    {
      auto xwtDoc = tw->StartTag( "doc" );
      tw->WriteEntityReference( "f" );
    }
    tw->EndDocument();
    EXPECT_EQ( tw.StrNarrowWritten(), "<!DOCTYPE doc PUBLIC \"-//test//doc\" \"doc.dtd\"><doc>&f;</doc>" );
  }
  // A literal with a '"' is in '\''.
  {
    TestWriter< TyChar > tw;
    tw->WriteDoctypeDecl( "doc", "", "a\"b.dtd", "" );
    tw->WriteElement( "doc", "" );
    tw->EndDocument();
    EXPECT_EQ( tw.StrNarrowWritten(), "<!DOCTYPE doc SYSTEM 'a\"b.dtd'><doc/>" );
  }
}

// The document type declaration of a DTD which is made - in a type of character which is not the writer's.
TYPED_TEST( WriterTest, DoctypeDeclOfDtd )
{
  typedef TypeParam TyChar;
  xml_dtd< char32_t > dtd;
  dtd.Edit( []( xml_dtd< char32_t > & _rdtd )
    {
      _rdtd.SetRootName( U"doc" );
      xml_dtd_element< char32_t > element;
      element.m_strName = U"doc";
      element.m_eContent = exdcMixed;
      _rdtd.SetElementDecl( element );
      xml_dtd_entity< char32_t > entity;
      entity.m_strName = U"e";
      entity.m_strValue = U"v";
      _rdtd.SetEntityDecl( entity );
    } );
  TestWriter< TyChar > tw;
  tw->WriteDoctypeDecl( dtd );
  {
    auto xwtDoc = tw->StartTag( "doc" );
    tw->WriteEntityReference( "e" );
    EXPECT_ANY_THROW( tw->WriteEntityReference( "f" ) ); // Every declaration was read: it isn't declared.
  }
  tw->EndDocument();
  EXPECT_EQ( tw.StrNarrowWritten(), "<!DOCTYPE doc [\n<!ELEMENT doc (#PCDATA)>\n<!ENTITY e \"v\">\n]><doc>&e;</doc>" );
  ReadDocument< TyChar > doc( tw.StrWritten() );
  ASSERT_EQ( doc.m_strError, "" );
  ASSERT_TRUE( doc.m_cursor.FMoveDown() );
  EXPECT_EQ( StrNarrow( doc.StrContent() ), "v" );
  // A DTD which has no document type declaration isn't one.
  TestWriter< TyChar > twNone;
  EXPECT_ANY_THROW( twNone->WriteDoctypeDecl( xml_dtd< TyChar >() ) );
}

// Validation of what is written - see xml_writer<>::SetValidate().
static const char s_szValidateDoctype[] =
  "<!DOCTYPE doc [\n"
  "<!ELEMENT doc (a,b*)>\n"
  "<!ELEMENT a (#PCDATA)>\n"
  "<!ELEMENT b EMPTY>\n"
  "<!ATTLIST b id ID #REQUIRED ref IDREF #IMPLIED t (x|y) 'x'>\n"
  "<!ENTITY ws ' \t'>\n"
  "<!ENTITY txt 'text'>\n"
  "<!ENTITY mk '<a>m</a>'>\n"
  "]>";
// Whether the errors have one which has _pszText.
bool FHasError( std::vector< std::string > const & _rrgstrErrors, const char * _pszText )
{
  for ( std::string const & rstr : _rrgstrErrors )
    if ( std::string::npos != rstr.find( _pszText ) )
      return true;
  return false;
}
std::string StrErrors( std::vector< std::string > const & _rrgstrErrors )
{
  std::string str;
  for ( std::string const & rstr : _rrgstrErrors )
    str += rstr + "\n";
  return str;
}

TYPED_TEST( WriterTest, ValidateValid )
{
  typedef TypeParam TyChar;
  TestWriter< TyChar > tw;
  tw->SetValidate();
  tw->WriteDoctypeDecl( s_szValidateDoctype );
  {
    auto xwtDoc = tw->StartTag( "doc" );
    tw->WriteCharData( "\n  " );
    tw->WriteElement( "a", "some text" );
    tw->WriteEntityReference( "ws" ); // White space - among the elements.
    tw->WriteComment( " c " );
    {
      auto xwtB = tw->StartTag( "b" );
      xwtB.AddAttribute( "id", " i1 " ); // Normalized as an ID: "i1".
    }
    {
      auto xwtB = tw->StartTag( "b" );
      xwtB.AddAttribute( "id", "i2" );
      xwtB.AddAttribute( "ref", "i1" );
      xwtB.AddAttribute( "t", "y" );
    }
    tw->WriteCharData( "\n" );
  }
  tw->EndDocument();
  EXPECT_TRUE( tw->FIsValid() ) << StrErrors( tw->RgValidityErrors() );
  EXPECT_FALSE( tw->FValidationIncomplete() );
  EXPECT_TRUE( tw->RgValidityErrorsOfDtd().empty() );
  // What is written is as valid when it is read.
  ReadDocument< TyChar > doc( tw.StrWritten() );
  EXPECT_EQ( doc.m_strError, "" );
}

TYPED_TEST( WriterTest, ValidateNotValid )
{
  typedef TypeParam TyChar;
  TestWriter< TyChar > tw;
  tw->SetValidate();
  tw->WriteDoctypeDecl( s_szValidateDoctype );
  {
    auto xwtDoc = tw->StartTag( "doc" );
    {
      auto xwtB = tw->StartTag( "b" ); // Not where the content model has a <b>: an <a> is first.
      xwtB.AddAttribute( "ref", "nowhere" ); // An IDREF which isn't an ID - and no id, which is #REQUIRED.
      xwtB.AddAttribute( "t", "z" ); // Not one of those listed.
    }
    tw->WriteCharData( "text" ); // What is within <doc> isn't looked at once it isn't as declared - see below.
    tw->WriteEntityReference( "txt" );
    {
      auto xwtC = tw->StartTag( "c" ); // Not declared.
    }
  }
  tw->EndDocument();
  EXPECT_FALSE( tw->FIsValid() );
  std::vector< std::string > const & rrgstrErrors = tw->RgValidityErrors();
  EXPECT_TRUE( FHasError( rrgstrErrors, "<b>" ) ) << StrErrors( rrgstrErrors );
  EXPECT_TRUE( FHasError( rrgstrErrors, "nowhere" ) ) << StrErrors( rrgstrErrors );
  EXPECT_TRUE( FHasError( rrgstrErrors, "[id]" ) ) << StrErrors( rrgstrErrors );
  EXPECT_TRUE( FHasError( rrgstrErrors, "[z]" ) ) << StrErrors( rrgstrErrors );
  EXPECT_TRUE( FHasError( rrgstrErrors, "<c>" ) ) << StrErrors( rrgstrErrors );
  EXPECT_EQ( rrgstrErrors.size(), 5u ) << StrErrors( rrgstrErrors );
  // It was written all the same.
  EXPECT_EQ( tw.StrNarrowWritten(), std::string( s_szValidateDoctype ) + "<doc><b ref=\"nowhere\" t=\"z\"/>text&txt;<c/></doc>" );
  // Character data in element content - from a reference as well; and white space in an element declared EMPTY.
  for ( int nCase = 0; nCase < 3; ++nCase )
  {
    TestWriter< TyChar > twCharData;
    twCharData->SetValidate();
    twCharData->WriteDoctypeDecl( s_szValidateDoctype );
    {
      auto xwtDoc = twCharData->StartTag( "doc" );
      twCharData->WriteElement( "a", "" );
      if ( 0 == nCase )
        twCharData->WriteCharData( " text " );
      else
      if ( 1 == nCase )
        twCharData->WriteEntityReference( "txt" );
      else
      {
        auto xwtB = twCharData->StartTag( "b" );
        xwtB.AddAttribute( "id", "i" );
        twCharData->WriteCharData( " " );
      }
    }
    twCharData->EndDocument();
    EXPECT_EQ( twCharData->RgValidityErrors().size(), 1u ) << nCase << ": " << StrErrors( twCharData->RgValidityErrors() );
    EXPECT_TRUE( FHasError( twCharData->RgValidityErrors(), ( 2 == nCase ) ? "EMPTY" : "character data" ) ) << nCase << ": " << StrErrors( twCharData->RgValidityErrors() );
  }
}

TYPED_TEST( WriterTest, ValidateWhatIsNotKnown )
{
  typedef TypeParam TyChar;
  // An entity with markup: what is within the element isn't known from there on.
  {
    TestWriter< TyChar > tw;
    tw->SetValidate();
    tw->WriteDoctypeDecl( s_szValidateDoctype );
    {
      auto xwtDoc = tw->StartTag( "doc" );
      tw->WriteEntityReference( "mk" );
    }
    tw->EndDocument();
    EXPECT_TRUE( tw->FValidationIncomplete() );
    EXPECT_TRUE( tw->FIsValid() ) << StrErrors( tw->RgValidityErrors() ); // Nothing which was known violates.
  }
  // An external subset which isn't read: not all of the declarations are known.
  {
    TestWriter< TyChar > tw;
    tw->SetValidate();
    tw->WriteDoctypeDecl( "<!DOCTYPE doc SYSTEM 'doc.dtd' [<!ELEMENT doc EMPTY>]>" );
    tw->WriteElement( "doc", "" );
    tw->EndDocument();
    EXPECT_TRUE( tw->FValidationIncomplete() );
    EXPECT_TRUE( tw->FIsValid() ) << StrErrors( tw->RgValidityErrors() );
  }
  // No document type declaration: there is nothing to be valid against.
  {
    TestWriter< TyChar > tw;
    tw->SetValidate();
    tw->WriteElement( "doc", "" );
    tw->EndDocument();
    EXPECT_FALSE( tw->FIsValid() );
    EXPECT_EQ( tw->RgValidityErrors().size(), 1u ) << StrErrors( tw->RgValidityErrors() );
  }
  // Without validation nothing is said.
  {
    TestWriter< TyChar > tw;
    tw->WriteDoctypeDecl( s_szValidateDoctype );
    tw->WriteElement( "doc", "" );
    tw->EndDocument();
    EXPECT_FALSE( tw->FIsValid() );
    EXPECT_TRUE( tw->RgValidityErrors().empty() );
    // And it is too late to set it.
    EXPECT_ANY_THROW( tw->SetValidate() );
  }
}

// Validity errors thrown - when the tag is committed.
TYPED_TEST( WriterTest, ValidateThrows )
{
  typedef TypeParam TyChar;
  TestWriter< TyChar > tw;
  tw->SetValidate();
  tw->SetThrowOnValidityError();
  tw->WriteDoctypeDecl( s_szValidateDoctype );
  // ( The tag of the document is ended as the exception passes: what that throws is passed over. )
  EXPECT_ANY_THROW(
    {
      auto xwtDoc = tw->StartTag( "doc" );
      tw->WriteElement( "c", "" );
    } );
  EXPECT_TRUE( tw->RgValidityErrors().empty() ); // Thrown and not recorded.
}

// The DTD of an xml_dtd - in another type of character - and namespaces: names are as they are written.
TYPED_TEST( WriterTest, ValidateWithDtd )
{
  typedef TypeParam TyChar;
  xml_dtd< char32_t > dtd;
  dtd.SetRootName( U"p:doc" );
  {
    xml_dtd_element< char32_t > element;
    element.m_strName = U"p:doc";
    element.m_eContent = exdcEmpty;
    dtd.SetElementDecl( element );
    xml_dtd_attdef< char32_t > attdef;
    attdef.m_strName = U"xmlns:p";
    attdef.m_eDefault = exdadImplied;
    dtd.SetAttDef( U"p:doc", attdef );
    attdef.m_strName = U"p:n";
    attdef.m_eType = exdatNMTOKENS;
    dtd.SetAttDef( U"p:doc", attdef );
  }
  TestWriter< TyChar > tw;
  tw->SetValidate();
  tw->WriteDoctypeDecl( dtd );
  {
    auto xwtDoc = tw->StartTag( "doc", "p", "urn:p" );
    xwtDoc.AddAttribute( "p:n", "  a  b " );
  }
  tw->EndDocument();
  EXPECT_TRUE( tw->FIsValid() ) << StrErrors( tw->RgValidityErrors() );
  EXPECT_EQ( tw.StrNarrowWritten(), "<!DOCTYPE p:doc [\n<!ELEMENT p:doc EMPTY>\n<!ATTLIST p:doc xmlns:p CDATA #IMPLIED>\n<!ATTLIST p:doc p:n NMTOKENS #IMPLIED>\n]><p:doc xmlns:p=\"urn:p\" p:n=\"  a  b \"/>" );
}

// Pretty printing - see xml_writer<>::SetOutputFormat().
_xml_output_format XofPretty( int8_t _byNSpacesPerIndent = 2, size_t _nMaxIndentLevels = 32 )
{
  _xml_output_format xof;
  xof.m_fNoFormatting = false;
  xof.m_byNSpacesPerIndent = _byNSpacesPerIndent;
  xof.m_nMaximumIndentLevelForWhitespace = _nMaxIndentLevels;
  return xof;
}
template < class t_TyChar >
std::basic_string< t_TyChar > StrWiden( const char * _psz )
{
  return std::basic_string< t_TyChar >( _psz, _psz + strlen( _psz ) );
}
// The document read and written pretty printed - as it was read: its XML declaration and its document type declaration.
template < class t_TyChar >
std::basic_string< t_TyChar > StrPrettyPrinted( std::basic_string< t_TyChar > const & _rstrText, _xml_output_format const & _rxof )
{
  ReadDocument< t_TyChar > doc( _rstrText );
  VerifyThrowSz( doc.m_strError.empty(), "%s", doc.m_strError.c_str() );
  TyTestXmlWriter< t_TyChar > xw;
  xw.SetWriteBOM( false );
  xw.SetWriteXMLDecl( !doc.m_cursor.FPseudoXMLDecl() );
  xw.SetUseNamespaces( doc.m_cursor.FUseXMLNamespaces() );
  xw.SetOutputFormat( _rxof );
  typename TyTestXmlWriter< t_TyChar >::_TyXMLDeclProperties xdp( doc.m_cursor.GetXMLDeclProperties() );
  xw.OpenMemFile( xdp, nullptr, !xdp.m_fHasEncoding );
  VerifyThrow( xw.FPrettyPrint() );
  xw.NWriteFromReadCursor( doc.m_cursor );
  return StrWrittenBy< t_TyChar >( xw );
}

// A document which is made: each thing on a line of its own - but within an element which has text, and where
//  xml:space="preserve" is in scope.
TYPED_TEST( WriterTest, PrettyPrintMade )
{
  typedef TypeParam TyChar;
  const _xml_output_format xof = XofPretty();
  TestWriter< TyChar > tw( true, false, &xof );
  ASSERT_TRUE( tw->FPrettyPrint() );
  tw->WriteComment( "c" );
  {
    auto xwtDoc = tw->StartTag( "doc" );
    xwtDoc.AddAttribute( "a", "1" );
    {
      auto xwtItem = tw->StartTag( "item" );
      tw->WriteElement( "name", "x" );
      tw->StartTag( "empty" ).EndTag();
    }
    tw->WriteProcessingInstruction( "pi", "" );
    {
      auto xwtMixed = tw->StartTag( "mixed" );
      tw->WriteCharData( "text " );
      {
        auto xwtB = tw->StartTag( "b" );
        tw->StartTag( "c" ).EndTag();
      }
      tw->WriteCharData( " more" );
    }
    {
      auto xwtPre = tw->StartTag( "pre" );
      xwtPre.AddAttribute( "xml:space", "preserve" );
      tw->StartTag( "q" ).EndTag();
    }
    tw->WriteCharData( "\n   \n" ); // White space where the indentation is: it isn't written.
  }
  tw->WriteComment( "after" );
  tw->EndDocument();
  const std::basic_string< TyChar > strWritten = tw.StrWritten();
  EXPECT_EQ( StrNarrow( strWritten ),
    "<!--c-->\n"
    "<doc a=\"1\">\n"
    "  <item>\n"
    "    <name>x</name>\n"
    "    <empty/>\n"
    "  </item>\n"
    "  <?pi?>\n"
    "  <mixed>text <b><c/></b> more</mixed>\n"
    "  <pre xml:space=\"preserve\"><q/></pre>\n"
    "</doc>\n"
    "<!--after-->\n" );
  ExpectDocument( strWritten );
}

// A document which is read, pretty printed: the white space between its markup is the indentation, the white space
//  within its tags is a single space - and the rest is as it was.
TYPED_TEST( WriterTest, PrettyPrintRead )
{
  typedef TypeParam TyChar;
  const std::basic_string< TyChar > strText = StrWiden< TyChar >(
    "<?xml version=\"1.0\"?>\n"
    "<!DOCTYPE doc [<!ENTITY e 'v'>]>\n"
    "<!-- before -->"
    "<doc   a = 'x'\n  b='y'   ><item>  <sub/>\n</item  ><t>text &e; here</t>"
    "<pre xml:space='preserve'>  <k/>  </pre><c><![CDATA[cd]]></c><!-- c --><?pi data?>\n\n</doc>\n" );
  std::basic_string< TyChar > strPretty;
  ASSERT_NO_THROW( strPretty = StrPrettyPrinted( strText, XofPretty() ) );
  EXPECT_EQ( StrNarrow( strPretty ),
    "<?xml version=\"1.0\"?>\n"
    "<!DOCTYPE doc [<!ENTITY e 'v'>]>\n"
    "<!-- before -->\n"
    "<doc a='x' b='y'>\n"
    "  <item>\n"
    "    <sub/>\n"
    "  </item>\n"
    "  <t>text &e; here</t>\n"
    "  <pre xml:space='preserve'>  <k/>  </pre>\n"
    "  <c><![CDATA[cd]]></c>\n"
    "  <!-- c -->\n"
    "  <?pi data?>\n"
    "</doc>\n" );
  ExpectDocument( strPretty );
  // Pretty printed again it is the same.
  std::basic_string< TyChar > strPrettyAgain;
  ASSERT_NO_THROW( strPrettyAgain = StrPrettyPrinted( strPretty, XofPretty() ) );
  EXPECT_EQ( StrNarrow( strPrettyAgain ), StrNarrow( strPretty ) );
}

// The indentation: tabs, no indentation - only the lines - and a depth beyond which it is no deeper.
TYPED_TEST( WriterTest, PrettyPrintIndent )
{
  typedef TypeParam TyChar;
  auto StrWrite = []( _xml_output_format const & _rxof ) -> std::string
  {
    TestWriter< TyChar > tw( true, false, &_rxof );
    {
      auto xwtA = tw->StartTag( "a" );
      auto xwtB = tw->StartTag( "b" );
      tw->StartTag( "c" ).EndTag();
    }
    tw->EndDocument();
    return tw.StrNarrowWritten();
  };
  EXPECT_EQ( StrWrite( XofPretty( -1 ) ), "<a>\n\t<b>\n\t\t<c/>\n\t</b>\n</a>\n" );
  EXPECT_EQ( StrWrite( XofPretty( 0 ) ), "<a>\n<b>\n<c/>\n</b>\n</a>\n" );
  EXPECT_EQ( StrWrite( XofPretty( 3 ) ), "<a>\n   <b>\n      <c/>\n   </b>\n</a>\n" );
  EXPECT_EQ( StrWrite( XofPretty( 2, 1 ) ), "<a>\n  <b>\n  <c/>\n  </b>\n</a>\n" );
  // Without formatting nothing is added.
  EXPECT_EQ( StrWrite( _xml_output_format() ), "<a><b><c/></b></a>" );
}

} // namespace ns_XMLGTestWriter

int main( int argc, char ** argv )
{
  g_strProgramName = argv[0];
  testing::InitGoogleTest( &argc, argv );
  return RUN_ALL_TESTS();
}
