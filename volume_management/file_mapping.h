/*
   Description:
     Definitions of File Mapping Submodule to be shared to other modules.
*/


void file_map(String fullpath, String filename);

void file_map_stripe(String *fullpaths, String *filenames, int parts);

void file_map_cache(String filename, String event_name);

void update_target_size_delete(String filename, String fileloc);
