/*
    Volume Manager
    hiniwalay ko lang
*/

#include <stdio.h>
#include <string.h>
#include <sqlite3.h>
#include <syslog.h>
#include <stdlib.h>

#include "../Global/global_definitions.h"
#include "../disk_pooling/initial_configurations.h"
#include "make_volumes.h"

// estimate number of initial disks
#define INIT_DISKS_SIZE 50



void makeVolumeAdd(String ipadd){

    int counter = 0;   
   int rc;
   sqlite3* db;

    String avail = "", avail_out = "", comm = "", comm_out = "", sql1 = "", sql = "";
    rc = sqlite3_open(DBNAME, &db);

    sprintf(avail, "cat %s", AV_DISKS);
    runCommand(avail, avail_out);
    strcpy(comm, "cat /proc/partitions | grep sd[b-z] | awk '{print $4}'");
    runCommand(comm, comm_out);

    String currdisks[100];
    int o = 0;
    for(o = 0; o < 100; o++) {
	strcpy(currdisks[o], "");
    }

    char *pch = strtok(avail_out, "\n");
    while (pch != NULL){
	strcpy(currdisks[counter], pch);
	counter++;
	pch = strtok(NULL, "\n");
    }

    String newdisk = "";
    int counter2 = 0;
    pch = strtok(comm_out, "\n");
    while (pch != NULL){
	printf("CURRDISK := %s || PCH := %s\n", currdisks[counter2], pch);
	if (strcmp(currdisks[counter2], pch) != 0){
		strcpy(newdisk, pch);
		break;
	}
	counter2++;
	pch = strtok(NULL, "\n");
    }
printf("mewdisk = %s\n", newdisk);

    char pvs[255];
    char vgs[255];
    char lvs[255];
    char mkfs[255];
    char mount[255];
    char mkdir[255];
    char chmod[255];

       strcpy(pvs,"pvcreate /dev/");
       strcat(pvs, newdisk); //pvcreate /dev/sdb
       strcpy(vgs,"vgcreate vg");
       strcat(vgs, newdisk);
       strcat(vgs," /dev/");
       strcat(vgs, newdisk); //vgcreate vgsdb /dev/sdb
       strcpy(lvs,"lvcreate -l 100%FREE -n lv");
       strcat(lvs, newdisk);
       strcat(lvs," vg");
       strcat(lvs, newdisk); //lvcreate -l 100%FREE -n lvsdb vgsdb
       // make file system
       strcpy(mkfs, "mkfs -t ext4 /dev/vg");
       strcat(mkfs, newdisk);
       strcat(mkfs, "/lv");
       strcat(mkfs, newdisk);
       // mounting
       strcpy(mkdir, "mkdir /mnt/lv");
       strcat(mkdir, newdisk);
       strcpy(mount, "mount /dev/vg");
       strcat(mount, newdisk);
       strcat(mount, "/lv");
       strcat(mount, newdisk);
       strcat(mount, " /mnt/lv");
       strcat(mount, newdisk);
       strcpy(chmod, "chmod 777 -R /mnt/lv");
       strcat(chmod, newdisk);


       printf("PV = %s\n", pvs);
       printf("VG = %s\n", vgs);
       printf("LV = %s\n", lvs);
       // execute commands
       system(pvs);
       system(vgs);
       system(lvs);
       system(mkfs);
       system(mkdir);
       system(mount);
       system(chmod);


       String assocvol = "", command1 = "", avspace = "", mountpt = "";
       strcat(assocvol,"/dev/vg");
       strcat(assocvol,newdisk);
       strcat(assocvol,"/lv");
       strcat(assocvol,newdisk);
       
       printf("ASSOCVOL := %s\n", assocvol);
       strcat(mountpt,"/mnt/lv");
       strcat(mountpt,newdisk);
       printf("MOUNTPT := %s\n", mountpt);
       sprintf(command1,"lvdisplay %s | grep 'LV Size' | awk '{print $3,$4}'",assocvol);
       printf("COMMAND1 := %s\n", command1);
       runCommand(command1,avspace);

       // edit here not sure if working (assume: avspace = "12.3 GiB")
       double space_bytes = toBytes(avspace);

       sprintf(sql1,"update Target set assocvol = '%s', mountpt = '%s', avspace = %lf where ipadd = '%s'", assocvol, mountpt, space_bytes, ipadd);
       printf("just checking sql1 = %s\n", sql1);
    //    syslog(LOG_INFO, "DiskPooling: SQL1 = %s\n", sql1);
       rc = sqlite3_exec(db,sql1,0,0,0);

       if (rc != SQLITE_OK){
           syslog(LOG_INFO, "AddTarget: Error: Target is not recorded");
           exit(0);
       }

       strcpy(assocvol,"");
       strcpy(mountpt,"");
       strcpy(avspace,"");

    sqlite3_close(db);
}

void makeVolume(int update) {
    char buff[512];
    FILE *partitions = popen("cat /proc/partitions | awk '{print $4}' | grep 'sd[b-z]'", "r");

    char pvs[255];
    char vgs[255];
    char lvs[255];
    char mkfs[255];
    char mount[255];
    char mkdir[255];
    char chmod[255];
    String initialDisks[INIT_DISKS_SIZE];

    int i;

    while(fgets(buff, sizeof(buff), partitions) != NULL){
       buff[strlen(buff)-1] = '\0';

       // do not create volume again if already existing; alternative: pvdisplay
       //if (update) {
       //    if (containsDisk(initialDisks, buff)) {
       //        continue;
       //    }
      // }

       strcpy(pvs,"pvcreate /dev/");
       strcat(pvs, buff); //pvcreate /dev/sdb
       strcpy(vgs,"vgcreate vg");
       strcat(vgs, buff);
       strcat(vgs," /dev/");
       strcat(vgs, buff); //vgcreate vgsdb /dev/sdb
       strcpy(lvs,"lvcreate -l 100%FREE -n lv");
       strcat(lvs, buff);
       strcat(lvs," vg");
       strcat(lvs, buff); //lvcreate -l 100%FREE -n lvsdb vgsdb
       // make file system
       strcpy(mkfs, "mkfs -t ext4 /dev/vg");
       strcat(mkfs, buff);
       strcat(mkfs, "/lv");
       strcat(mkfs, buff);
       // mounting
       strcpy(mkdir, "mkdir /mnt/lv");
       strcat(mkdir, buff);
       strcpy(mount, "mount /dev/vg");
       strcat(mount, buff);
       strcat(mount, "/lv");
       strcat(mount, buff);
       strcat(mount, " /mnt/lv");
       strcat(mount, buff);
       strcpy(chmod, "chmod 777 -R /mnt/lv");
       strcat(chmod, buff);


       printf("PV = %s\n", pvs);
       printf("VG = %s\n", vgs);
       printf("LV = %s\n", lvs);
       // execute commands
       system(pvs);
       system(vgs);
       system(lvs);
       system(mkfs);
       system(mkdir);
       system(mount);
       system(chmod);
       // debug
    //    printf("comm=%s\n", pvs);
    //    printf("comm=%s\n", vgs);
    //    printf("comm=%s\n", lvs);
    //    printf("comm=%s\n", mkfs);
    //    printf("comm=%s\n", mkdir);
    //    printf("comm=%s\n", mount);
    //    printf("comm=%s\n", chmod);
    //    printf("\n");
    }
    pclose(partitions);
}
