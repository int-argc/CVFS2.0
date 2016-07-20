# CVFS2.0
CVFS 2.0 - Centralized Virtual File Server 2.0

---

This project demonstrates a storage environment running on the iSCSI protocol. The system presents a cluster of iSCSI targets as a single centralized storage and acts as a middleman between the iSCSI targets and the client network.

The system utilizes the Samba share to interface with the client network. Files transferred to the targets are presented to the clients through the Samba shared folder.

It runs on Linux.

## How to Install

Configuration of iSCSI targets as storage servers and configuring a separate machine as an iSCSI initiator is required. The system should be run in the iSCSI initiator.

This can run in a **HHHHYPERVISORRRR**.

### Dependencies

* open-iscsi
* sqlite3
* libsqlite3-dev
* nmap
* apache2
* php5-sqlite
* php5
* libapache2-mod-php5
* samba (3.6.25)


### Compatibility

The system is tested on the following machines:
* Ubuntu 12.04
* Ubuntu 12.04
* Ubuntu 12.04

### Instructions (brief)

Setup the iSCSI targets and the iSCSI initiator.

The samba vfs module in the source (`configs/vfs_my_module.c`) needs to be compiled and installed.

The samba configuration used in our testing is located in `configs/smb.conf`.

Make the following directories:
* `/mnt/Share`
* `/mnt/CVFSTemp`
* `/mnt/CVFSCache`
* `/mnt/CVFSFStore`
* `/mnt/CVFStorage`

Go to the root of the project then `make`. The executables are placed inside file_transaction. `./cvfs_driver` to run the system or `./cvfs_driver init` to initialize.

I know the instructions here are not clear. Refer to our non-existent wiki for more information.

## Notes

This project is not ready for enterprise use. It is created for research purposes.
