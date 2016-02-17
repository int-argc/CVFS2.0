/*
    CVFS Inital Configuration
    @author: Aidz and Jeff           :">
    requires:
        open-iscsi
        nmap
        sqlite3
        libsqlite3-dev
    uses:
        cmd_exec.c
        volman.c
*/

#include <stdio.h>
#include <string.h>
#include "global_defs.h"
#include "cmd_exec.h"
#include "volman.h"
#include "init_conf.h"
#include "sort.h"
#include <stdlib.h>

#define TARGET_FILE "target_list.txt"
#define GBSTR  "GiB"
#define MBSTR  "MB"    // assumption
#define TBSTR  "TB"

double toBytes(string sUnit) {
    char *ptr;

    ptr = strtok(sUnit, " ");
    double val = atof(ptr);
    ptr = strtok(NULL, " ");
    if (strcmp(ptr, GBSTR) == 0) {
        val *= 1000000000;
    } else if (strcmp(ptr, MBSTR) == 0) {
        val *= 1000000;
    } else if (strcmp(ptr, TBSTR) == 0) {
        val *= 1000000000000;
    }
    // printf("val = %f\n", val);
    return val;
}

void initialize() {

    system("clear");

    string ip, netmask, command, alltargetsStr, currtarget, iqn, sendtargets,sql;
    string disklist, assocvol, mountpt, avspace, command1, sql1;

    //sqlite3 information
    int rc = 0;

    //open sqlite3 db
    rc = sqlite3_open("cvfs_db",&db);
    if (rc){
       printf("\nCannot open database.\n");
       exit(0);
    }

    // get network information of initiator
    // what if interface is not eth0? what if eth1...?
    runCommand("ip addr show eth0 | grep \'inet \' | awk \'{print $2}\' | cut -f1 -d\'/\'", ip);
    runCommand("ip addr show eth0 | grep \'inet \' | awk \'{print $2}\' | cut -f2 -d\'/\'", netmask);
    printf("*****  Network information  *****\n");
    ip[strlen(ip)] = '\0';
    netmask[strlen(netmask)] = '\0';
    printf("IP address: %s/%s\n\n", ip, netmask);


    // do nmap for active hosts with port 3260 open
    printf("Scanning network...\n");
    sprintf(command, "nmap -v -n %s%s%s -p 3260 | grep open | awk '/Discovered/ {print $NF}'", ip, "/", netmask);
    runCommand(command, alltargetsStr);

    // discover iscsi targets on hosts scanned successfully
    char *ptr;
    ptr = strtok(alltargetsStr, "\n");

    while(ptr != NULL) {
	printf("%s is a target.\n", ptr);
	sprintf(sendtargets,"iscsiadm -m discovery -t sendtargets -p %s | awk '{print $2}'",ptr);
	runCommand(sendtargets,iqn);
        printf("%s\n", iqn);

        sprintf(sql,"insert into Target(ipadd,iqn) values ('%s','%s');",ptr,iqn);

        rc = sqlite3_exec(db,sql,0,0,0);
	if (rc != SQLITE_OK){
	   printf("\nDid not insert successfully!\n");
           exit(0);
        }
	strcpy(sendtargets,"");
        strcpy(sql,"");
        ptr = strtok(NULL, "\n");
    }

    printf("\n\nLogging in to targets...");
    system("iscsiadm -m node --login");
    printf("\n\nAvailable partitions written to file \"%s\"\n", AV_DISKS);
    sleep(5);
    sprintf(command, "cat /proc/partitions > %s", AV_DISKS);
    system(command);
    system("cat /proc/partitions");

    makeVolume(0);

    runCommand("cat AvailableDisks.txt | grep sd[b-z] | awk '{print $4}'",disklist);

    char *ptr1;
    int counter = 1;    // to aidz: di ako sure dito ah.. pano kung nag delete then init_conf sure ba na 1 lagi tapos sunod sunod?
    ptr1 = strtok(disklist,"\n");

    while(ptr1 != NULL){
       strcat(assocvol,"/dev/vg");
       strcat(assocvol,disklist);
       strcat(assocvol,"/lv");
       strcat(assocvol,disklist);

       strcat(mountpt,"/mnt/lv");
       strcat(mountpt,disklist);

       sprintf(command1,"lvdisplay %s | grep 'LV Size' | awk '{print $3,$4}'",assocvol);

       runCommand(command1,avspace);

       // edit here not sure if working (assume: avspace = "12.3 GiB")
       double space_bytes = toBytes(avspace);

       sprintf(sql1,"update Target set assocvol = '%s', mountpt = '%s', avspace = %lf where tid = %d", assocvol, mountpt, space_bytes, counter);

       rc = sqlite3_exec(db,sql1,0,0,0);

       if (rc != SQLITE_OK){
           printf("Did not insert successfully!");
           exit(0);
       }

       strcpy(assocvol,"");
       strcpy(mountpt,"");
       strcpy(avspace,"");
       counter++;
       ptr1 = strtok(NULL,"\n");
    }

    printf("\n\nInitialization finished\n");
    printf("\n\nNow Watching CVFSTemp Directory\n");
    watch_copy();
}
