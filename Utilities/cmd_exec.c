#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>

#include "../Global/global_definitions.h"
#include "cmd_exec.h"

void runCommand(String comm, String out){
   FILE *fp;

   fp = popen(comm, "r");
   if (fp == NULL){
     syslog(LOG_INFO, "Utilities: runCommand: error fp is NULL\n");
     exit(1);
   }

   String line;
   while(fgets(line, sizeof(line) - 1, fp) != NULL){
     strcat(out, line);
   }
   out[strlen(out) - 1] = '\0';
   pclose(fp);
}

void writeFromCommand(String comm, String filePath){
   String cmdOut;
   FILE *fp;

   runCommand(comm, cmdOut);

   fp = fopen(filePath, "w");
   fprintf(fp, "%s", cmdOut);

   fclose(fp);
}
