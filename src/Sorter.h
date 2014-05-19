#pragma once
#include <string>
#include <vector>
#include <list>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>
#include "HugeFile.h"

class CSorter
{
public:
  CSorter(char *filePath);
  ~CSorter(void);
  void SortAndSaveTo(char* filePath);
private:
  void ReadIntoVector(std::vector<unsigned int> &buffer);
  void SaveVectorToFile(std::vector<unsigned int> &buffer);
  boost::uintmax_t GetFileChunkSize();
  boost::uintmax_t GetFileValuesCount();
  static void SortSingleBuffer(std::vector<unsigned int> *buf);
  std::string GetNextTempFileName();
  void MergeSortedFiles(boost::filesystem::path resultFilePath);
  boost::uintmax_t GetValuesLeftToRead();
  void CreateBuffers();
  void FreeBuffers();
  void PrepareThreads();
  unsigned int GetCoresCount();

  boost::filesystem::path m_filePath;
  std::vector<boost::filesystem::path> m_sortedFileChunks;
  CHugeFile m_file;
  boost::uintmax_t m_valuesReadCount;
  boost::uintmax_t m_chunkValuesCount;
  std::vector< std::vector<unsigned int>* > m_buffers;
  boost::thread_group *m_threadGroup;

};

struct MergedFile
{
public:
  MergedFile(const char *filePath) 
  { 
    file.OpenForReading(filePath);
    readFrom = true; 
    ended = false;
    val = 0;
  };
  void ReadNextVal() 
  {
    if (readFrom)
    {
      if (file.GetNextVal(val) == false) 
        ended = true; 
      readFrom = false; 
    }
  };
  CHugeFile file;
  bool readFrom;
  bool ended;
  unsigned int val;
};

