#include "common.h"

EAPI int
elm_main(int argc, char **argv)
{
   int i;
   
   for (i = 1; i < argc; i++)
     {
	// parse args
     }
   
   data_init();
   create_main_win();
   elm_run();
   data_shutdown();
   elm_shutdown();
   return 0;
}
ELM_MAIN()
