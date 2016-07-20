#include "includes.h"
#include "smbd/smbd.h"
#include "system/filesys.h"
#include "../librpc/gen_ndr/ndr_netlogon.h"
#include "auth.h"
#include <stdlib.h>

#undef DBGC_CLASS
#define DBGC_CLASS DBGC_VFS

#define ALLOC_CHECK(ptr, label) do { if ((ptr) == NULL) { errno = ENOMEM; goto label; } } while(0)


static int vfs_my_module_connect(vfs_handle_struct *handle, const char *service, const char *user)
{
	int ret = SMB_VFS_NEXT_CONNECT(handle, service, user);

	if (ret < 0) {
		return ret;
	}


	return 0;
}

static void vfs_my_module_disconnect(vfs_handle_struct *handle)
{
	SMB_VFS_NEXT_DISCONNECT(handle);
}

static const char *sharevolume(vfs_handle_struct *handle)
{
	const char *tmp_str = NULL;

	tmp_str = lp_parm_const_string(SNUM(handle->conn), "vfs_my_module", "lpath","/mnt/CVFSTemp");

	return tmp_str;
}

static const char *stripevolume(vfs_handle_struct *handle)
{
	const char *tmp_str = NULL;

	tmp_str = lp_parm_const_string(SNUM(handle->conn), "vfs_my_module", "spath","/mnt/CVFSTemp");

	return tmp_str;
}

static SMB_OFF_T vfs_my_module_get_file_size(vfs_handle_struct *handle,
				       const struct smb_filename *smb_fname)
{
	struct smb_filename *smb_fname_tmp = NULL;
	NTSTATUS status;
	SMB_OFF_T size;

	status = copy_smb_filename(talloc_tos(), smb_fname, &smb_fname_tmp);
	if (!NT_STATUS_IS_OK(status)) {
		size = (SMB_OFF_T)0;
		goto out;
	}

	if (SMB_VFS_STAT(handle->conn, smb_fname_tmp) != 0) {
		size = (SMB_OFF_T)0;
		goto out;
	}

	size = smb_fname_tmp->st.st_ex_size;
 out:
	TALLOC_FREE(smb_fname_tmp);
	return size;
}

static SMB_OFF_T vfs_my_module_maxsize(vfs_handle_struct *handle)
{
	SMB_OFF_T maxsize;

	maxsize = conv_str_size(lp_parm_const_string(SNUM(handle->conn),
					    "vfs_my_module", "maxsize", "536870912"));

	return maxsize;
}

static bool vfs_my_module_keep_dir_tree(vfs_handle_struct *handle)
{
	bool ret;

	ret = lp_parm_bool(SNUM(handle->conn), "vfs_my_module", "keeptree", True);

	return ret;
}

static mode_t vfs_my_module_directory_mode(vfs_handle_struct *handle)
{
	int dirmode;
	const char *buff;

	buff = lp_parm_const_string(SNUM(handle->conn), "vfs_my_module", "directory_mode", "777");

	if (buff != NULL ) {
		sscanf(buff, "%o", &dirmode);
	} else {
		dirmode=S_IRUSR | S_IWUSR | S_IXUSR;
	}

	return (mode_t)dirmode;
}

static mode_t vfs_my_module_subdir_mode(vfs_handle_struct *handle)
{
	int dirmode;
	const char *buff;

	buff = lp_parm_const_string(SNUM(handle->conn), "vfs_my_module", "subdir_mode", "777");

	if (buff != NULL ) {
		sscanf(buff, "%o", &dirmode);
	} else {
		dirmode=vfs_my_module_directory_mode(handle);
	}
	return (mode_t)dirmode;
}

static bool vfs_my_module_directory_exist(vfs_handle_struct *handle, const char *dname)
{
	SMB_STRUCT_STAT st;

	if (vfs_stat_smb_fname(handle->conn, dname, &st) == 0) {
		if (S_ISDIR(st.st_ex_mode)) {
			return True;
		}
	}

	return False;
}

