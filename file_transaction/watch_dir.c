
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/times.h>
#include <sys/inotify.h>
#include <limits.h>
#include <dirent.h>
#include <errno.h>
#include <sqlite3.h>
#include <syslog.h>

#include "../Global/global_definitions.h"
#include "../volume_management/file_mapping.h"
#include "../volume_management/file_assembly.h"
#include "../disk_pooling/file_presentation.h"
#include "../cache_access/cache_operation.h"
#include "../file_striping/file_striping.h"

#define MAX_EVENTS 1024 /*Max. number of events to process at one go*/
#define LEN_NAME 128 /*Assuming that the length of filename won't exceed 16 bytes*/
#define EVENT_SIZE ( sizeof (struct inotify_event) ) /*size of one event*/

#define BUF_LEN  ( MAX_EVENTS * ( EVENT_SIZE + NAME_MAX + 1 )) /*buffer to store the data of events*/

#define MAXDEPTH 30

#define SHIT_STORAGE "/mnt/CVFSFStorage"

#define MAX_WTD 69 //Assuming all max watches are only 69

void make_folder(String root){
	sqlite3 *db;
	sqlite3_stmt *res;
	int rc;
	const char *tail;
	rc = sqlite3_open(DBNAME, &db);

	String sql = "";
	strcpy(sql, "Select mountpt from Target;");

	if (rc != SQLITE_OK){
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	rc = sqlite3_prepare_v2(db, sql, 1000, &res, &tail);
	if (rc != SQLITE_OK){
		fprintf(stderr, "Can't execute prepare v2: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	//int counter = 0;
	while (sqlite3_step(res) == SQLITE_ROW){
		String mkdir;
		sprintf(mkdir, "mkdir '%s/%s/'", sqlite3_column_text(res,0), root);
		//syslog(LOG_INFO, "FileTransaction: MKDIR = %s\n", mkdir);
		system(mkdir);

		String chmod;
		sprintf(chmod, "chmod -R 777 '%s/%s/'", sqlite3_column_text(res,0), root);
		system(chmod);
	}
	//counter = 0;
	sqlite3_finalize(res);
	sqlite3_close(db);
}

void get_root(int wds[], int trigger[] , String dirs[], int counter, int wd, String arr[])
{
    int x;
    for (x = 1; x < counter; x++){
        if (wd == wds[x]){
	    strcpy(arr[x], dirs[x]);
            return get_root(wds, trigger, dirs, counter, trigger[x], arr);
	}
    }
    //return arr;
}

void *watch_temp()

{
    int length, i = 0, wd;
    int fd;
    char buffer[BUF_LEN];

    int wds[MAX_WTD];	//for the wds
    int trigger[MAX_WTD];	//for the event->wd that trigger the current wds
    String dirs[MAX_WTD];	//for the directory names that is trigger by the wds
    int counter = 1;

    /*Initialize inotify*/
    fd = inotify_init();
    if ( fd < 0 ) {
       perror( "Couldn't initialize inotify" );
    }
    /*add watch to directory*/
    wd = inotify_add_watch(fd, TEMP_LOC, IN_ALL_EVENTS);
    wds[counter-1] = wd;
    strcpy(dirs[counter-1], TEMP_LOC);

    if (wd == -1){
        syslog(LOG_INFO, "FileTransaction: Couldn't add watch to %s\n", TEMP_LOC);
    } else {
        syslog(LOG_INFO, "FileTransaction: WRITE :: Watching:: %s\n", TEMP_LOC);

    }

    /*do it forever*/
    while(1){
	//select(fd+1, &descriptors, NULL, NULL, &time_to_wait);
       create_link();
       i = 0;
       length = read(fd, buffer, BUF_LEN);

       if (length < 0){
          perror("read");
       }
       while(i < length){
           struct inotify_event *event = (struct inotify_event *) &buffer[i];

           if (event->len){

	      if (event->mask & IN_CREATE){
		if (event->mask & IN_ISDIR){
		     //printf("%s is created.\n", event->name);
		     String dir_to_watch = "";
		     String root = "";
		     String arr[MAXDEPTH];
		     int d;
		    //Initialize array....
		     for (d = 0; d < 30; d++){
			strcpy(arr[d], "");
		     }

		     get_root(wds,trigger, dirs, counter,event->wd,arr);

		     for (d = 1; d < counter; d++){
			if(strcmp(arr[d], "") != 0) {
			    strcat(root, arr[d]);
			    strcat(root, "/");
			}
		     }

		     String x;
		     sprintf(x, "%s%s", root, event->name);

                     sprintf(dir_to_watch,"%s/%s%s/", TEMP_LOC, root, event->name);
		     wd = inotify_add_watch(fd, dir_to_watch, IN_ALL_EVENTS);

                     if (wd == -1){

		     } else {
			syslog(LOG_INFO, "FileTransaction: WRITE := Watching := %s\n", dir_to_watch);
		     }
		     //printf("DIR_TO_WATCH := %s\n", dir_to_watch);

		     wds[counter] = wd;
		     trigger[counter] = event->wd;
                     strcpy(dirs[counter], event->name);



		/***************CREATES in /mnt/CVFSFSTorage AND LINK DIRECTORY to SHare *********/
		     String dir = "", chmod = "",  sors = "", dest = "";
		     sprintf(dir, "mkdir '%s/%s'", SHIT_STORAGE, event->name);
		     system(dir);

                    sprintf(chmod, "chmod 777 -R '%s/%s'", SHIT_STORAGE, event->name);
		    system(chmod);

		     sprintf(sors, "%s/%s", SHIT_STORAGE, event->name);
		     sprintf(dest, "%s/%s", SHARE_LOC, x);
		     symlink(sors,dest);
	         /******************************************************************************/
		     make_folder(x);
		     counter++;
		}
              }

              if (event->mask & IN_CLOSE){
		if (event->mask & IN_ISDIR){
		} else {
		     String root = "";
		     String arr[MAXDEPTH];
		     int n = sizeof(wds) / sizeof(wds[0]);
		     int d, i, rooti;
		      //initialize the array...
		      for (d = 0; d < MAXDEPTH; d++){
			strcpy(arr[d], "");
		      }


		      get_root(wds, trigger, dirs, counter, event->wd, arr);
		      for (d = 1; d < counter; d++){
			if(strcmp(arr[d], "") != 0) {
			    strcat(root, arr[d]);
			    strcat(root, "/");
			}
		      }

                      String filepath = "";
		      String filename = "";
		      sprintf(filename, "%s%s", root, event->name);
                      FILE *fp;
                      sprintf(filepath, "%s/%s%s", TEMP_LOC, root, event->name);
		      //syslog(LOG_INFO, "FileTransaction: FILEPATH := %s\n", filepath);
		      //syslog(LOG_INFO, "FileTransaction: hahahaa: filepath:%s\n\tfilename:%s", filepath, filename);
                      fp = fopen(filepath, "rb");
                      if (fp != NULL){
                         fseek(fp, 0L, SEEK_END);
                         long sz = ftell(fp);
                         rewind(fp);
                         //check if stripe file
                        if (sz > STRIPE_SIZE){
                           //before striping, check cache
			   printf("STRIPED: %s | SIZE : %ld bytes\n", event->name, sz);
                           //printf("%s will be striped.\n", event->name);
			   //printf("Inserting into CacheContent...\n");
                           update_cache_list(event->name, root);
                           //printf("Cache Count: %d\n", getCacheCount());
                          // if (getCacheCount() < MAX_CACHE_SIZE) { //max cache size is 10
                           printf("Cache Size: %d\n", getCacheCount());
			   printf("%s will be put to cache.\n", filename);
                           file_map_cache(filename, event->name);
                             //create_link_cache(filename);
                          // }
                           //stripe(event->n);
                           //refreshCache();	
			   //printf("ROOT = %s\n", root);
			   //printf("FILEPATH := %s\n", filepath);
			   //printf("FILENAME := %s\n", filename);
			   stripe(root, filepath, filename);
			   //refreshCache();
                        } else {
			   syslog(LOG_INFO, "FileTransaction: Transferring %s to targets...\n", filename);
                           file_map(filepath, filename);
                        }
		    }
                  }
              }

              if (event->mask & IN_MOVED_TO){
                 if (event->mask & IN_ISDIR)
                      printf("The directory %s is transferring.\n", event->name);
                  else
                      printf("The file %s is transferring.\n", event->name);

              }

              i += EVENT_SIZE + event->len;
           }
       }
    }
    /* Clean up */
    inotify_rm_watch(fd, wd);
    close(fd);
}
