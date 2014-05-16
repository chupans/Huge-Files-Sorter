#include "Sorter.h"
#include "Memory.h"
#include <algorithm>
#include <boost/foreach.hpp>
using namespace std;
using namespace boost;
using namespace filesystem;

CSorter::CSorter( char *filePath ) : m_filePath(filePath)
{
  m_threadGroup = NULL;
  m_valuesReadCount = 0;
  auto alloc = allocator<unsigned int>();
  m_chunkValuesCount = min(150000000, GetFileValuesCount());
  m_chunkValuesCount /= GetCoresCount();
  CreateBuffers();
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
  BOOST_FOREACH(auto pVector, m_buffers)
  {
    pVector->~vector();
  }
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
    BOOST_FOREACH(auto pVector, m_buffers)
    {
      pVector->resize(GetFileChunkSize());
      ReadIntoVector(*pVector);
    }
    PrepareThreads();
    printf("Threads prepared. Now joining\n");
    m_threadGroup->join_all();
    BOOST_FOREACH(auto pVector, m_buffers)
    {
      SaveVectorToFile(*pVector);
    }
  }
  fclose(m_pFile);

  path newFile = GetNextTempFileName();
  MergeSortedFiles(newFile);
}

size_t CSorter::GetFileChunkSize()
{    
  return min(m_chunkValuesCount, GetValuesLeftToRead());
}

size_t CSorter::GetFileValuesCount()
{
  return GetFileSize() / sizeof(unsigned int);
}

void CSorter::ReadIntoVector( std::vector<unsigned int> &buffer )
{
  size_t count = buffer.size();
  if (count == 0)
    return;
  m_valuesReadCount += fread(&buffer[0], sizeof(unsigned int), count, m_pFile);
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

void CSorter::PrepareThreads()
{
  if (m_threadGroup != NULL)
  {
    m_threadGroup->~thread_group();
  }
  m_threadGroup = new boost::thread_group();
  BOOST_FOREACH(auto pVector, m_buffers)
  {
    m_threadGroup->create_thread(boost::bind(&CSorter::SortSingleBuffer, pVector));
  }
}

void CSorter::CreateBuffers()
{
  vector<unsigned int> *pVector;
  auto nCores = GetCoresCount();
  for (int i = 0; i < nCores; i++)
  {
    pVector = new vector<unsigned int>();
    pVector->reserve(m_chunkValuesCount);
    m_buffers.push_back(pVector);
  }
}

unsigned int CSorter::GetCoresCount()
{
  return thread::hardware_concurrency();
}

//Wrapper to use in threads. To make sure that iterators are valid
void CSorter::SortSingleBuffer( std::vector<unsigned int> *buf )
{
  printf("started sorting\n");
  sort(buf->begin(), buf->end());
}

struct FileToReadFrom
{
public:
  FileToReadFrom(const char *filePath) { pFile = fopen(filePath, "rb"); readFrom = true; ended = false;};
  ~FileToReadFrom() { fclose(pFile);};
  void ReadNextVal() 
  {
    if (readFrom)
    {
      fread(&val, sizeof(unsigned int), 1, pFile); 
      if (feof(pFile)) 
        ended = true; 
      readFrom = false; 
    }
  };
  FILE *pFile;
  bool readFrom;
  bool ended;
  unsigned int val;
};

void CSorter::MergeSortedFiles( boost::filesystem::path resultFile )
{
  bool continueMerge = true;
  unsigned int resultTemp;
  FILE *pResultFile;
  auto files = list<FileToReadFrom*>();
  list<FileToReadFrom*>::iterator fIt;

  BOOST_FOREACH(auto filePath, m_sortedFileChunks)
  {
    files.push_back(new FileToReadFrom(filePath.string().data()));
  }
  pResultFile = fopen(resultFile.string().data(), "wb");

  while (continueMerge)
  {
    for (fIt = files.begin(); fIt != files.end();)
    {
      (*fIt)->ReadNextVal();

      if ((*fIt)->ended)
      {
        (*fIt)->~FileToReadFrom();
        fIt = files.erase(fIt);
      }
      else
        fIt++;
    }

    if (files.size() == 0)
      continueMerge = false;
    else
    {
      fIt = min_element(files.begin(), files.end(), [](FileToReadFrom* l, FileToReadFrom* r) { return l->val < r->val; });

      resultTemp = (*fIt)->val;
      (*fIt)->readFrom = true;

      fwrite(&resultTemp, sizeof(unsigned int), 1, pResultFile);
    }
  }

  fclose(pResultFile);
  BOOST_FOREACH(auto pFile, files)
  {
    pFile->~FileToReadFrom();
  }
}