static bool vfs_my_module_delete_dir(vfs_handle_struct *handle, const char *dname, const struct smb_filename *smb_fname){
    size_t len;
	mode_t mode;
	char *new_dir = NULL;
	char *tmp_str = NULL;
	char *token;
	char *tok_str;
	bool ret = False;
	char *saveptr;
    int j = 0;
	mode = vfs_my_module_directory_mode(handle);

	tmp_str = SMB_STRDUP(dname);
	ALLOC_CHECK(tmp_str, done);
	tok_str = tmp_str;

	len =strlen(dname)+1;
	new_dir = (char *)SMB_MALLOC(len + 1);
	ALLOC_CHECK(new_dir, done);
	*new_dir = '\0';
	if (dname[0] == '/') {
		/* Absolute path. */
		safe_strcat(new_dir,"/",len);
	}
	/* Create directory tree if neccessary */
	for(token = strtok_r(tok_str, "/", &saveptr); token;
	    token = strtok_r(NULL, "/", &saveptr)) {
		safe_strcat(new_dir, token, len);
			DEBUG(1, ("DELETE DIR: deleting dir %s\n", new_dir));
			if (SMB_VFS_NEXT_RMDIR(handle, new_dir) != 0) {
				DEBUG(1,("DELETE DIR: delete failed for %s with error: %s\n", new_dir, strerror(errno)));
				ret = False;
			}
		safe_strcat(new_dir, "/", len);
		mode = vfs_my_module_subdir_mode(handle);
		j++;
		DEBUG(1,("DELETE DIR COUNTER: %d\n", j));
		ret = True;
	}


done:
	SAFE_FREE(tmp_str);
	SAFE_FREE(new_dir);
	return ret;
}

static int vfs_my_module_create_dir(vfs_handle_struct *handle, const char *dname, const struct smb_filename *smb_fname)
{
	size_t len;
	mode_t mode;
	char *new_dir = NULL;
	char *tmp_str = NULL;
	char *token;
	char *tok_str;
	bool ret = False;
	char *saveptr;
    int i = 0;
	mode = vfs_my_module_directory_mode(handle);

	tmp_str = SMB_STRDUP(dname);
	ALLOC_CHECK(tmp_str, done);
	tok_str = tmp_str;

	len = strlen(dname)+1;
	new_dir = (char *)SMB_MALLOC(len + 1);
	ALLOC_CHECK(new_dir, done);
	*new_dir = '\0';
	if (dname[0] == '/') {
		/* Absolute path. */
		safe_strcat(new_dir,"/",len);
	}

	/* Create directory tree if neccessary */
	for(token = strtok_r(tok_str, "/", &saveptr); token;
	    token = strtok_r(NULL, "/", &saveptr)) {
		safe_strcat(new_dir, token, len);
		if (vfs_my_module_directory_exist(handle, new_dir))
			DEBUG(10, ("CREATE DIR: dir %s already exists\n", new_dir));
		else {
			DEBUG(1, ("CREATE DIR: creating new dir %s\n", new_dir));
			if (SMB_VFS_NEXT_MKDIR(handle, new_dir,mode) != 0) {
				DEBUG(1,("CREATE DIR: failed for %s with error: %s\n", new_dir, strerror(errno)));
				ret = False;
			}

		}
		safe_strcat(new_dir, "/", len);
		mode = vfs_my_module_subdir_mode(handle);
	i++;
	DEBUG(1,("CREATE DIR COUNTER: %d\n", i));
	ret = True;
	}
done:
	SAFE_FREE(tmp_str);
	SAFE_FREE(new_dir);
	return i;
}

