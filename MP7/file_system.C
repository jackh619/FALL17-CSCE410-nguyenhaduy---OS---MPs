/*
     File        : file_system.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File System class.
                   Has support for numerical file identifiers.
 */

/*--------------------------------------------------------------------------*/
/* DEFINES */
/*--------------------------------------------------------------------------*/

    /* -- (none) -- */

/*--------------------------------------------------------------------------*/
/* INCLUDES */
/*--------------------------------------------------------------------------*/

#include "assert.H"
#include "console.H"
#include "file_system.H"


/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

FileSystem::FileSystem() {
    Console::puts("In file system constructor.\n");
    assert(false);

    memset(bitmap, 0, BITMAP_SIZE);
    size = 0;
    file_size = 0;
    files = NULL;
    file_locks = NULL;
    inode_index = 0;
}

/*--------------------------------------------------------------------------*/
/* FILE SYSTEM FUNCTIONS */
/*--------------------------------------------------------------------------*/

bool FileSystem::Mount(SimpleDisk * _disk) {
    Console::puts("mounting file system form disk\n");
    assert(false);
    disk = _disk;
    memset(super_block_data, 0, SUPER_BLOCK_SIZE);
    unsigned int num = SUPER_BLOCK_SIZE/512;
    
    for (int i = 0; i < num; i++) {
        disk->read(i, buff);
        memcpy(&super_block_data[i*512], buff, 512);
    }

    SuperBlock* superblock = (SuperBlock*) super_block_data;
    file_size = superblock->fsize;
    inode_index = superblock->inode_index;

    if (file_size != 0) {
        files = (File*) new File[file_size];
 
        for (int i = 0; i < file_size; i++) {
            files[i].file_id = superblock->files_no[i];
            files[i].file_size = superblock->files_size[i];
            files[i].inode_no = superblock->inode_no[i];
            files[i].inode_size = superblock->inode_size[i];
        }
    }
    
    int num_bitmap = BITMAP_SIZE/512;
    
    for (int i = 0; i < num_bitmap; i++) {
        disk->read(num+i, buff);
        memcpy(&bitmap[i*512],buff, BLOCK_SIZE);
    }

    return true;
}

bool FileSystem::Format(SimpleDisk * _disk, unsigned int _size) {
    Console::puts("formatting disk\n");
    assert(false);

    int num_of_blocks = _size/BLOCK_SIZE;
    if (num_of_blocks*BLOCK_SIZE < _size)
        num_of_blocks += 1; 
    
    memset(super_block_data, 0, SUPER_BLOCK_SIZE);
    SuperBlock* superblock = (SuperBlock*) super_block_data;

    superblock->fsize = 0;
    superblock->inode_index = 8;
    memset(superblock->files_no, 0, MAX_FILE_SIZE);
    memset(superblock->files_size, 0, MAX_FILE_SIZE);
    memset(superblock->inode_no, 0, MAX_FILE_SIZE);
    memset(superblock->inode_size, 0, MAX_FILE_SIZE);

    unsigned int num = SUPER_BLOCK_SIZE/512;
    for (int i = 0; i < num; i++) {
        memcpy(buff, &super_block_data[i*512],512);
        _disk->write(i, buff);
    }

    int num_bitmap = BITMAP_SIZE/512;
    for (int i = 0; i < num_bitmap; i++) {
        memset(buff, 0, 512);
        if (i == 0) {
            buff[0] = 0xFF;
        }
        _disk->write(num+i, buff);
    }
    
    unsigned int start_block_no = num+num_bitmap;
    memset(buff, 0, 512);
    for (int i = 0; i < num_of_blocks; i++) {
        _disk->write(i+start_block_no, buff);
    }
    
    return true;
}

File * FileSystem::LookupFile(int _file_id) {
    Console::puts("looking up file\n");
    assert(false);

    for (int i = 0; i < file_size; i++) {
        if (files[i].file_id  == _file_id) {
            return &files[i];
        }
    }

    return NULL;
}

bool FileSystem::CreateFile(int _file_id) {
    Console::puts("creating file\n");
    assert(false);

    Console::puts("creating file\n");
    for (int i = 0; i < file_size; i++) {
        if (files[i].file_id  == _file_id) {
                Console::puts("File Exists\n");
        return false;
        }
    }

    File* new_file = new File(this, disk);
    new_file->file_id = _file_id;
    new_file->inode_no = getFreeBlock();
    disk->read(new_file->inode_no, buff);
    I_Node* i_node = (I_Node*) buff;
    i_node->file_id = _file_id;
    i_node->next_block = 0;
    i_node->curr_size = 0;
    memset(i_node->blocks, 0, 500);
    new_file->inode_size += 1;
    disk->write(new_file->inode_no,buff);
    
    if (file_size == 0) {
        files = (File*) new File[1];
        files[0] = *new_file;
    } else {
    
        File* new_files = (File*) new File[file_size + 1];
        for (int i = 0; i < file_size; i++) {
            new_files[i] = files[i];
        }
        new_files[file_size] = *new_file;
        delete[] files;
        files = new_files;   
    }
    
    update_add_locks();

    file_size++;
    updateDisk();
    return true;
}

bool FileSystem::DeleteFile(int _file_id) {
    Console::puts("deleting file\n");
    assert(false);
}
