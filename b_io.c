/**************************************************************
* Class:  CSC-415-0# Fall 2022
* Names: Atharva Veer
*        Leiyi Gao
*        Sudip Lamichhane
*        Kiran Shrestha
* Student IDs:     AV (918551067)
*                  LG (920871013)
*                  SL (922140190)
*                   KS (921549353)
* GitHub Name:  prithvigrunge
* Group Name:   Make Run
* Project: Basic File System
*
* File: fsInit.c
*
* Description: This is the file contains API with all user interfaces.
*
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>			// for malloc
#include <string.h>			// for memcpy
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "b_io.h"
#include "mfs.c"
#include "fsLow.h"
//#include "fsInit.c"
#include <stdbool.h>

#define MAXFCBS 20
#define B_CHUNK_SIZE 512
#define initArray 50
#define DIRMAX_LEN		4096
#define O_RDONLY         00
#define O_WRONLY         01
#define O_RDWR           02




typedef struct b_fcb
	{
	/** TODO add al the information you need in the file control block **/

	char * buf;		//holds the open file buffer
	int location;   
	int index;		//holds the current position in the buffer
	int buflen;		//holds how many valid bytes are in the buffer
	int linuxFd;
	char *filename;

	fdDir* dir;

	} b_fcb;


uint64_t bufSize;

b_fcb fcbArray[MAXFCBS];

int startup = 0;	//Indicates that this has not been initialized

//Method to initialize our file system
void b_init ()
	{
	//init fcbArray to all free
	for (int i = 0; i < MAXFCBS; i++)
		{
		fcbArray[i].buf = NULL; //indicates a free fcbArray
		}
		
	startup = 1;
	}



//Method to get a free FCB element
b_io_fd b_getFCB ()
	{
	for (int i = 0; i < MAXFCBS; i++)
		{
		if (fcbArray[i].buf == NULL)
			{
			return i;		//Not thread safe (But do not worry about it for this assignment)
			}
		}
	return (-1);  //all in use
	}
	




// Interface to open a buffered file
// Modification of interface for this assignment, flags match the Linux flags for open
// O_RDONLY, O_WRONLY, or O_RDWR
b_io_fd b_open (char * filename, int flags)
	{
	
	
	int returnFd;
	
	if (startup == 0) b_init();  //Initialize our system


	returnFd = b_getFCB();

	//if file exists checker
	bool fileExists = true;
	

	//calling a new Directory
	fdDir* newDir;
	newDir = malloc(sizeof(newDir)); //bringing into memory

	b_fcb* fcb = &fcbArray[returnFd];
	

	//printf("Directory Entries %d\n", newDir->numberOfEntries);

	
	// Search through the total entries and check using name
	//  if found, then true, if not found then false
	//imprve search
	//Loop through total entries
	for(int i = 0; i < initArray; i++){


		//ompare path with the filename, if exits then true
		if(strcmp(newDir[i].path, filename) == 0){
			//printf("Directory Path Exists\n");

			return fileExists;


		} else {  //if file doesnt exits then false

			//printf("Directory Path Doesnt Exists\n");

			fileExists = false;

			
		}



	}



	// If the file doesnt exits and we are allowed to create it
	// We create a new directory entry

	if(fileExists != true ){
			

		//and it flags us to create
			if(flags & O_CREAT){

				
				//set all directory information
				// create a new directory entry
				//newDir = createDirEntry();

				printf("Created new entry\n");


			}else{
				
			}
			


	}


	
		
	

	//Setting information to our FIle Control Block
	fcb->filename = filename;
	fcb->index = 0;
	fcb->buflen = 0;
	fcb->buf = malloc(B_CHUNK_SIZE + 1);
	fcb->dir = newDir;


	


	return (returnFd);


} 













// Interface to seek function	
int b_seek (b_io_fd fd, off_t offset, int whence)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor


		}
		
		
	return (0); //Change this
	}





// b_write() WORKS VERY WELL
// Interface to write function	         // 15
int b_write (b_io_fd fd, char * buffer, int count)
	{
	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

		int moreInBuff = 0;  //if we have enough space in buffer to write
		int lessInBuff = 0;  //if we dont have enough in buffer to write



		//Begin
		
		//call a fcb pointer to the given file descriptor from fcbArray
		b_fcb* fcb = &fcbArray[fd];

		//calculate the bytes left in the buffer to write to
		int bytesLeft = fcb->buflen - fcb->index;                        //0
		printf("Bytes Left in our buffer to write to: %d\n", bytesLeft);




		//if we have enough space in buffer to write
		if(bytesLeft > count){      								 //      0 >  15  X
 
			moreInBuff = count;
			lessInBuff = 0;

		} else {  //if we dont have enough in buffer to write       // YES,  

			moreInBuff = bytesLeft;                                 //  moreInBuff = 0
			lessInBuff = count - moreInBuff;                           // 15 - 0 = 15   lessInBuff = 15


		}

		


		//VCB* newVcb;

		//if we have space in our buffer to write 
		if(moreInBuff > 0){                            // 0, so skip     


		// 1. Memcopy bytes to caller buffer from our buffer
			memcpy(buffer, fcb->buf + fcb->index, moreInBuff);


			fcb->index = fcb->index + moreInBuff;        //increment the index
			fcb->buflen = fcb->buflen - moreInBuff;      //decrement our buffer size
			fcb->location = fcb->location + moreInBuff;  //increment lba position

		}



		//  If requested bytes is more than our buffer size,

		

		

		if(lessInBuff > 0){                        //Start here, fresh block

		// 1. We first empty our buffer (Done using moreInBuff)
			fcb->index = 0;                       //index is 0
			fcb->buflen = 0;					  // we have empty buffer, so size 0


		// 2. Start with an empty buffer and LBAwrite to our buffer


			//we write a new block of data to our buffer
			LBAwrite(fcb->buf, 1 ,fcb->location); 

			fcb->buflen = B_CHUNK_SIZE; //out buffer is full now


			//memcopy the asked bytes from our buffer to the caller buff
			memcpy(buffer, fcb->buf, lessInBuff);


			//incre/ decre index and buff size
			fcb->index = fcb->index + lessInBuff;
			fcb->buflen = fcb->buflen - lessInBuff;




		}



	
	return moreInBuff + lessInBuff;

	

	
}



