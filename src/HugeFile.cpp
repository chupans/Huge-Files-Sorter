#include "HugeFile.h"
#include "Utility.h"
#include "assert.h"
using namespace boost;
using namespace filesystem;
using namespace std;

#ifdef BOOST_OS_WINDOWS

CHugeFile::CHugeFile()
{
  m_bufValPos = 0;
  m_readFromBuf = true;
  m_eofHit = false;
}

CHugeFile:: ~CHugeFile()
{
  if (m_hFile != NULL)
    CloseHandle(m_hFile);
}


void CHugeFile::CloseFile()
{
  if (m_hFile != NULL)
  {
    CloseHandle(m_hFile);
    m_hFile = NULL;
  }
}


void CHugeFile::CreateAndOpenForWriting(path filePath)
{
  m_hFile = CreateFile(filePath.wstring().data(),
                       GENERIC_WRITE,
                       NULL,
                       NULL,
                       CREATE_ALWAYS,
                       FILE_ATTRIBUTE_NORMAL,
                       NULL);
}

void CHugeFile::OpenForReading(path filePath)
{
  m_hFile = CreateFile(filePath.wstring().data(),
                       GENERIC_READ,
                       NULL,
                       NULL,
                       OPEN_EXISTING,         
                       FILE_ATTRIBUTE_NORMAL, 
                       NULL);   
  auto newSize = Utility::AllocateBuffer(INTERNAL_BUFFER_SIZE, &m_internalBuffer);
  m_internalBuffer.resize(newSize);
}

size_t CHugeFile::ReadValuesIntoBuffer(vector<unsigned int> &buffer)
{
  DWORD NumberOfBytesRead = 0;
  ReadFile(m_hFile, &buffer[0], sizeof(unsigned int) * buffer.size(), &NumberOfBytesRead , NULL);
  return (size_t)NumberOfBytesRead / sizeof(unsigned int);
}

bool CHugeFile::GetNextVal(unsigned int &val)
{
  if (m_bufValPos == m_internalBuffer.size())
  {
    if (m_eofHit)
      return false;
    else
      m_readFromBuf = true;
  }

  if (m_readFromBuf)
  {
    DWORD NumberOfBytesRead = 0;
    m_bufValPos = 0;
    m_readFromBuf = false;
    auto readRet = ReadFile(m_hFile, &m_internalBuffer[0], sizeof(unsigned int) * m_internalBuffer.size(), &NumberOfBytesRead, NULL);
    size_t nValuesRead = (size_t)NumberOfBytesRead / sizeof(unsigned int);
    if (nValuesRead != m_internalBuffer.size() && readRet == TRUE)
      m_eofHit = true;
    m_internalBuffer.resize(nValuesRead);
  }


  if (m_bufValPos < m_internalBuffer.size())
  {
    val = m_internalBuffer[m_bufValPos];
    m_bufValPos++;
    return true;
  }
  else
  {
    assert(0);
  }
}

void CHugeFile::WriteBufferIntoFile(vector<unsigned int> &buffer)
{
  DWORD NumberOfBytesWritten = 0;
  WriteFile(m_hFile, &buffer[0], sizeof(unsigned int) * buffer.size(), &NumberOfBytesWritten, NULL);
}

#elif #ifdef BOOST_OS_LINUX

CHugeFile::CHugeFile()
{
}
CHugeFile:: ~CHugeFile()
{

}
void CHugeFile::CreateAndOpenForWriting(path filePath)
{

}
void CHugeFile::OpenForReading(path filePath)
{

}
size_t CHugeFile::ReadValuesIntoBuffer(vector<unsigned int> &buffer)
{
  return 0;
}
void CHugeFile::WriteBufferIntoFile(vector<unsigned int> &buffer)
{

}
void CHugeFile::WriteValueIntoFile(unsigned int val)
{

}

#endif