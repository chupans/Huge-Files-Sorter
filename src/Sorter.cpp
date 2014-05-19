#include "Sorter.h"
#include <algorithm>
#include <limits>
#include <boost/foreach.hpp>
#include "Utility.h"
#include "MergedFile.h"
#define NOMINMAX

using namespace std;
using namespace boost;
using namespace filesystem;

CSorter::CSorter( boost::filesystem::path fileToSortPath ) : m_fileToSortPath(fileToSortPath)
{
  printf("\nPreparing to sort\n");
  m_threadGroup = NULL;
  m_valuesReadCount = INT64_C(0);
  CalculateFileChunkSize();
  CreateBuffers();
}

void CSorter::CreateBuffers()
{
  vector<unsigned int> *pVector;
  auto nCores = Utility::GetCoresCount();
  size_t desiredSize = m_chunkValuesCount;
  for (unsigned int i = 0; i < nCores; i++)
  {
    pVector = new vector<unsigned int>();
    m_chunkValuesCount = min(Utility::AllocateBuffer(desiredSize, pVector), m_chunkValuesCount);
    auto bufS = (m_chunkValuesCount*sizeof(unsigned int) / (1024*1024));
    printf("Allocated buffer with size of %u MB\n", bufS);
    m_buffers.push_back(pVector);
  }

  BOOST_FOREACH(pVector, m_buffers)
  {
    pVector->reserve(m_chunkValuesCount);
  }
}

void CSorter::FreeBuffers()
{
  BOOST_FOREACH(auto pVector, m_buffers)
  {
    delete pVector;
  }
}
CSorter::~CSorter(void)
{
  delete m_threadGroup;
}

uintmax_t CSorter::GetValuesLeftToRead()
{
  if (Utility::GetFileValuesCount(m_fileToSortPath) < m_valuesReadCount)
    return 0;
  else 
    return Utility::GetFileValuesCount(m_fileToSortPath) - m_valuesReadCount;
}

void CSorter::CalculateFileChunkSize()
{
  auto freeMemorySize = Utility::GetFreeSystemMemory();
  printf("Free memory size %llu MB\n", freeMemorySize /(1024*1024));
  freeMemorySize = std::min(((unsigned long long)std::numeric_limits<size_t>::max()), freeMemorySize);
  freeMemorySize *= 0.8f;

  m_chunkValuesCount = Utility::ConvertLong(min(freeMemorySize / sizeof(unsigned int), Utility::GetFileValuesCount(m_fileToSortPath)));
  m_chunkValuesCount /= Utility::GetCoresCount();
}

size_t CSorter::GetFileChunkSize()
{    
  return min(m_chunkValuesCount, Utility::ConvertLong(GetValuesLeftToRead()));
}

void CSorter::SortAndSaveTo( boost::filesystem::path resultFilePath )
{
  m_file.OpenForReading(m_fileToSortPath);

  while(GetValuesLeftToRead() > 0)
  {
    BOOST_FOREACH(auto pVector, m_buffers)
    {
      pVector->resize(GetFileChunkSize());      //Can resize vector to 0
      m_valuesReadCount += m_file.ReadValuesIntoBuffer(*pVector);
    }
    PrepareThreads();
    m_threadGroup->join_all();
    printf("Saving sorted chunks\n");
    BOOST_FOREACH(auto pVector, m_buffers)
    {
      SaveVectorIntoTempFile(*pVector);
    }
  }
  FreeBuffers();
  m_file.CloseFile();
  printf("\nMerging\n");
  MergeSortedFileChunksInto(resultFilePath);
}

void CSorter::SaveVectorIntoTempFile( vector<unsigned int> &buffer )
{
  CHugeFile file;
  path filePath = Utility::GetNextTempFileName().data();
  file.CreateAndOpenForWriting(filePath);
  file.WriteBufferIntoFile(buffer);
  m_sortedFileChunks.push_back(filePath);
}

void CSorter::PrepareThreads()
{
  if (m_threadGroup != NULL)
  {
    delete m_threadGroup;
  }
  m_threadGroup = new thread_group();
  BOOST_FOREACH(auto pVector, m_buffers)
  {
    m_threadGroup->create_thread(boost::bind(&CSorter::SortSingleBuffer, pVector));
  }
}

void CSorter::SortSingleBuffer( std::vector<unsigned int> *buf )
{
  printf("Started asynchronous sort of chunk of file\n");
  sort(buf->begin(), buf->end());
}

void CSorter::MergeSortedFileChunksInto( filesystem::path resultFilePath )
{
  bool continueMerge = true;
  unsigned int resultTemp;
  CHugeFile resultFile;
  auto filesToMerge = list<CMergedFile*>();
  list<CMergedFile*>::iterator fIt;
  vector<unsigned int> resultFileBuffer;
  size_t iBuf = 0;
  
  auto freeMemorySize = Utility::GetFreeSystemMemory();
  freeMemorySize = std::min(((unsigned long long)std::numeric_limits<size_t>::max()), freeMemorySize);
  freeMemorySize *= 0.8f;
  auto bufferSize = Utility::ConvertLong(min(freeMemorySize / sizeof(unsigned int), Utility::GetFileValuesCount(m_fileToSortPath)));

  Utility::AllocateBuffer(bufferSize, &resultFileBuffer);
  resultFileBuffer.resize(resultFileBuffer.capacity());
  printf("Merge buffer size = %u Mb\n", resultFileBuffer.capacity()/(256*1024));

  BOOST_FOREACH(auto filePath, m_sortedFileChunks)
  {
    filesToMerge.push_back(new CMergedFile(filePath.string().data()));
  }
  resultFile.CreateAndOpenForWriting(resultFilePath);
  printf("Files count = %u\n", filesToMerge.size());

  while (continueMerge)
  {
    //Delete chunks that was fully read
    for (fIt = filesToMerge.begin(); fIt != filesToMerge.end();)
    {
      if ((*fIt)->IsEof())
      {
        delete (*fIt);
        fIt = filesToMerge.erase(fIt);
		printf("Deleting file\n");
      }
      else
        fIt++;
    }

    //Check if there is no chunks left
    if (filesToMerge.size() == 0)
    {
      continueMerge = false;
      if (iBuf > 0)     //Check if we wrote some values to buffer and write them into result file if so
      {
		printf("Buffer isn't empty\n");
        resultFileBuffer.resize(iBuf);
        resultFile.WriteBufferIntoFile(resultFileBuffer);
      }
    }
    else
    {
      fIt = min_element(filesToMerge.begin(), filesToMerge.end(), [](CMergedFile* l, CMergedFile* r) { return l->GetVal() < r->GetVal(); });

      resultTemp = (*fIt)->GetVal();
      (*fIt)->ReadNextVal();

      resultFileBuffer[iBuf] = resultTemp;
      iBuf++;

      //If buffer is full write its contents to result file
      if (iBuf == resultFileBuffer.size())
      {
        resultFile.WriteBufferIntoFile(resultFileBuffer);
		printf("Writing buffer\n");
        iBuf = 0;
      }
    }
  }

  BOOST_FOREACH(auto filePath, m_sortedFileChunks)    //removing temp files
  {
    remove(filePath);
  }
}