static ssize_t vfs_my_module_pwrite(vfs_handle_struct *handle, files_struct *fsp,
			    const void *data, size_t n,
			    off_t offset)
{		connection_struct *conn = handle->conn;
	char *path_name = NULL;
       	char *temp_name = NULL;
	char *final_name = NULL;
	const struct smb_filename *smb_fname = fsp->fsp_name;
	struct smb_filename *smb_fname_final = NULL;
	const char *base;
	char *repository,*repositoryTemp = NULL;
	int i = 1;
	SMB_OFF_T maxsize, minsize;
	SMB_OFF_T file_size; /* space_avail;	*/
	bool exist;
	NTSTATUS status;
	ssize_t rc = -1;
    int count = 0;
    const char *share = "/mnt/share";
	repository = talloc_sub_advanced(NULL, lp_servicename(SNUM(conn)),
					conn->session_info->unix_name,
					conn->connectpath,
					conn->session_info->utok.gid,
					conn->session_info->sanitized_username,
					conn->session_info->info3->base.domain.string,
					sharevolume(handle));
	ALLOC_CHECK(repository, done);

	trim_char(repository, '\0', '/');

	if(!repository || *(repository) == '\0') {
		rc = SMB_VFS_NEXT_PWRITE(handle, fsp,data,n,offset);
		goto done;
	}

    file_size = vfs_my_module_get_file_size(handle, smb_fname);
    maxsize = vfs_my_module_maxsize(handle);

    if(maxsize > 0 && file_size > maxsize){
    repository = talloc_sub_advanced(NULL, lp_servicename(SNUM(conn)),
					conn->session_info->unix_name,
					conn->connectpath,
					conn->session_info->utok.gid,
					conn->session_info->sanitized_username,
					conn->session_info->info3->base.domain.string,
					stripevolume(handle));
	ALLOC_CHECK(repository, done);

	trim_char(repository, '\0', '/');

	if(!repository || *(repository) == '\0') {
		rc = SMB_VFS_NEXT_PWRITE(handle, fsp,data,n,offset);
		goto done;
	}
    }

	if (strncmp(smb_fname->base_name, repository,
		    strlen(repository)) == 0) {
		rc = SMB_VFS_NEXT_PWRITE(handle, fsp,data,n,offset);
		goto done;
	}

	base = strrchr(smb_fname->base_name, '/');
	if (base == NULL) {
		base = smb_fname->base_name;
		path_name = SMB_STRDUP("/");
		ALLOC_CHECK(path_name, done);
	}
	else {
		path_name = SMB_STRDUP(smb_fname->base_name);
		ALLOC_CHECK(path_name, done);
		path_name[base - smb_fname->base_name] = '\0';
		base++;
	}

	/* original filename with path */
	DEBUG(10, ("file transaction: fname = %s\n", smb_fname_str_dbg(smb_fname)));
	/* original path */
	DEBUG(10, ("file transaction: fpath = %s\n", path_name));
	/* filename without path */
	DEBUG(10, ("file transaction: base = %s\n", base));

    if (vfs_my_module_keep_dir_tree(handle) == True) {
		if (asprintf(&temp_name, "%s/%s", repository, path_name) == -1) {
			ALLOC_CHECK(temp_name, done);
		}
	} else {
		temp_name = SMB_STRDUP(repository);
	}

	ALLOC_CHECK(temp_name, done);
	exist = vfs_my_module_directory_exist(handle, temp_name);
	if (exist) {
		DEBUG(10, ("file transaction: Directory already exists\n"));
	} else {
		DEBUG(10, ("file transaction: Creating directory %s\n", temp_name));
		count = vfs_my_module_create_dir(handle, temp_name, smb_fname);
	}
	if (asprintf(&final_name, "%s/%s", temp_name, base) == -1) {
		ALLOC_CHECK(final_name, done);
	}

	/* Create smb_fname with final base name and orig stream name. */
	status = create_synthetic_smb_fname(talloc_tos(), final_name,
					    smb_fname->stream_name, NULL,
					    &smb_fname_final);
	if (!NT_STATUS_IS_OK(status)) {
		rc = SMB_VFS_NEXT_PWRITE(handle, fsp,data,n,offset);
		goto done;
	}

		TALLOC_FREE(smb_fname_final->base_name);
		smb_fname_final->base_name = talloc_strdup(smb_fname_final,
							   final_name);
		if (smb_fname_final->base_name == NULL) {
			rc = SMB_VFS_NEXT_PWRITE(handle, fsp,data,n,offset);
			goto done;
		}

	SMB_VFS_NEXT_RENAME(handle, smb_fname, smb_fname_final);
	rc = SMB_VFS_NEXT_PWRITE(handle, fsp,data,n,offset);

	if (rc != 0) {
		rc = SMB_VFS_NEXT_PWRITE(handle, fsp,data,n,offset);
		goto done;
	}

done:
    vfs_my_module_delete_dir(handle,path_name,smb_fname);
    while(count !=0){
    vfs_my_module_delete_dir(handle,path_name,smb_fname);
    SMB_VFS_NEXT_RMDIR(handle, path_name);
    count--;
    }
	SAFE_FREE(path_name);
	SAFE_FREE(temp_name);
	SAFE_FREE(final_name);
	TALLOC_FREE(smb_fname_final);
	TALLOC_FREE(repository);
	return rc;
}

static struct vfs_fn_pointers vfs_my_module_fns = {
    .connect_fn = vfs_my_module_connect,
	.disconnect = vfs_my_module_disconnect,
	.pwrite = vfs_my_module_pwrite,
};

NTSTATUS vfs_my_module_init(void){
	return smb_register_vfs(SMB_VFS_INTERFACE_VERSION, "vfs_my_module",
					&vfs_my_module_fns);
}
