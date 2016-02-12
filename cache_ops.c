/*
    Cache Operations for cache access module
    @author: Jeff
*/

#include <stdio.h>
#include <stdlib.h>
#include <sqlite3.h>

#include "global_defs.h"

#define DBNAME	"cvfs_db"

/*
    increments frequency count of a file, this function assumes that
    the row exists in the database (inserted at initial file write)
*/
void incrementFrequency(string filename) {
    sqlite3 *db;
    char *errmsg = 0;
	int rc;
    string query = "";

    printf("Opening database %s\n", DBNAME);
	rc = sqlite3_open(DBNAME, &db);
	if(rc) {
		fprintf(stderr, "Can\'t open database %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

    sprintf(query, "UPDATE CacheContent SET frequency = frequency + 1 WHERE filename LIKE '%s';", filename);

	rc = sqlite3_exec(db, query, 0, 0, &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL Error: %s\n", errmsg);
		sqlite3_free(errmsg);
	}

	// close db
	sqlite3_close(db);

}

// temporary driver
int main() {
    // test increment: success!

}



/*
cache operation:

this code will be put on read module
so everytime read is recieved, do this func
void incrementFrequency(String filename) {
    update database +1 on frequency of filename
}

on write, (new file -> bahala na yung write mag insert taena)
// note: dapat check size di pde sobrang laki sa cache

// baka dapat naka thread ito or everytime mag increment frequency i call ito
refresh cache() {
    db = select top 10 from cache;
    ls = ls of cache folder
    compare db with ls, if not in ls, copy file to ls
    delete files in ls not in top 10
}




*/