// Interface to read a buffer

// Filling the callers request is broken into three parts
// Part 1 is what can be filled from the current buffer, which may or may not be enough
// Part 2 is after using what was left in our buffer there is still 1 or more block
//        size chunks needed to fill the callers request.  This represents the number of
//        bytes in multiples of the blocksize.
// Part 3 is a value less than blocksize which is what remains to copy to the callers buffer
//        after fulfilling part 1 and part 2.  This would always be filled from a refill 
//        of our buffer.
//  +-------------+------------------------------------------------+--------+
//  |             |                                                |        |
//  | filled from |  filled direct in multiples of the block size  | filled |
//  | existing    |                                                | from   |
//  | buffer      |                                                |refilled|
//  |             |                                                | buffer |
//  |             |                                                |        |
//  | Part1       |  Part 2                                        | Part3  |
//  +-------------+------------------------------------------------+--------+
int b_read (b_io_fd fd, char * buffer, int count)
	{

	if (startup == 0) b_init();  //Initialize our system

	// check that fd is between 0 and (MAXFCBS-1)
	if ((fd < 0) || (fd >= MAXFCBS))
		{
		return (-1); 					//invalid file descriptor
		}

		
		//b_fcb* fcb = &fcbArray[fd];


		int bytesRead;		//total bytes read using LBAread variable		  
		int returnBytes;    //return variable to return to the console
		int blocks;	        // # of 512 byte blocks


		//total bytes left in our buffer
		int bytesLeft = fcbArray[fd].buflen - fcbArray[fd].index;
				 

		//initializing the values to perform a task on 
		int enoughInBuffer = 0;
		int copyDirectly = 0;
	   	int copyLeftover = 0;


		
   //Tracking the file size to check end of file into account
  //fcbArray[fd].fileSize = fcbArray[fd].fileSize - count;


  // If we have more than 0 bytes in the file, we read the file
   //if(fcbArray[fd].fileSize >= 0){}

	
	
	if (bytesLeft >= count){     //if more bytes are in buffer
	

		// we sent count to enough in buffer which later uses
		// memmove to move count bytes to buffer
		enoughInBuffer = count; 

	

	} else { 


		//If we dont have any bytes in our buffer, mainly used: 
		//if the requested amount is greater than B_CHUNK_SIZE.
		//or 
		//if count bytes is less than 512 and buffer does not have 
		//enough bytes
	 

		//First we empty our buffer into caller's buffer
		enoughInBuffer = bytesLeft; 


		
		//The remaining bytes are sent to this process
		//to perform buffering
		
		copyDirectly = count - bytesLeft;;

		

	} // end of if-else






	// XXXXXXXXXXX IF THERE ARE ENOUGH BYTES IN BUFFER XXXXXXXXXXX

	
	if (enoughInBuffer > 0){ 
	
		//using memmove() to move data to caller buffer
		memmove(buffer, fcbArray[fd].buf , count); 

		//incrementing our index
		fcbArray[fd].index = fcbArray[fd].index + enoughInBuffer; 

		//decrementing our remaining bytes
		bytesLeft -= count; 


	} 


	// XXXXXXXXXX IF COUNT BYTES ARE GREATER THAN OUR BUFFER XXXXXXXXXXX

	
	if (copyDirectly > 0){ 

		//since we have cleared out our buffer, we label as 0
		fcbArray[fd].buflen = 0; 	 
		fcbArray[fd].index = 0;  

		int bytesFromBLock; // total bytes from all blocks 

		int n = 0; //temp value to hold the bytes read


		
		for (int i = 0; i < blocks; i++){
			bytesRead = LBAread(buffer, 1, fcbArray[fd].location++);  //FIX THIS PART

			n = n + bytesRead; //total bytes read placeholder

		}


		bytesFromBLock = n; //all bytes read from the for-loop

		//assigning the remaining bytes
		copyLeftover = copyDirectly -  bytesFromBLock;


		
		if(copyDirectly > bytesFromBLock){
			bytesRead = LBAread(fcbArray[fd].buf, 1, fcbArray[fd].index++);   //FIX THIS PART

			if(bytesRead < 0){
				printf("Errorsssssssssss\n");
				exit(0);
			}

			fcbArray[fd].index = 0;   //index starts at 0
			//fcbArray[fd].buflen = bytesRead;  //size of buffer is 512 

			//moves bytes from our buffer tp caller's buffer
			memmove(buffer, fcbArray[fd].buf , copyLeftover);  


			//Modifying our buffer
			fcbArray[fd].index += copyLeftover; 
			fcbArray[fd].buflen = bytesRead - copyLeftover;

		}

	
	} //end of if-else

	//adding up all the bytes read from each process
	returnBytes = enoughInBuffer + copyDirectly;


	printf("\nGave %d bytes to the caller's buffer\n", returnBytes);
	return returnBytes; //return total bytes read
	




	 
	}





	
// Interface to Close the file	
int b_close (b_io_fd fd){

	//b_fcb* fcb = &fcbArray[fd];

	free(fcbArray[fd].buf);
	fcbArray[fd].buf = NULL;

	
	
	return 0;

}

































































