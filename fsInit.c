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

#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>

#include "fsLow.h"
#include "mfs.h"
#include "b_io.h"

#define SIZE_OF_BLOCK 512

#define initArray 50
char *fileSystem;

//VOLUME CONTROL BLOCK STRUCT
typedef struct VCB
{

	char title[100];
	char header[20];

	uint64_t blockSize; //size of block

	uint64_t volumeSize; //Volume Size in bytes

	uint64_t magicNumber; //signature

	uint64_t numOfBlocks; //number of blocks

	uint64_t rootDirectory; //pointer to root directory

	uint64_t freeSpaceStart; //Free Space Block

	uint64_t totalDirEntries;

	uint64_t totalDirEntryBlocks;

} VCB;



//DIRECTORY STRUCT
typedef struct DirectoryEntry
{

	fdDir dir;
	int index;
	time_t created;

} DirectoryEntry;



// INITIALIZE FREE SPACE FUNCTION
int initBitMap(VCB *vcb, uint64_t numBlocks)
{

	// //printf(" \n block number %ld \t ",numBlocks);
	// //void *calloc(size_t nmemb, size_t size);

	// //convert into bytes
	int convertIntoBytes = (numBlocks + 7) / 8;

	 int convertIntoBlock = (convertIntoBytes + (SIZE_OF_BLOCK-1)) / SIZE_OF_BLOCK;
	//printf("total block = \t %d \n ",convertIntoBlock);
	
	 char *numNeedMemory = malloc(convertIntoBlock * SIZE_OF_BLOCK);
	//printf("total memory =\t %ld",sizeof(numNeedMemory));
	int pushFreeSpace = LBAwrite(numNeedMemory, convertIntoBlock, 1);

	// //assign free space start to VCB

	vcb->freeSpaceStart = pushFreeSpace;
	printf("VCB free space start %ld\n", vcb->freeSpaceStart);

	return pushFreeSpace;

} //END OF REE SPACE





//INITIALIZE OUR ROOT DIRECTORY
void initRoot(VCB *newVcb, fdDir* rootDir)
{

	//a. First again, you need memory – how much?

	//fdDir* rootDir = calloc(newVcb->numOfBlocks, SIZE_OF_BLOCK);

	/*DirectoryEntry *rootDir = calloc(newVcb->numOfBlocks, SIZE_OF_BLOCK);
	*/


	// DirectoryEntry *rootDir = malloc(sizeof(DirectoryEntry));



	// c
	//int bytes = sizeof(fdDir) * initArray;
	int bytes = sizeof(fdDir) * initArray;

	rootDir = malloc(bytes);

	//rootDir->dir->numberOfEntries = initArray;
	//printf("Bytes Needed: %d\n", bytes);

	// d
	int blocks = (bytes / SIZE_OF_BLOCK);

	//printf("total block = %d",blocks);

	if (bytes % SIZE_OF_BLOCK != 0)
	{

		blocks++;
	}
	//printf("Blocks needed: %d\n", blocks);

	int bytesForDirEntry = blocks * SIZE_OF_BLOCK;
	//printf("Bytes for Directory Entry: %d\n", bytesForDirEntry);

	int totalEntries = bytesForDirEntry / sizeof(DirectoryEntry);
	//printf("\n  Total Entries: %d\n", totalEntries);

	rootDir->numberOfEntries = totalEntries;

	//rootDir[50];
	for(int i=0;i<=initArray;i++){
		rootDir[i].index=i;
		//rootDir[i].created=time(0);
		rootDir[i].parentLocation=i;
		strcpy(rootDir[i].path,"/");
		//if 1 its directory   if its
		//rootDir->type=1;
		//rootDir->dirdir.type=1;//if 1 its directory
		rootDir[i].numberOfEntries = totalEntries;
		//printf(" indexs postion %d \n",rootDir[i].parentLocation);
	}

	

	// e


	strcpy(rootDir[0].name,".");
	strcpy(rootDir[1].name,"..");

	//printf("%s checking out dir name \n  %s second one checking out   \n  \n",rootDir[0].name,rootDir[1].path);


	//printf("Exit\n");
	

	//implement this
	//LBAwrite()
	//printf("total block read %ld",newVcb->freeSpaceStart);
	int temp =LBAwrite (newVcb, blocks, newVcb->freeSpaceStart);
	newVcb->freeSpaceStart=temp;
	//printf(" tired of  hcekcing out %ld \n ",newVcb->freeSpaceStart);
}













//INITIALIZING OUR VCB
//ititialize everything to 0 if empty, must be in a known state

void initVCB(VCB *newVcb, uint64_t volumeSize, uint64_t blockSize)
{

	strcpy(newVcb->title, "Volume Control Block"); //title
	strcpy(newVcb->header, PART_CAPTION);		   //header

	newVcb->magicNumber = 314156; //magic number

	uint64_t totalBlocks = volumeSize / blockSize; //total blocks
	//printf("total Blocks  %ld\n", totalBlocks);

	////return number of blocks to our VCB
	newVcb->numOfBlocks = totalBlocks; //num of blocks

	//return volume size to our VCB
	newVcb->volumeSize = volumeSize; //volume
}

int initFileSystem(uint64_t numberOfBlocks, uint64_t blockSize)
{
	//printf("Initializing File System with %ld blocks with a block size of %ld\n", numberOfBlocks, blockSize);
	/* TODO: Add any code you need to initialize your file system. */

	//allocate the memory 512
	//malLoc a block of memory to our vcb pointer
	VCB *newVCB = malloc(blockSize);
	VCB *checking = malloc(blockSize);
	uint64_t bytesPosition;

	//Testing
	printf("checking before  %ld", newVCB->magicNumber);

	//LBAread block 0
	uint64_t read = LBAread(newVCB, 1, 0);
	printf("checking after %ld", newVCB->magicNumber);
	printf("\n Read Blocks: %ld\n", read);

	printf("\n new vcb  %ld", checking->magicNumber);

	//todo we will  change latter

	//if magic number matches, the VCB is initialized

	if (newVCB->magicNumber == 3141555)
	{
		printf("Volume Initialized\n");
	}
	else //if VCB isn't initialized, begin initializing VCB,
	{	 //free space, and root directory

		printf("\nVolume Not Initialized based on Magic Number\n\n");

		uint64_t volume = numberOfBlocks * blockSize;


		//call initVCB to initialize our VCB
		initVCB(newVCB, volume, blockSize);

		/*
		printf("Title: %s\n", newVCB->title);
		printf("Header: %s\n", newVCB->header);
		printf("Number of Blocks: %ld\n", newVCB->numOfBlocks);
		printf("Volume: %ld\n", newVCB->volumeSize);
		printf("Magic Number: %ld\n", newVCB->magicNumber);
		*/

		//call initBitMap() to initialize free space
		 int freeSpace = initBitMap(newVCB, newVCB->numOfBlocks);

		// printf("freespace returining from function \t %d\n",freeSpace);

		// printf(" free space %d",freeSpace);

		//call initRoot() to initialize the root directory
		 fdDir* rootDir;
		initRoot(newVCB, rootDir);

			//return free space to our VCB
	//	newVCB->freeSpaceStart = freeSpace;

			//return root directory to our VCB
			//	newVCB->rootDirectory = rootDir;

		//using LBAwrite() to write VCB to block 0
		//int64_t wrote = LBAwrite(newVCB, 1, 0);
		//printf("Wrote VCB to Block 0: %ld\n", wrote);
	}

	return 0;
}

void exitFileSystem()
{
	printf("System exiting\n");
}


