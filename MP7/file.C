/*
     File        : file.C

     Author      : Riccardo Bettati
     Modified    : 2017/05/01

     Description : Implementation of simple File class, with support for
                   sequential read/write operations.
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
#include "file.H"

/*--------------------------------------------------------------------------*/
/* CONSTRUCTOR */
/*--------------------------------------------------------------------------*/

File::File() {
    /* We will need some arguments for the constructor, maybe pointer to disk
     block with file management and allocation data. */
    Console::puts("In file constructor.\n");
    assert(false);
}

File::File(FileSystem* _file_system, SimpleDisk * _disk) {
    Console::puts("In file constructor.\n");
    file_id = 0;
    file_size = 0;
    inode_no = 0;
    inode_size = 0;
    cur_size = 0;
    cur_block = 0; 
    cur_position = 0;
    file_system = _file_system;
    disk = _disk;
}

/*--------------------------------------------------------------------------*/
/* FILE FUNCTIONS */
/*--------------------------------------------------------------------------*/

int File::Read(unsigned int _n, char * _buf) {
    Console::puts("reading from file\n");
    assert(false);
}


void File::Write(unsigned int _n, const char * _buf) {
    Console::puts("writing to file\n");
    assert(false);
}

void File::Reset() {
    Console::puts("reset current position in file\n");
    assert(false);
    
}

void File::Rewrite() {
    Console::puts("erase content of file\n");
    assert(false);
}


bool File::EoF() {
    Console::puts("testing end-of-file condition\n");
    assert(false);
}
