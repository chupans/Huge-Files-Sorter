#include "Sorter.h"
#include "Memory.h"
#include <algorithm>
#include <boost/foreach.hpp>
#include "Utility.h"

using namespace std;
using namespace boost;
using namespace filesystem;

CSorter::CSorter( char *filePath ) : m_filePath(filePath)
{
  m_threadGroup = NULL;
  m_valuesReadCount = 0;
  auto freeMemorySize = Utility::GetFreeSystemMemory();
  m_chunkValuesCount = min((boost::uintmax_t)freeMemorySize * 0.8f, GetFileValuesCount());
  m_chunkValuesCount /= GetCoresCount();
  CreateBuffers();
}

CSorter::~CSorter(void)
{
  delete m_threadGroup;
}

void CSorter::SortAndSaveTo( char* filePath )
{
  m_file.OpenForReading(m_filePath);

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
    printf("Saving\n");
    BOOST_FOREACH(auto pVector, m_buffers)
    {
      SaveVectorToFile(*pVector);
    }
  }
  FreeBuffers();
  m_file.CloseFile();
  printf("Merging\n");
  path resultFile = filePath;
  MergeSortedFiles(resultFile);
}

uintmax_t CSorter::GetFileChunkSize()
{    
  return min(m_chunkValuesCount, GetValuesLeftToRead());
}

uintmax_t CSorter::GetFileValuesCount()
{
  return file_size(m_filePath) / sizeof(unsigned int);
}

void CSorter::ReadIntoVector( std::vector<unsigned int> &buffer )
{
  size_t count = buffer.size();
  if (count == 0)
    return;
  m_valuesReadCount += m_file.ReadValuesIntoBuffer(buffer);
}

uintmax_t CSorter::GetValuesLeftToRead()
{
  if (GetFileValuesCount() < m_valuesReadCount)
    return 0;
  else 
    return GetFileValuesCount() - m_valuesReadCount;
}

void CSorter::SaveVectorToFile( std::vector<unsigned int> &buffer )
{
  CHugeFile file;
  path filePath = GetNextTempFileName().data();
  file.CreateAndOpenForWriting(filePath);
  file.WriteBufferIntoFile(buffer);
  m_sortedFileChunks.push_back(filePath);
}

std::string CSorter::GetNextTempFileName()
{
  //TODO: check if file with this name doesn't exist
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
    delete m_threadGroup;
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
  uintmax_t desiredSize = m_chunkValuesCount;
  for (int i = 0; i < nCores; i++)
  {
    pVector = new vector<unsigned int>();
    Utility::AllocateBuffer(desiredSize, pVector);
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

void CSorter::MergeSortedFiles( boost::filesystem::path resultFilePath )
{
  bool continueMerge = true;
  unsigned int resultTemp;
  CHugeFile resultFile;
  auto filesToMerge = list<MergedFile*>();
  list<MergedFile*>::iterator fIt;
  vector<unsigned int> buffer;
  size_t iBuf = 0;
  
  auto freeMemorySize = Utility::GetFreeSystemMemory();

  Utility::AllocateBuffer(min(freeMemorySize * 0.8f, (size_t)GetFileValuesCount()), &buffer);
  buffer.resize(buffer.capacity());

  BOOST_FOREACH(auto filePath, m_sortedFileChunks)
  {
    filesToMerge.push_back(new MergedFile(filePath.string().data()));
  }
  resultFile.CreateAndOpenForWriting(resultFilePath);

  while (continueMerge)
  {
    for (fIt = filesToMerge.begin(); fIt != filesToMerge.end();)
    {
      (*fIt)->ReadNextVal();

      if ((*fIt)->ended)
      {
        delete (*fIt);
        fIt = filesToMerge.erase(fIt);
      }
      else
        fIt++;
    }

    if (filesToMerge.size() == 0)
    {
      continueMerge = false;
      if (iBuf > 0)
      {
        buffer.resize(iBuf);
        resultFile.WriteBufferIntoFile(buffer);
      }
    }
    else
    {
      fIt = min_element(filesToMerge.begin(), filesToMerge.end(), [](MergedFile* l, MergedFile* r) { return l->val < r->val; });

      resultTemp = (*fIt)->val;
      (*fIt)->readFrom = true;

      buffer[iBuf] = resultTemp;
      iBuf++;

      if (iBuf == buffer.size())
      {
        resultFile.WriteBufferIntoFile(buffer);
        iBuf = 0;
      }
    }
  }

  BOOST_FOREACH(auto filePath, m_sortedFileChunks)    //removing temp files
  {
    filesystem::remove(filePath);
  }
}

void CSorter::FreeBuffers()
{
  BOOST_FOREACH(auto pVector, m_buffers)
  {
    delete pVector;
  }
}