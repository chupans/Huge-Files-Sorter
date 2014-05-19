#ifdef _DEBUG
#include "vld.h"
#endif

#include "Sorter.h"
#include "generator.h"

int main(int argc, char* argv[])
{

  generate(500, "C:\\work\\Huge Files Sorter\\test\\file1.binary");
  printf("Files generated\n");
  CSorter sorter("C:\\work\\Huge Files Sorter\\test\\file1.binary");
  sorter.SortAndSaveTo("C:\\work\\Huge Files Sorter\\test\\result.binary");
  return 0;
}