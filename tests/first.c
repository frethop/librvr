#include "rvrcmd.h"

void main(char **argv) {

   /* Test for the first couple RVR commands */
   init_port();
   set_logging_level(VERBOSE);

   wake();

   char * hello = echo("Hello World", strlen("Hello World"));
   char * h = hello;
   for (int i=0; i<12; i++) printf("%02x ", *(h++)); 
   printf("Sent 'Hello World' and got back '%s'\n", hello);

}   
