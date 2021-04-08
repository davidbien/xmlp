#pragma once

// xmlpgtest.h
// Header for shared xmlp gtest stuff.
// dbien
// 06APR2021

extern std::string g_strProgramName;

namespace ns_XMLGTest
{
__XMLP_USING_NAMESPACE

class XmlpTestEnvironment : public testing::Environment
{
  typedef XmlpTestEnvironment _TyThis;
  typedef testing::Environment _TyBase;
public:
  typedef std::pair< EFileCharacterEncoding, bool > _TyKeyEncodingBOM;
  typedef map< _TyKeyEncodingBOM, string > _TyMapTestFiles;
  explicit XmlpTestEnvironment( const char * _pszFileName )
    : m_strFileNameOrig( _pszFileName )
  {
  }
protected:
  void SetUp() override 
  {
    // 0) Check if the caller used a slash that doesn't match the OS slash and substitute.
    // 1) Open the file.
    // 2) Figure out the encoding of the file. Also check if it is a failure scenario.
    // 3) Create all potential encodings of the file - i.e. UTF32, UTF16, and UTF8 big and little endian.
    //  a) Write each encoding with and without a BOM.
    // 4) Write all encodings in a unittest directory in the output.
    // 5) List all file names in this environment object in m_rgFileNamesTestDir.
    using namespace filesystem;
    path pathFileOrig( m_strFileNameOrig );
    VerifyThrowSz( exists( pathFileOrig ), "Original test file[%s] doesn't exist.", m_strFileNameOrig.c_str() );
    VerifyThrowSz( pathFileOrig.has_parent_path(), "Need full path to input file[%s] - couldn't find parent path.", m_strFileNameOrig.c_str() );
    VerifyThrowSz( pathFileOrig.has_extension() && ( pathFileOrig.extension() == ".xml" ), "File doesn't have an '.xml' extention [%s].", m_strFileNameOrig.c_str() );
    VerifyThrowSz( pathFileOrig.has_stem(), "No base filename in[%s].", m_strFileNameOrig.c_str() );
    path pathBaseFile( "unittests" );
    m_pathStemOrig = pathFileOrig.stem();
    pathBaseFile /= m_pathStemOrig;
    FileObj fo( OpenReadOnlyFile( m_strFileNameOrig.c_str() ) );
    VerifyThrowSz( fo.FIsOpen(), "Couldn't open file [%s]", m_strFileNameOrig.c_str() );
    uint8_t rgbyBOM[vknBytesBOM];
    size_t nbyLenghtBOM;
    int iResult = FileRead( fo.HFileGet(), rgbyBOM, vknBytesBOM, &nbyLenghtBOM );
    Assert( !iResult );
    Assert( nbyLenghtBOM == vknBytesBOM ); // The smallest valid xml file is 4 bytes... "<t/>".
    VerifyThrowSz( nbyLenghtBOM == vknBytesBOM, "Unable to read [%lu] bytes from the file[%s].", vknBytesBOM, m_strFileNameOrig.c_str() );
    m_fExpectFailure = ( string::npos != m_strFileNameOrig.find("FAIL") ); // simple method for detecting expected failures.
    EFileCharacterEncoding efceEncoding = efceFileCharacterEncodingCount;
    if ( !iResult && ( nbyLenghtBOM == vknBytesBOM ) )
      efceEncoding = GetCharacterEncodingFromBOM( rgbyBOM, nbyLenghtBOM );
    bool fEncodingFromBOM = ( efceFileCharacterEncodingCount != efceEncoding );
    if ( !fEncodingFromBOM )
    {
      // Since we know we are opening XML we can use an easy heuristic to determine the encoding:
      efceEncoding = DetectEncodingXmlFile( rgbyBOM, vknBytesBOM );
      Assert( efceFileCharacterEncodingCount != efceEncoding ); // Unless the source isn't XML the above should succeed.
      VerifyThrowSz( efceFileCharacterEncodingCount != efceEncoding, "Unable to sus encoding for the file[%s].", m_strFileNameOrig.c_str() );
      nbyLenghtBOM = 0;
    }
    
    size_t grfNeedFiles = ( 1 << ( 2 * efceFileCharacterEncodingCount ) ) - 1;
    m_grfFileOrig = ( ( 1 << efceEncoding ) << ( size_t(fEncodingFromBOM) * efceFileCharacterEncodingCount ) );
    grfNeedFiles &= ~m_grfFileOrig;
    while( grfNeedFiles )
    {
      // This isn't as efficient as it could be but it feng shuis...
      size_t stGenerate = _bv_get_clear_first_set( grfNeedFiles );
      bool fWithBOM = stGenerate >= efceFileCharacterEncodingCount;
      EFileCharacterEncoding efceGenerate = EFileCharacterEncoding( stGenerate % efceFileCharacterEncodingCount );
      // Name the new file with the base of the original XML file:
      path pathConvertedFile( pathBaseFile );
      pathConvertedFile += "_";
      pathConvertedFile += PszCharacterEncodingShort( efceGenerate );
      if ( fWithBOM )
        pathConvertedFile += "BOM";
      pathConvertedFile += ".xml";
      (void)NFileSeekAndThrow( fo.HFileGet(), nbyLenghtBOM, vkSeekBegin );
      ConvertFileMapped( fo.HFileGet(), efceEncoding, pathConvertedFile.string().c_str(), efceGenerate, fWithBOM );
      VerifyThrow( m_mapFileNamesTestDir.insert( _TyMapTestFiles::value_type( _TyKeyEncodingBOM( efceGenerate, fWithBOM ), pathConvertedFile.string() ) ).second );
    }
    // Now copy the original to the output directory - don't modify the name so we know it's the original.
    path pathConvertedFile( pathBaseFile );
    pathConvertedFile += ".xml";
    ConvertFileMapped( fo.HFileGet(), efceEncoding, pathConvertedFile.string().c_str(), efceEncoding, fEncodingFromBOM );
    VerifyThrow( m_mapFileNamesTestDir.insert( _TyMapTestFiles::value_type( _TyKeyEncodingBOM( efceEncoding, fEncodingFromBOM ), pathConvertedFile.string() ) ).second );

    // Create a directory for the base file - sans extension - this is where the output files for the given unit test will go.
    std::error_code ec;
    (void)create_directory( pathBaseFile, ec );
    VerifyThrowSz( !ec, "Unable to create unittest output directory [%s] ec.message()[%s].", pathBaseFile.string().c_str(), ec.message().c_str() );
    m_pathBaseFile = std::move( pathBaseFile );
  }

