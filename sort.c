#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <sys/inotify.h>
#include "global_defs.h"
#include "cmd_exec.h"

#define DBNAME "cvfs_db"
#define EVENT_SIZE  ( sizeof (struct inotify_event) )
#define EVENT_BUF_LEN     ( 1024 * ( EVENT_SIZE + 16 ) )
#define WATCH_DIR   "/mnt/CVFSTemp/"

string ls_file_out = "";

int callback(void *notUsed, int argc, char **argv, char **colname){

   string ls_size = "", ls_size_out = "", mv = "";
   char *ptr;

   ptr = strtok(ls_file_out,"\n");
   while (ptr != NULL){
     sprintf(ls_size,"ls -l /mnt/CVFSTemp/ | grep '%s' | awk '{print $5}'", ptr);
     runCommand(ls_size, ls_size_out);

     if (atof(ls_size_out) > 546870912.00){
        //do something here
     } else {
        printf("\nFile: %s | File Size: %s bytes\n", ptr, ls_size_out);
	printf("File redirected to %s\n", argv[1]);
	sprintf(mv,"mv '/mnt/CVFSTemp/%s' '%s/%s'",ptr,argv[1],ptr);
        system(mv);
     }
     strcpy(ls_size_out,"");
     ptr = strtok(NULL,"\n");
   }

   return 0;
}

void doneCopy(struct inotify_event *e) {
    if (e->len <= 0) {
        printf("WTF? file no name?\n");
    }

    printf("wd = %d\n", e->wd);
    if (e->mask & IN_CLOSE_NOWRITE) {
        printf("IN_CLOSE_NOWRITE ");
        printf("%s is done copying.\n", e->name);
    }
    else if (e->mask & IN_CLOSE_WRITE) {
        printf("IN_CLOSE_WRITE ");
        printf("%s is done copying.\n", e->name);
    }
    else
        printf("wtf?\n");
}

// can put main
void watch_copy() {
    char buffer[EVENT_BUF_LEN];
    int wd, len, i;
    char *p;

    int fd = inotify_init();
    if (fd < 0) {
        perror("inotify_init failed");
    }
    wd = inotify_add_watch(fd, WATCH_DIR, IN_CLOSE_NOWRITE);   // may kulang for sure | IN_CLOSE_WRITE?
    if (wd < 0) {
        perror("inotify_add_watch failed");
    }

    while (1) {
        len = read(fd, buffer, EVENT_BUF_LEN);
        struct inotify_event *event;
        i = 0;
        if (len <= 0)
            perror("inotify read failed");

        p = buffer;
        while(p < buffer + len) {
            event = (struct inotify_event *) p;
            doneCopy(event);

            p += EVENT_SIZE + event->len;
        }
    }
}

int main()
{
   char *errmsg = 0;
   int rc;
   string sql = "", ls = "";

   rc = sqlite3_open(DBNAME,&db);
   if (rc != SQLITE_OK){
     fprintf(stderr, "Can't open database %s\n", sqlite3_errmsg(db));
     sqlite3_close(db);
     exit(1);
   }

   strcpy(ls, "ls /mnt/CVFSTemp");

   while (1){
     runCommand(ls, ls_file_out);

     //there are still files in CVFSTemp
     if (strcmp(ls_file_out,"")){
       strcpy(sql,"SELECT MAX(AVSPACE), MOUNTPT FROM TARGET;");

       rc = sqlite3_exec(db, sql, callback, 0, &errmsg);
       if (rc != SQLITE_OK){
          fprintf(stderr, "SQL Error: %s\n", errmsg);
          sqlite3_free(errmsg);
          sqlite3_close(db);
       }
     }
     strcpy(ls_file_out,"");
   }

   return 0;
}
