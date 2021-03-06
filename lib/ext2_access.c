// ext2 definitions from the real driver in the Linux kernel.
#include "ext2fs.h"

// This header allows your project to link against the reference library. If you
// complete the entire project, you should be able to remove this directive and
// still compile your code.
//#include "reference_implementation.h"

// Definitions for ext2cat to compile against.
#include "ext2_access.h"



///////////////////////////////////////////////////////////
//  Accessors for the basic components of ext2.
///////////////////////////////////////////////////////////

// Return a pointer to the primary superblock of a filesystem.
struct ext2_super_block * get_super_block(void * fs) {
    // FIXME: Uses reference implementation.
    return fs + SUPERBLOCK_OFFSET;
}


// Return the block size for a filesystem.
__u32 get_block_size(void * fs) {
    // FIXME: Uses reference implementation.
    //return _ref_get_block_size(fs);
    return EXT2_BLOCK_SIZE(get_super_block(fs));
}


// Return a pointer to a block given its number.
// get_block(fs, 0) == fs;
void * get_block(void * fs, __u32 block_num) {
    // FIXME: Uses reference implementation.
    //return _ref_get_block(fs, block_num);
    return fs + get_block_size(fs) * block_num;
}


// Return a pointer to the first block group descriptor in a filesystem. Real
// ext2 filesystems will have several of these, but, for simplicity, we will
// assume there is only one.
struct ext2_group_desc * get_block_group(void * fs, __u32 block_group_num) {
    // FIXME: Uses reference implementation.
    //return _ref_get_block_group(fs, block_group_num);
    return fs + SUPERBLOCK_OFFSET + SUPERBLOCK_SIZE;
}


// Return a pointer to an inode given its number. In a real filesystem, this
// would require finding the correct block group, but you may assume it's in the
// first one.
struct ext2_inode * get_inode(void * fs, __u32 inode_num) {
    // FIXME: Uses reference implementation.
    //return _ref_get_inode(fs, inode_num);
    // determine offset
    struct ext2_super_block* super = get_super_block(fs);
    __u32 blocks_per_group = EXT2_BLOCKS_PER_GROUP(super);
    __u32 block_group = (inode_num - 1) / blocks_per_group;
    __u32 offset = (inode_num - 1) % blocks_per_group;
    // find the first indoe in group
    struct ext2_group_desc* group = get_block_group(fs, block_group);
    __u32 inode_table = group->bg_inode_table;
    void* first_entry = get_block(fs, inode_table);
    return first_entry + offset * EXT2_INODE_SIZE(super);
}



///////////////////////////////////////////////////////////
//  High-level code for accessing filesystem components by path.
///////////////////////////////////////////////////////////

// Chunk a filename into pieces.
// split_path("/a/b/c") will return {"a", "b", "c"}.
//
// This one's a freebie.
char ** split_path(char * path) {
    int num_slashes = 0;
    for (char * slash = path; slash != NULL; slash = strchr(slash + 1, '/')) {
        num_slashes++;
    }

    // Copy out each piece by advancing two pointers (piece_start and slash).
    char ** parts = (char **) calloc(num_slashes, sizeof(char *));
    char * piece_start = path + 1;
    int i = 0;
    for (char * slash = strchr(path + 1, '/');
         slash != NULL;
         slash = strchr(slash + 1, '/')) {
        int part_len = slash - piece_start;
        parts[i] = (char *) calloc(part_len + 1, sizeof(char));
        strncpy(parts[i], piece_start, part_len);
        piece_start = slash + 1;
        i++;
    }
    // Get the last piece.
    parts[i] = (char *) calloc(strlen(piece_start) + 1, sizeof(char));
    strncpy(parts[i], piece_start, strlen(piece_start));
    return parts;
}


// Convenience function to get the inode of the root directory.
struct ext2_inode * get_root_dir(void * fs) {
    return get_inode(fs, EXT2_ROOT_INO);
}


// Given the inode for a directory and a filename, return the inode number of
// that file inside that directory, or 0 if it doesn't exist there.
// 
// name should be a single component: "foo.txt", not "/files/foo.txt".
__u32 get_inode_from_dir(void * fs, struct ext2_inode * dir, 
        char * name) {
    // FIXME: Uses reference implementation.
    //return _ref_get_inode_from_dir(fs, dir, name);
    int i;
    void* block;
    void* dir_ptr;
    struct ext2_dir_entry_2* cur_dir;
    __u32 block_size = get_block_size(get_super_block(fs));
    for(i=0;i<EXT2_NDIR_BLOCKS;i++)
      {
	block = get_block(fs, dir->i_block[i]);
	cur_dir = (struct ext2_dir_entry_2*) block;
	for(dir_ptr = block;
	    dir_ptr < (block + block_size) && cur_dir->rec_len;
	    dir_ptr += cur_dir->rec_len)
	  {
	    cur_dir = (struct ext2_dir_entry_2*)dir_ptr;
	    if(strncmp(name, cur_dir->name, cur_dir->name_len) == 0)
	      return cur_dir->inode;
	  }
      }
    return 0;
}


// Find the inode number for a file by its full path.
// This is the functionality that ext2cat ultimately needs.
__u32 get_inode_by_path(void * fs, char * path) {
    // FIXME: Uses reference implementation.
    //return _ref_get_inode_by_path(fs, path);
    int count = 0;
    __u32 inode_from_dir;
    struct ext2_inode* root_dir = get_root_dir(fs);
    struct ext2_inode* next = root_dir;
    char** path_atoms = split_path(path);
    char* slash;
    for(slash = path; slash != NULL; slash = strchr(slash+1, '/'))
      {
	inode_from_dir = get_inode_from_dir(fs, next, path_atoms[count]);
	if(!inode_from_dir)
	  return 0;
	next = get_inode(fs, inode_from_dir);
	count++;
      }
    return inode_from_dir;
}

