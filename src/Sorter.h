#pragma once
#include <string>
#include <vector>
#include <list>
#include <boost/filesystem.hpp>
#include <boost/thread.hpp>


class CSorter
{
public:
  CSorter(char *filePath);
  ~CSorter(void);
  void Process();
  size_t GetFileChunkSize();
  size_t GetFileValuesCount();
  size_t GetFileSize();
  void GetFreeMemorySize();
  static void SortSingleBuffer(std::vector<unsigned int> *buf);
private:
  void ReadIntoVector(std::vector<unsigned int> &buffer);
  void SaveVectorToFile(std::vector<unsigned int> &buffer);
  std::string GetNextTempFileName();
  void MergeTwoSortedFiles(boost::filesystem::path file1, boost::filesystem::path file2, boost::filesystem::path resultFile);
  void MergeSortedFiles(boost::filesystem::path resultFile);
  void MergeTwoSortedVectors(std::vector<unsigned int> &buf1, std::vector<unsigned int> &buf2, boost::filesystem::path resultFile);
  size_t GetValuesLeftToRead();
  void CreateBuffers();
  void PrepareThreads();
  unsigned int GetCoresCount();

  boost::filesystem::path m_filePath;
  std::vector<boost::filesystem::path> m_sortedFileChunks;
  FILE *m_pFile;
  size_t m_valuesReadCount;
  size_t m_chunkValuesCount;
  std::vector< std::vector<unsigned int>* > m_buffers;
  boost::thread_group *m_threadGroup;

};
