#include<stdio.h>
#include<unistd.h>
int main()
{
  close(fileno(stderr));
  printf("%d",fileno(stderr));
}
