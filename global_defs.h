// global definitions

typedef char string[1024];

sqlite3 *db;

// available disks on the system
# define AV_DISKS   "AvailableDisks.txt"

#define DBNAME	"cvfs_db"
#define SHARELOC	"/mnt/Share"	// use full path please!!!!
#define TEMPLOC "/mnt/CVFSTemp"
#define CACHELOC "/mnt/CVFSCache"  // use real cache FULL PATH
