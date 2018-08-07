#include"./incl/keymnglog.h"
int main()
{
  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[0],0,"func main() 00000000000");
  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[1],1,"func main() 00000000000");
  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[2],2,"func main() 00000000000");
  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[3],3,"func main() 00000000000");
  KeyMng_Log(__FILE__,__LINE__,KeyMngLevel[4],4,"func main() 00000000000");
}
