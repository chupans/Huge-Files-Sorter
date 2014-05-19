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
  CSorter(boost::filesystem::path fileToSortPath);
  ~CSorter(void);
  void SortAndSaveTo(boost::filesystem::path resultFilePath);
private:
  void SaveVectorIntoTempFile(std::vector<unsigned int> &buffer);
  void CalculateFileChunkSize();
  size_t GetFileChunkSize();
  static void SortSingleBuffer(std::vector<unsigned int> *buf);     //Wrapper to use in threads
  void MergeSortedFileChunksInto(boost::filesystem::path resultFilePath);
  boost::uintmax_t GetValuesLeftToRead();
  void CreateBuffers();
  void FreeBuffers();
  void PrepareThreads();


  boost::filesystem::path m_fileToSortPath;
  std::vector<boost::filesystem::path> m_sortedFileChunks;
  CHugeFile m_file;                                         //File 
  boost::uintmax_t m_valuesReadCount;                       //Amount of values already read from binary file
  size_t m_chunkValuesCount;                                //Amount of values in a chunk of binary file
  std::vector< std::vector<unsigned int>* > m_buffers;
  boost::thread_group *m_threadGroup;

};

