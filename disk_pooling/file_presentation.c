#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sqlite3.h>
#include <string.h>
#include <syslog.h>

#include "../Global/global_definitions.h"
#include "../Utilities/cmd_exec.h"
#include "file_presentation.h"

void* create_link(){
   int rc;
   const char *tail;

   String query = "", comm = "", comm_out = "";
   //int inCache = 0;

   sqlite3 *db;
   sqlite3_stmt *res;

   rc = sqlite3_open(DBNAME, &db);
   if (rc != SQLITE_OK){
     fprintf(stderr, "File Presentation: Can't open database: %s.\n", sqlite3_errmsg(db));
     sqlite3_close(db);
     exit(1);
   }

   sprintf(comm, "ls %s", CACHE_LOC);
   runCommand(comm, comm_out);
   //String original;
   //strcpy(original, comm_out);
   //char *pch = strtok(comm_out, "\n");

   sprintf(query, "SELECT filename, fileloc FROM VolContent;");
    int good = 0;
     while(!good) {
	rc = sqlite3_prepare_v2(db, query, 1000, &res, &tail);
	if (rc != SQLITE_OK) {
		//printf("db is locked\n");
	} else {
		good = 1;
	}
      }
   

 
   while (sqlite3_step(res) == SQLITE_ROW){
	int inCache = 0;
        String orig = "";
        strcpy(orig, comm_out);

	String file = "";
	strcpy(file, sqlite3_column_text(res,0));
	char *pch1 = strtok(file,"/");
	String x = "", dir = "";
	int folder = 0;
	while (pch1 != NULL){
		strcat(dir, pch1);
		strcat(dir, "/");
		strcpy(x, pch1);
		pch1 = strtok(NULL, "/");
		folder = 1;
	}
	dir[strlen(dir)-strlen(x) - 1] = '\0';

	if (!folder){
		strcpy(x,sqlite3_column_text(res,0));
	} 

	char *pch = strtok(orig, "\n");
	while (pch != NULL){
		//if (strcmp(sqlite3_column_text(res,0),"part1.sysmgmt-s001.vmdk") == 0){
		//	printf("RES : %s || PCH : %s\n", sqlite3_column_text(res,0), pch);
		//}
		if (strcmp(x,pch) == 0){
			inCache = 1;
			break;
		}
		pch = strtok(NULL, "\n");
	}
        
	//printf("FILE := %s || IN CACHE = %d\n", sqlite3_column_text(res,0), inCache);
	//printf("X := %s\n", x);
	if (!inCache){
		//printf("NOT IN CAHCE!!!\n");
		//if not in cache, link only part1 and linear file
		if (strstr(sqlite3_column_text(res,0), "part1.") != NULL || strstr(sqlite3_column_text(res,0), "part") == NULL){
			String source = "", dest = "";
			sprintf(source, "%s/%s", sqlite3_column_text(res,1), sqlite3_column_text(res,0));
			//printf("SOURCE := %s\n", source);
			if (strstr(sqlite3_column_text(res,0), "part1.") != NULL){
				//printf("STRIPED!!!!\n");
				//String part = "";
				//strcpy(part, sqlite3_column_text(res,0));
				//printf("X := %s DIR : = %s\n", x, dir);
				memmove(x, x + strlen("part1."), 1 + strlen(x + strlen("part1.")));
				//printf("X AFTER := %s\n", x);
				sprintf(dest, "%s/%s%s", SHARE_LOC, dir, x);
			} else {
				sprintf(dest, "%s/%s", SHARE_LOC, sqlite3_column_text(res,0));
			}


			//printf("SOURCE := %s | DEST := %s\n", source, dest);
			//printf("DEST := %s\n", dest);
			if (symlink(source,dest) == 0){
				syslog(LOG_INFO, "File Presentation: Created Link: %s\n", dest);
			}else {	}
		}
	} else {
		if (strstr(sqlite3_column_text(res,0),"part1.") != NULL){
			String filename = "", source = "", dest = "";
			//strcpy(filename, sqlite3_column_text(res,0));
			sprintf(source, "%s/%s", CACHE_LOC, x);
			memmove(x,x+strlen("part1."),1+strlen(x+strlen("part1.")));
			//sprintf(source, "%s/%s", CACHE_LOC, sqlite3_column_text(res,0));
			sprintf(dest, "%s/%s%s", SHARE_LOC, dir,x);
			//printf("IN CACHE: SOURCE := %s | DEST := %s\n", source, dest);
			if (symlink(source,dest) == 0){
				syslog(LOG_INFO,"File Presentation: Created Link %s\n", dest);
			} 
		}
	}
   }

   sqlite3_finalize(res);
   sqlite3_close(db);
}


void update_link_cache(String filename, String fileloc){
    String ln, rm;

    //sprintf(rm, "rm '/mnt/Share/%s'", filename);
    //syslog(LOG_INFO, "DiskPooling: Rm = %s\n", rm);
    //system(rm);
    // sprintf(ln, "ln -s '%s/%s' '%s/%s'", fileloc, filename, SHARE_LOC, filename);
    //syslog(LOG_INFO, "DiskPooling: LN = %s\n", ln);
    // system(ln);
    String sors = "", dest = "";
	sprintf(sors, "'%s/%s'", fileloc, filename);
	sprintf(dest, "'%s/%s'", SHARE_LOC, filename);
    if(symlink(sors, dest) == 0) {
        syslog(LOG_INFO, "DiskPooling: Link Created: '%s'\n", dest);
    } else {
        printf("!!! Error creating link %s", dest);
    }

    //syslog(LOG_INFO, "DiskPooling: Update Link for file: %s\n", filename);
}
