#include "Sorter.h"
#include "generator.h"

int main()
{
  generate();
  CSorter a("C:\\work\\Huge Files Sorter\\test\\file.binary");
  a.Process();
}