#include "Sorter.h"
#include "Memory.h"
#include <algorithm>
using namespace std;
using namespace boost;
using namespace filesystem;

CSorter::CSorter( char *filePath ) : m_filePath(filePath)
{
  m_valuesReadCount = 0;
//   try
//   {
//     a.resize(a.max_size()/sizeof(unsigned int) - 10);
//   }
//   catch (std::bad_alloc& ba)
//   {
//   	printf("Error");
//   }
//   catch (...)
//   {
//     printf("something gone really bad");
//   }
}

CSorter::~CSorter(void)
{
}

size_t CSorter::GetFileSize()
{
  return file_size(m_filePath);
}

void CSorter::GetFreeMemorySize()
{
  size_t a, b;
  a = getTotalSystemMemory();
  b = FindLargestSizeOfMemory();
}

void CSorter::Process()
{
  vector<unsigned int> buf;
  buf.reserve(GetFileChunkSize());
  m_pFile = fopen(m_filePath.string().data(), "rb");

  while(GetValuesLeftToRead() > 0)
  {
    buf.resize(GetFileChunkSize());
    ReadIntoVector(buf);
    sort(buf.begin(), buf.end());
    SaveVectorToFile(buf);
  }
  fclose(m_pFile);

  MergeAllFiles();
}

size_t CSorter::GetFileChunkSize()
{    
  return min(100000, GetValuesLeftToRead());
}

size_t CSorter::GetFileValuesCount()
{
  return GetFileSize() / sizeof(unsigned int);
}

void CSorter::ReadIntoVector( std::vector<unsigned int> &buffer )
{
  int i = 0;
  for(i = 0; !feof(m_pFile) && i != buffer.size(); i++)
  {
    fread(&buffer[i], sizeof(unsigned int), 1, m_pFile);
    m_valuesReadCount++;
  }
}

unsigned int ReadOneInt(FILE *pFile)
{
  unsigned int temp;
  fread(&temp, sizeof(unsigned int), 1, pFile);
  return temp;
}

void CSorter::MergeTwoSortedFiles( boost::filesystem::path file1, boost::filesystem::path file2, boost::filesystem::path resultFile )
{
  unsigned int temp1, temp2, resultTemp;
  bool readFrom1, readFrom2;
  size_t readDataSize1, readDataSize2;
  FILE *pFile1, *pFile2, *pResultFile;
  pFile1 = fopen(file1.string().data(), "rb");
  pFile2 = fopen(file2.string().data(), "rb");
  pResultFile = fopen(resultFile.string().data(), "wb");

  readFrom1 = readFrom2 = true;
  readDataSize1 = readDataSize2 = 0;
  while (true)
  {
    if (readFrom1)
    {
      temp1 = ReadOneInt(pFile1);
      if (feof(pFile1))
        break;
      readDataSize1 += sizeof(unsigned int);
    }
    if (readFrom2)
    {
      temp2 = ReadOneInt(pFile2);
      if (feof(pFile2))
        break;
      readDataSize2 += sizeof(unsigned int);
    }
    readFrom1 = readFrom2 = false;

    if (temp1 <= temp2)
    {
      resultTemp = temp1;
      readFrom1 = true;
    }
    else
    {
      resultTemp = temp2;
      readFrom2 = true;
    }
    fwrite(&resultTemp, sizeof(unsigned int), 1, pResultFile);
  }

  if (!feof(pFile1))
  {
    size_t dataLeftToRead = filesystem::file_size(file1) - readDataSize1;
    while(!feof(pFile1))
    {
      fwrite(&temp1, sizeof(unsigned int), 1, pResultFile); //temp1 already contains value that has been read but not written
      temp1 = ReadOneInt(pFile1);
    }
  }
  else // File2 left some values
  {
    size_t dataLeftToRead = filesystem::file_size(file2) - readDataSize2;
    while(!feof(pFile2))
    {
      fwrite(&temp2, sizeof(unsigned int), 1, pResultFile); //temp2 already contains value that has been read but not written
      temp2 = ReadOneInt(pFile2);
    }
  }
  fclose(pFile1);
  fclose(pFile2);
  fclose(pResultFile);
}

void CSorter::MergeTwoSortedVectors( std::vector<unsigned int> &buf1, std::vector<unsigned int> &buf2, boost::filesystem::path resultFile )
{
  size_t i_1, i_2;
  FILE *pResultFile;
  pResultFile = fopen(resultFile.string().data(), "wb");
  for (i_1 = i_2 = 0; i_1 < buf1.size() && i_2 < buf2.size();)
  {
    if(buf1[i_1] <= buf2[i_2]) 
    {
      fwrite(&buf1[i_1], sizeof(unsigned int), 1, pResultFile);
      i_1++;
    }
    else 
    {
      fwrite(&buf2[i_2], sizeof(unsigned int), 1, pResultFile);
      i_2++;
    }
  }
  if (i_1 < buf1.size())
  {
    fwrite(&buf1[i_1], sizeof(unsigned int), buf1.size() - i_1, pResultFile);
  }
  else
  {
    fwrite(&buf2[i_2], sizeof(unsigned int), buf2.size() - i_2, pResultFile);
  }
  fclose(pResultFile);
}

size_t CSorter::GetValuesLeftToRead()
{
  return GetFileValuesCount() - m_valuesReadCount;
}

void CSorter::SaveVectorToFile( std::vector<unsigned int> &buffer )
{
  path filePath = GetNextTempFileName().data();
  FILE *pTempFile;
  pTempFile = fopen(filePath.string().data(), "wb");
  fwrite(&buffer[0], sizeof(unsigned int), buffer.size(), pTempFile);
  fclose(pTempFile);
  m_sortedFileChunks.push_back(filePath);
}

std::string CSorter::GetNextTempFileName()
{
  char buf[100];
  static int i = 0;
  sprintf(buf, "temp%d", i);
  i++;
  string filename = buf;
  return filename;
}

void CSorter::MergeAllFiles()
{
  int i_1, i_2;
  vector<path> temp;

  while(m_sortedFileChunks.size() != 1)
  {
    temp.clear();
    for (i_1 = 0, i_2 = 1; i_2 < m_sortedFileChunks.size(); i_1+=2, i_2+=2)
    {
      path newFile = GetNextTempFileName();
      MergeTwoSortedFiles(m_sortedFileChunks[i_1], m_sortedFileChunks[i_2], newFile);
      temp.push_back(newFile);
    }
    if (i_1 < m_sortedFileChunks.size())
      temp.push_back(m_sortedFileChunks[i_1]);
    m_sortedFileChunks = temp;
  }
}
