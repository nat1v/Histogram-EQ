#include <stdio.h>
#include <stdlib.h>


FILE *ImgIN;
FILE *ImgOUT;

int main(int argc, char *argv[])
{
  
  ImgIN = fopen(argv[1], "rb");
  ImgOUT = fopen(argv[2], "wb");;
  


  return 0;
}
