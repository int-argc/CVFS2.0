/*
   Name: Global Definitions
   Description:
     Global Definition for CVFS2.0 System
*/

typedef char String[1024];

#define AV_DISKS	"../file_transaction/AvailableDisks.txt"

#define DBNAME		"../Database/cvfs_db"
#define SHARE_LOC	"/mnt/Share"
#define	TEMP_LOC	"/mnt/CVFSTemp"
#define CACHE_LOC	"/mnt/CVFSCache"
#define ASSEMBLY_LOC	"/mnt"
#define STORAGE_LOC	"/mnt/CVFStorage"

// #define MAX_CACHE_SIZE	2

// #define STRIPE_SIZE 	134217728	//512MB
// #define L_SIZE	536870912

// configuration files
#define CACHE_CONF  "../configs/cache_size.conf"
#define STRIPE_CONF  "../configs/stripe_size.conf"

// global variables
extern int MAX_CACHE_SIZE;
extern long STRIPE_SIZE;