  // TearDown() is invoked immediately after a test finishes.
  void TearDown() override 
  {
    // Nothing to do in TearDown() - we want to leave the generated unit test files so that they can be analyzed if there are any issues.
  }
public:
  filesystem::path PathCreateUnitTestOutputDirectory() const
  {
    using namespace filesystem;
    const ::testing::TestInfo* const test_info =
    ::testing::UnitTest::GetInstance()->current_test_info();
    path pathSuite = m_pathBaseFile;
    pathSuite /= path( g_strProgramName ).stem(); // since we have multiple unit test files now.
    pathSuite /= test_info->test_suite_name();
    path pathTest = test_info->name();
    path::iterator itTestNum = --pathTest.end();
    pathSuite += "_";
    pathSuite += itTestNum->c_str();
    std::error_code ec;
    (void)create_directories( pathSuite, ec );
    VerifyThrowSz( !ec, "Unable to create unittest output directory[%s] ec.message()[%s].", pathSuite.string().c_str(), ec.message().c_str() );
    return pathSuite;
  }
  // Return the filename for the output file for the given bit.
  filesystem::path GetNextFileNameOutput( filesystem::path const & _rpathOutputDir, size_t & _rgrfBitOutput, const _TyMapTestFiles::value_type *& _rpvtGoldenFile ) const
  {
    filesystem::path pathOutputFile( _rpathOutputDir );
    size_t stGenerate = _bv_get_clear_first_set( _rgrfBitOutput );
    bool fWithBOM = stGenerate >= efceFileCharacterEncodingCount;
    EFileCharacterEncoding efceEncoding = EFileCharacterEncoding( stGenerate % efceFileCharacterEncodingCount );
    _TyMapTestFiles::const_iterator citFile = m_mapFileNamesTestDir.find( _TyKeyEncodingBOM(efceEncoding, fWithBOM ) );
    _rpvtGoldenFile = &*citFile;
    VerifyThrow( m_mapFileNamesTestDir.end() != citFile ); // unexpected.
    pathOutputFile /= m_pathStemOrig;
    if ( ( 1ull << stGenerate ) != m_grfFileOrig )
    {
      pathOutputFile += "_";
      pathOutputFile += PszCharacterEncodingShort( efceEncoding );
      if ( fWithBOM )
        pathOutputFile += "BOM";
    }
    pathOutputFile += ".xml";
    return pathOutputFile;
  }
  void CompareFiles( filesystem::path const & _rpathOutputFile, const _TyMapTestFiles::value_type * _pvtGoldenFile )
  {
    // Map both files and compare the memory - should match byte for byte. If unit tests have extra whitespace in markup (i.e. between attribute declarations)
    //  then the files may not match. It's important to structure the unit tests so that they don't have extra markup whitespace. All non-markup whitespace
    //  should be duplicated correctly - i.e CHARDATA in between tags and other elements.
    size_t nbyOutput, nbyGolden;
    FileMappingObj fmoOutput( _FmoOpen( _rpathOutputFile, nbyOutput ) );
    FileMappingObj fmoGolden( _FmoOpen( _pvtGoldenFile->second, nbyGolden ) );
    // Move through even if the files don't match in size to find the first byte where they don't match for diag purposes.
    // Can't map zero size files so we know we don't have any...
    const uint8_t * pbyOutputCur = fmoOutput.Pby();
    const uint8_t * const pbyOutputEnd = pbyOutputCur + nbyOutput;
    const uint8_t * pbyGoldenCur = fmoGolden.Pby();
    const uint8_t * const pbyGoldenEnd = pbyGoldenCur + nbyGolden;
    for ( ; ( pbyOutputEnd != pbyOutputCur ) && ( pbyGoldenEnd != pbyGoldenCur ); ++pbyOutputCur, ++pbyGoldenCur )
    {
      Assert( *pbyOutputCur == *pbyGoldenCur );
      VerifyThrowSz( *pbyOutputCur == *pbyGoldenCur, "Mismatch at byte number [%lu] outputfile[%s] goldenfile[%s].", ( pbyOutputCur - fmoOutput.Pby() ), 
         _rpathOutputFile.string().c_str(), _pvtGoldenFile->second.c_str() );
    }
  }
  FileMappingObj _FmoOpen( filesystem::path const & _rpathFile, size_t & _rnbySize )
  {
    FileObj fo( OpenReadOnlyFile( _rpathFile.string().c_str() ) );
    VerifyThrowSz( fo.FIsOpen(), "Unable to open file [%s].", _rpathFile.string().c_str() );
    FileMappingObj fmo( MapReadOnlyHandle( fo.HFileGet(), &_rnbySize ) );
    VerifyThrowSz( fmo.FIsOpen(), "Couldn't map file [%s]", _rpathFile.string().c_str() );
    return fmo;
  }
   
  string m_strFileNameOrig;
  filesystem::path m_pathStemOrig;
  size_t m_grfFileOrig{0}; // The bit for the original file so we can match the name in the output.
  filesystem::path m_pathBaseFile;
  bool m_fExpectFailure{false}; // If this is true then we expect the test to fail.
  _TyMapTestFiles m_mapFileNamesTestDir; // The resultant files that were written to the test directory in the output.
};

inline void MapFileForMemoryTest( const char * _pszFileName, FileMappingObj & _rfmo, size_t & _nbyBytesFile )
{
  FileObj fo( OpenReadOnlyFile( _pszFileName ) );
  VerifyThrowSz( fo.FIsOpen(), "Couldn't open file [%s]", _pszFileName );
  FileMappingObj fmo( MapReadOnlyHandle( fo.HFileGet(), &_nbyBytesFile ) );
  VerifyThrowSz( fmo.FIsOpen(), "Couldn't map file [%s]", _pszFileName );
  _rfmo = std::move( fmo );
}

} // namespace ns_XMLGTest
