#include "Sorter.h"
#include "generator.h"

int main()
{
  generate();
  printf("Files generated\n");
  CSorter a("C:\\work\\Huge Files Sorter\\test\\file.binary");
  a.Process();
}