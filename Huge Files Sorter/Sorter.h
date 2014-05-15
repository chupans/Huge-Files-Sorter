#pragma once
#include <string>
#include <vector>
#include <list>
#include <boost/filesystem.hpp>


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

  boost::filesystem::path m_filePath;
  std::vector<boost::filesystem::path> m_sortedFileChunks;
  FILE *m_pFile;
  size_t m_valuesReadCount;

};
