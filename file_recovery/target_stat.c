// check target status if up or down
// can only check 1 target at a time

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#include <syslog.h>

#include "../Global/global_definitions.h"
#include "../Utilities/cmd_exec.h"


#define INTERVAL    300     // 5 mins

// untested
void kick_target(String mountpt, String iqn, String assocvol, String ipadd) {
    int rc;
    sqlite3 *db;
    char *errmsg = 0;
    String query = "";
    rc = sqlite3_open(DBNAME, &db);

    // kick out target from pool
    openlog("cvfs2", LOG_PID|LOG_CONS, LOG_USER);


    // not sure what other thing to do, but here's a draft:
    // delete from volcontent where mountpt = mounpt
    strcpy(query, "");
	sprintf(query, "DELETE FROM VolContent WHERE fileloc = '%s';", mountpt);
	rc = sqlite3_exec(db, query, 0, 0, &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL Error: %s\n", errmsg);
		sqlite3_free(errmsg);
        syslog(LOG_INFO, "File Recovery: Failed to kick target, SQL error");
		exit(1);
	}
    // delete from cache content
    strcpy(query, "");
	sprintf(query, "DELETE FROM CacheContent WHERE mountpt = '%s';", mountpt);
	rc = sqlite3_exec(db, query, 0, 0, &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL Error: %s\n", errmsg);
		sqlite3_free(errmsg);
        syslog(LOG_INFO, "File Recovery: Failed to kick target, SQL error");
		exit(1);
	}
    // delete from target table
    strcpy(query, "");
	sprintf(query, "DELETE FROM Target WHERE iqn = '%s';", iqn);
	rc = sqlite3_exec(db, query, 0, 0, &errmsg);
	if (rc != SQLITE_OK) {
		fprintf(stderr, "SQL Error: %s\n", errmsg);
		sqlite3_free(errmsg);
        syslog(LOG_INFO, "File Recovery: Failed to delete target, SQL error");
		exit(1);
	}

    // deactivate volume
    //unmount
    String umount = "";
    sprintf(umount, "umount '%s'", mountpt);
    system(umount);

    //remove folder
    String rm = "";
    sprintf(rm, "rmdir '%s'", mountpt);
    system(rm);

    //remove logical volume
    String lvremove = "";
    sprintf(lvremove, "lvremove '%s'", assocvol);
    system(lvremove);

    //remove volume group
    String vgremove = "", vgname = "";
    char *pch = NULL;
    int counter = 0;
    pch = strtok(assocvol, "/");
    while (pch != NULL){
        strcat(vgname, "/");
        strcat(vgname, pch);
        counter++;
        pch = strtok(NULL, "/");
        if (counter == 2)
        break;
    }
    sprintf(vgremove, "vgremove -a n '%s'", vgname);
    printf("vgremove = %s\n", vgremove);
    system(vgremove);

    syslog(LOG_INFO, "File Recovery: Target %s (%s) is removed from network due to inactivity.", iqn, ipadd);
    sqlite3_close(db);
}

// tested: working
int is_target_up(String ipadd) {
    String ping = "", out = "";
    sprintf(ping, "ping -c 4 %s | grep -oP \'\\d+(?=%% packet loss)\'", ipadd);
    runCommand(ping, out);
    if(strcmp(out, "100") == 0)    // 100 % packet loss = down
        return 0;
    return 1;
}

// run this in separate thread
// untested
void check_target() {
    sqlite3 *db;
	sqlite3_stmt *res;
	int rc;
	const char *tail;
	rc = sqlite3_open(DBNAME, &db);
    String iqn = "", mountpt = "", assocvol = "", prevadd = "";

    while (1) {
    	String query = "";
    	strcpy(query, "select ipadd, iqn, mountpt, assocvol from Target;");

    	if (rc != SQLITE_OK){
    		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
    		sqlite3_close(db);
    		exit(1);
    	}

    	rc = sqlite3_prepare_v2(db, query, 1000, &res, &tail);
    	if (rc != SQLITE_OK){
    		fprintf(stderr, "Can't execute prepare v2: %s\n", sqlite3_errmsg(db));
    		sqlite3_close(db);
    		exit(1);
    	}

    	// for each target, we check if its up
    	while (sqlite3_step(res) == SQLITE_ROW) {
            String curradd = "";
            strcpy(curradd, sqlite3_column_text(res,0));
            strcpy(iqn, sqlite3_column_text(res,1));
            strcpy(mountpt, sqlite3_column_text(res,2));
            strcpy(assocvol, sqlite3_column_text(res,3));
            if(!is_target_up(curradd)) {  // if down,
                if(strcmp(curradd, prevadd) == 0) {   // down for 10 mins?! get outta my network!
                    kick_target(mountpt, iqn, assocvol, curradd);
                }
                strcpy(prevadd, curradd);
            }
    	}

        sleep(INTERVAL);
    }
	//counter = 0;
	sqlite3_finalize(res);
	sqlite3_close(db);
}

int main() {
    check_target();
    return 0;

}

