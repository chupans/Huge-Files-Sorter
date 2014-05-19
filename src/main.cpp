#include "Sorter.h"
#include "generator.h"
#include "Utility.h"
#include "boost/filesystem.hpp"

int main(int argc, char* argv[])
{
  char anwser;
  boost::filesystem::path binF;
  if (argc > 1)
  {
    if (boost::filesystem::is_regular_file(argv[1]))
    {
      printf("Sort file %s\n(y/n)? ", argv[1]);
      scanf("%c", &anwser);
      if (anwser == 'y' || anwser == 'Y')
      {
        binF = argv[1];
      }
    }
  }
  else
  {
    printf("Generate new file?\n(y/n)? ");
    scanf("%c", &anwser);
    if (anwser == 'y' || anwser == 'Y')
    {
      unsigned int genSize;
      printf("Type size of file to generate in MB: ");
      scanf("%u", &genSize);
      printf("\n");
      Generate(genSize, "file.binary");
      binF = "file.binary";
      printf("File file.binary generated\n");
    }
  }
  
  if (boost::filesystem::is_regular_file(binF))
  {
    CSorter sorter(binF);
    sorter.SortAndSaveTo("result.binary");
    printf("Finished\n");
  }

  char i = 0;
  printf("\nType 1 and press enter to exit\n");
  while (i != '1')
  {
    scanf("%c", &i);
  }
  return 0;
}