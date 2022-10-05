/**************************************************************
* Class:  CSC-415-0# Fall 2021
* Names: Atharva Veer
* 		 Leiyi Gao
* 		 Sudip Lamichhane
* 		 Kiran Shrestha
* Student IDs:     AV (918551067)
* 				   LG ()
* 				   SL (922140190)
* 				   KS (921549353)
* GitHub Name:  prithvigrunge
* Group Name:   Make Run
* Project: Basic File System
*
* File: fsInit.c
*
* Description: Main driver for file system assignment.
*
* This file is where you will start and initialize your system
*
**************************************************************/

#ifndef _B_IO_H
#define _B_IO_H
#include <fcntl.h>

typedef int b_io_fd;

//DirectoryEntry dirEntryArr[];

b_io_fd b_open (char * filename, int flags);
int b_read (b_io_fd fd, char * buffer, int count);
int b_write (b_io_fd fd, char * buffer, int count);
int b_seek (b_io_fd fd, off_t offset, int whence);
int b_close (b_io_fd fd);

#endif
