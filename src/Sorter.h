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
private:
  void ReadIntoVector(std::vector<unsigned int> &buffer);
  void SaveVectorToFile(std::vector<unsigned int> &buffer);
  std::string GetNextTempFileName();
  void MergeAllFiles();
  void MergeTwoSortedFiles(boost::filesystem::path file1, boost::filesystem::path file2, boost::filesystem::path resultFile);
  void MergeTwoSortedVectors(std::vector<unsigned int> &buf1, std::vector<unsigned int> &buf2, boost::filesystem::path resultFile);
  size_t GetValuesLeftToRead();
  void CreateBuffers(unsigned int buffersCount);
  void PrepareThreads(std::vector<unsigned int> &buf1);

  boost::filesystem::path m_filePath;
  std::vector<boost::filesystem::path> m_sortedFileChunks;
  FILE *m_pFile;
  size_t m_valuesReadCount;
  std::vector< std::vector<unsigned int>* > m_buffers;
  boost::thread_group m_threadGroup;

};
