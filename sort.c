#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <sqlite3.h>
#include <sys/inotify.h>
#include "global_defs.h"
#include "stripe.h"
#include "sort.h"

#define DBNAME "cvfs_db"
#define WATCH_DIR "/mnt/CVFSTemp/"
#define BUFF_SIZE ((sizeof(struct inotify_event)+16)*1024)

void updateVolContent(string filename, const char* fileloc){
   int rc;
   string sql = "";
   
   sprintf(sql, "insert into VolContent values ('%s','%s');",filename,fileloc);
   rc = sqlite3_exec(db,sql,0,0,0);
   if (rc != SQLITE_OK){
     printf("Failed to update VolContent database!\n");
     sqlite3_close(db);
     exit(1);
   }
}

void sort(string filename){
   int rc;
   sqlite3_stmt *res;
   string sql = "", mv = "";
   strcpy(sql,"select max(avspace), mountpt from Target;");
   
   rc = sqlite3_prepare_v2(db,sql,-1,&res,0);
   if (rc != SQLITE_OK){
     printf("Error: PREPARE V2.\n");
     sqlite3_close(db); exit(1);
   }
   rc = sqlite3_step(res);
   if (rc == SQLITE_ROW){
     printf("File %s is redirected to %s.\n",filename, sqlite3_column_text(res,1));
     sprintf(mv,"mv '/mnt/CVFSTemp/%s' '%s/%s'", filename, sqlite3_column_text(res,1),filename);
     system(mv);
     updateVolContent(filename,sqlite3_column_text(res,1));
   }
}

void get_event (int fd)
{
   ssize_t len, i = 0;
   string action = "";
   char buff[BUFF_SIZE] = {0};

   len = read (fd, buff, BUFF_SIZE);
   
   while (i < len) {
      struct inotify_event *pevent = (struct inotify_event *)&buff[i];
      if (pevent->mask & IN_MOVED_TO){ 
          sprintf(action,"File %s is transferring.",pevent->name);
	  printf("%s\n",action);
      }
      if (pevent->mask & IN_CLOSE_WRITE){
          string ls_size = "", ls_size_out = "";
          sprintf(ls_size,"ls -l /mnt/CVFSTemp | grep '%s' | awk '{print $5}'", pevent->name); 
          runCommand(ls_size,ls_size_out);
          //system(ls_size);
          //printf("File Size : %s",ls_size_out);
          if (atof(ls_size_out) > 536870912.00){
	  //printf("File %s will be striped.",pevent->name);
             stripe(pevent->name);
          } else {
             sort(pevent->name);
          }
      }
      i += sizeof(struct inotify_event) + pevent->len;
   }
}
   
void watch_copy()
{
   int result;
   int fd;
   int wd;   /* watch descriptor */
   int rc;
  
   rc = sqlite3_open(DBNAME,&db);
   if (rc != SQLITE_OK){
     printf("Error: Cannot connect to database!\n");
     exit(1);
   }

   fd = inotify_init();
   if (fd < 0) {
      printf("\nError: File Descriptor\n");
      exit(1);
   }
   
   wd = inotify_add_watch (fd, WATCH_DIR, IN_MOVED_TO | IN_CLOSE_WRITE | IN_CLOSE_NOWRITE);
   if (wd < 0) {
      printf("\nError: Watch Descriptor\n");
      exit(1);
   }
 
   
   while (1) {
      	get_event(fd);
   }
}
