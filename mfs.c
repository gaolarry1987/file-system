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
#include "mfs.h"
#include "FSAPI.h"
#include "fsLow.h"
//#define blockSize 512

#define SIZE_OF_BLOCK 512


int dirLoop(char* token,int lba){

    fdDir* DE;
    int size = sizeof(DE);
    DE=malloc(size);
    char *buf=malloc(SIZE_OF_BLOCK);
    memset(buf,0,sizeof(buf));

    int  temp =  LBAread(buf,1,lba); 
    for(int i=0;i<10;i++){
        memcpy(DE,buf+(size*i),size);

        if(strcmp(token,DE->name)==0){

            int res= DE->directoryStartLocation;
            return res;
        }

        if(i == 20){//last entry of every block
            if(strcmp(DE->name,"LAST") == 0){
                LBAread(buf,1,DE->directoryStartLocation);
            }
        }
    }
    return -1;
}

int absolutePathChild(char *path){

       
        if(strcmp(path,"/root")==0 && strlen(path)==5){
            return 6;
        }
        char str[sizeof(path)];
        path+=1;

        char* token = strtok(path,"/");
        int lba=0; 
        //root
        int i=0;

         token = strtok(NULL,"/"); 
        while(token !=NULL){
            if(token==NULL){
                break;
            }
            if(i==0){ 
                lba = dirLoop(token,6);
            }
            else{
                lba = dirLoop(token,lba);
            }
            i++;
             token = strtok(NULL,"/");
        } 
       return lba;
}

int charCountInString(char* string,char check){
    int count = 0;
    char stringCharAt[strlen(string)];
    strcpy(stringCharAt,string);
    for(int i=0;i<strlen(string);i++){
        if(stringCharAt[i]==check){
            count++;
        }
    }
    return count;
}


int pathCheck(const char* path){
  
    if(charCountInString(path,'/')==0){
        return 0; //child
    }
    else if(strncmp("/root",path,strlen("/root"))==0){
        return 1; //absolute path
    }
    else if(charCountInString(path,'/')>0){
        return 2; //relative path
    }else{
        return 3; //invalid path
    }
}

char* convertToAbsolutePath(char* path){
    int pathType = pathCheck(path);
    if(pathType==0||pathType==2){

        char* buf;
        int size = sizeof(CurrentDir)+sizeof(path)+2;
        buf=malloc(size);
        strcpy(buf,CurrentDir);
        strcat(buf,"/");
        strcat(buf,path);
        return buf;
        }
        else if(pathType==1){

            strcpy(CurrentDir,path);
            return CurrentDir;

        }
}


int childLBA(const char* buf){
    
    int pathType = 2;
   
    if(pathType==0||pathType==2){
    
        //child, we need to know what directory we are in when called!!!!!!!
        int res =absolutePathChild(convertToAbsolutePath(buf));
        return res;
   
    } else if(pathType==1){
       
        //absolute path
 
      return absolutePathChild(buf);   
    
    } else{
       
        return 0;
    }

}
//char* defaultDirName  and   char* CurrentDir=malloc(1024);



int fs_mkdir(const char *pathname, mode_t mode){
    if(strcmp(pathname,"root")==0||strcmp(pathname,".")==0||strcmp(pathname,"..")==0||strcmp(pathname,"NULL")==0||strcmp(pathname,"LAST")==0||strcmp(pathname,"empty")==0){
        printf("Directory name Illegal\n");
        return 0;
    }
    fdDir* DE;
    int size,newBlk;
    size= sizeof(*DE);
    DE = malloc(size);
    char *buf = malloc(SIZE_OF_BLOCK);
    int CurrentDirLBA = absolutePathChild(CurrentDir);

    // dirLBA(); Finds the LBA of the current directory

    memset(buf,0,SIZE_OF_BLOCK);//sets every byte in buf to 0
    LBAread(buf,1,CurrentDirLBA);//reads the first block of data in the current directory

    //see if dir already exists and loop through entries to make sure theres no dupliates
    for(int i = 0;i<(SIZE_OF_BLOCK/size);i++){
        memmove(DE,buf+(size*i),size);
        if(strcmp(DE->name,pathname)==0){
            printf("\n Directory already exists please choose another name!\n");
            return 0;
        }

        if(i == 9){//block's last entry
            if(strcmp(DE->name,"LAST") == 0){//
                i = 0;//sets i to zero to restart the for loop to search th enew block
                LBAread(buf,1,DE->directoryStartLocation);//loads tje next block of a directory is a directory spans more than one block
            }
        }
    }

    LBAread(buf,1,CurrentDirLBA);//Rereads the first block of data of the current directory
    for(int i = 0; i <SIZE_OF_BLOCK/size;i++){
        memmove(DE,buf+(size*i),size);

        if(DE->type==0){
            DE->type=2;//PROBABLY NEED TO FILL IN  MORE FIELDS

            memset(DE->name,0,20);//clears out garbage data IMPORTANT!
            strcpy(DE->name,pathname);
            DE->directoryStartLocation=allocate();
            memcpy(buf+(size*i),DE,size);
            LBAwrite(buf,1,CurrentDirLBA);//reads the updated directory block onto the disk
            printf("Directory %s Sucessfully Created!\n",pathname);

            initChildDir(DE->directoryStartLocation,CurrentDirLBA);//double check LBA passing

            return 1;
        }
        else if( i == 9){
            if(strcmp(DE->name,"LAST") == 0){//last is the 10th directory entry and one more directory block to search
                i = 0;//restart from a new block
                LBAread(buf,1,DE->directoryStartLocation);
            }
            else if(strcmp(DE->name,"NULL") == 0){//end of the directory, no empty space, another block after it

                newBlk=allocate();

                DE->directoryStartLocation=newBlk;//new block

                memset(DE->name,0,20);//clean the trash

                strcpy(DE->name,"LAST");

                memcpy(buf+(size*9),DE,size);

                LBAwrite(buf,1,newBlk);//reads the updated directory block into the disk

                initBlk(newBlk);

                LBAread(buf,1,newBlk);

                DE->type=2;

                memset(DE->name,0,20);//clean trash

                strcpy(DE->name,pathname);

                DE->directoryStartLocation=allocate();//holds LBA of the new child directory

                initChildDir(DE->directoryStartLocation,dirLBA());//this maybe wrong

                memcpy(buf,DE,size);

                LBAwrite(buf,1,newBlk);

            }
        }
    }

    free(buf);
    free(DE);
}

int fs_rmdir(const char *pathname)
{ //rmDir deletes any file in CuurentDir
    if (strcmp(pathname, "root") == 0 || strcmp(pathname, ".") == 0 || strcmp(pathname, "..") == 0 || strcmp(pathname, "NULL") == 0 || strcmp(pathname, "LAST") == 0 || strcmp(pathname, "empty") == 0)
    {
        printf("Directory Illegal to remove\n");
        return 0;
    }
    fdDir *dr;
    int size = sizeof(*dr);
    dr = malloc(size);
    char *buf = malloc(SIZE_OF_BLOCK);
    memset(buf, 0, SIZE_OF_BLOCK);
    int CurrentDirLBA = childLBA(CurrentDir); //LBA of the current directory

    memset(buf, 0,SIZE_OF_BLOCK);
    LBAread(buf, 1, CurrentDirLBA); //loads the first block of the directory

    //removing direcrtory entry data
    for (int i = 0; i < 10; i++)
    {
        memmove(dr, buf + (size * i), size);
        if (strcmp(dr->name, pathname) == 0 && dr->type == 2)
        {
            //sets to default
            dr->type = 0;
            strcpy(dr->name, "empty");
            deallocate(dr->directoryStartLocation);
            memcpy((buf + (sizeof(*dr) * i)), dr, size);

            LBAwrite(buf, 1, CurrentDirLBA); //writes to actual needed directory being deleted and make bytes to 0
            printf("\n directory successfully removed \n");
        }
    }

    fdDir *DE;

    DE = malloc(size);

    dr = malloc(size);

    //formating block so existing directory entries come before empty directories
    for (int i = 0; i < 10; i++)
    {

        memmove(dr, buf + (size * i), size);
        if (strcmp(dr->name, "empty") == 0 || strcmp(dr->name, "") == 0 || strlen(dr->name) == 0 && strcmp(dr->name, "NULL") != 0 && strcmp(dr->name, "LAST") != 0 && dr->type == 0)
        {
            memmove(DE, buf + (size * (i + 1)), size);
            if (strcmp(DE->name, "empty") != 0 || strcmp(DE->name, "") != 0 || strcmp(DE->name, "NULL") != 0 || strcmp(DE->name, "LAST") != 0 && DE->type != 0)
            {
                //moves empty
                memcpy(buf + (size * (i + 1)), dr, size);
                memset(dr, 0, 48);
                memcpy(buf + (size * i), dr, size);
                //re-orgnize directories
                memcpy(buf + (size * i), DE, size);
            }
        }
    }

    LBAwrite(buf, 1, CurrentDirLBA); //write to the actual Dir that needed, and make bytes to 0

    free(buf);

    return 0;
}

fdDir *DE;

fdDir *fs_opendir(const char *name)
{ //for ls to open a stream to dir and return a dir poiter

   // cdBuf = malloc(blockSize);
   
  
    cdBuf = malloc(SIZE_OF_BLOCK);
   


    int size = sizeof(DE);
    DE = malloc(size);

   int lba = childLBA(name);
    DE->dirEntryPosition = 0;
    memset(DE->name, 0, 20); //clean the trash
    memset(cdBuf, 0, SIZE_OF_BLOCK);

    //temp
    

    LBAread(cdBuf, 1, lba);

    memmove(DE, cdBuf, size); //1st entry

    DE->dirEntryPosition = 48;
   
    return DE;
}
struct fs_diriteminfo *fs_readdir(fdDir *dirp)
{

    struct fs_diriteminfo *DI;
    int size = sizeof(*dirp);
    DI = malloc(259);
    int temp = dirp->dirEntryPosition;

    if (strcmp(dirp->name, ".") == 0 && strlen(dirp->name) == 1)
    {

        strcpy(DI->d_name, dirp->name);
        memmove(dirp, cdBuf + temp, size);
        dirp->dirEntryPosition = temp + 48;

        return DI;
    }

    strcpy(DI->d_name, dirp->name);

    memmove(dirp, cdBuf + temp, size);
    dirp->dirEntryPosition = temp + size;

    if (strcmp(DI->d_name, "NULL") == 0 || strcmp(DI->d_name, "empty") == 0 || strcmp(DI->d_name, "LAST") == 0)
    {
        return NULL;
    }

    return DI;
}

int fs_closedir(fdDir *dirp)
{ //used in display files

    free(cdBuf);
}

//
char *fs_getcwd(char *buf, size_t size)
{ //return current array

    strcpy(buf, CurrentDir);

    return buf;
}

int fs_setcwd(char *buf)
{
    char *newPath = malloc(sizeof(CurrentDir));

    int lba = childLBA(buf);

    if (lba == -1)
    {
        printf("no such file or directory\n");
        return 1;
    }

    convertToAbsolutePath(buf);
    // CurrentDir=convertToAbsolutePath(buf);

    return 0;
}
int fs_isFile(char *path)
{
    fdDir *DE;
    int size;
    size = sizeof(*DE);
    char *buf = malloc(SIZE_OF_BLOCK);
    DE = malloc(size);
    memset(buf, 0, SIZE_OF_BLOCK); //sets every byte in buf to 0
    LBAread(buf, 1, 6);        //I only ca do hard coded here
    for (int i = 0; i < 10; i++)
    { //Dir size is also hard coded
        memmove(DE, buf + (size * i), size);
        if (strcmp(DE->name, path) == 0)
        {
            if (DE->type == 0 || DE->type == 2)
            {
                return 0;
            }
            else if (DE->type == 1)
            {
                return 1;
            }
        }
    }
}
int fs_isDir(char *path)
{ //1 is for directory, 0 for others

    fdDir *DE;
    int size;
    size = sizeof(*DE);
    char *buf = malloc(SIZE_OF_BLOCK);
    DE = malloc(size);
    memset(buf, 0, SIZE_OF_BLOCK); //sets every byte in buf to 0
    LBAread(buf, 1, 6);        //hard coded
    for (int i = 0; i < 10; i++)
    {
        memmove(DE, buf + (size * i), size);
        if (DE->name == path)
        {
            if (strcmp(DE->name, path) == 0 && DE->type == 2)
            {
                return 1;
            }
            else if (DE->type == 1 || DE->type == 0)
            {
                return 0;
            }
        }
    }
}
 

    
int fs_delete(char *filename)
{ //removes a file
    int CurrDir = dirLBA();
    fdDir *DE;
    int size, FileLBA;
    size = sizeof(*DE);
    char *buf = malloc(SIZE_OF_BLOCK);
    DE = malloc(size);
    LBAread(buf, 1, CurrDir);

    for (int i = 0; i < SIZE_OF_BLOCK / size; i++)
    {
        memcpy(DE, buf + (i * size), size);
        if (strcmp(DE->name, filename) == 0)
        {
            DelFCB(DE->directoryStartLocation);
        }
        else if (strcmp(DE->name, "LAST") == 0)
        {
            LBAread(buf, 1, DE->directoryStartLocation);
            i = 0;
        }
        else if (strcmp(DE->name, "") == 0)
        {
            printf("/n File does not exist in current directory/n");
            return 0;
        }
    }
    DelFCB(FileLBA);
}



int fs_stat(const char *path, struct fs_stat *buf)
{
    char *buf1 = malloc(SIZE_OF_BLOCK); //read entire block
    fdDir *DE;
    int size = sizeof(*DE);
    DE = malloc(size);
    int lbaParent = 6;
    int lba = childLBA(path);
    memset(DE->name, 0, 20);
    memset(buf1, 0, SIZE_OF_BLOCK);

    LBAread(buf1, 1, lba);

    //LBAread(buf1,1,lbaParent);
    int lsBytes = 0;
    memset(DE, 0, size);
    memset(buf1, 0, SIZE_OF_BLOCK);
    LBAread(buf1, 1, lba);
    for (int i = 0; i < (SIZE_OF_BLOCK / size); i++)
    {
        memmove(DE, buf1 + (size * i), size);
        if (strcmp(path, "NULL") == 0)
        {
            break;
        }
        if (strcmp(path, "empty") != 0 || strcmp(path, "") != 0 || strcmp(path, "LAST") != 0)
        {
            lsBytes++;
        }
        break;
    }
    int totalBytes = size * lsBytes;
    buf->st_size = totalBytes;
    return 1;
}




int mv_dir(char *filename, char *path)
{
    fdDir *source;
    fdDir *destination;

    int size = sizeof(*destination);

    destination = malloc(size);

    //start at file search

    source = malloc(size);
    char *buf = malloc(SIZE_OF_BLOCK);
    memset(source, 0, size);
    memset(buf, 0, SIZE_OF_BLOCK);
    //locating the file directory entry
    int CurrentDirLBA = childLBA(CurrentDir);

    LBAread(buf, 1, CurrentDirLBA);
    for (int i = 0; i < 10; i++)
    {
        memmove(source, buf + (size * i), size);
        if (strcmp(source->name, "NULL") == 0)
        {
            printf("No Such Filename!\n");
            return 0;
        }

        if (strcmp(filename, source->name) == 0 && source->type == 1)
        {

            //check the path to new Dir is valid before deleting the file
            int dirValidator = childLBA(path);
            if (dirValidator != 0)
            {
                fdDir *check = malloc(size);
                memset(buf, 0, SIZE_OF_BLOCK);
                LBAread(buf, 1, dirValidator);
                for (int i = 0; i < 10; i++)
                {
                    memmove(check, buf + (size * i), size);
                    if (strcmp(check->name, "empty") == 0)
                    {
                        fs_isFile(filename);
                        break;
                    }
                    else if (strcmp(check->name, "NULL") == 0)
                    {
                        printf("no space in Destination Directory\n");
                        return 0;
                    }
                }
                break;
                //delete the file
            }
        }
    }

    memset(buf, 0, SIZE_OF_BLOCK);
    int destLba = childLBA(path);
    LBAread(buf, 1, destLba);
    memset(destination, 0, size);

    for (int i = 0; i < 10; i++)
    {
        memmove(destination, buf + (size * i), size);

        if (strcmp(destination->name, "empty") == 0)
        {

            memcpy(buf + (size * i), source, size);
            LBAwrite(buf, 1, destLba);
            printf("move sucessful!\n");
            return 1;
        }
    }
    return 0;
}


int fs_rmFile(const char *pathname)
{
    fdDir *dr;
    int size = sizeof(*dr);
    dr = malloc(size);
    char *buf = malloc(SIZE_OF_BLOCK);
    memset(buf, 0, SIZE_OF_BLOCK);
    int CurrentDirLBA = childLBA(CurrentDir); //Finds the LBA of the currentDir

    memset(buf, 0, SIZE_OF_BLOCK);
    LBAread(buf, 1, CurrentDirLBA); //loads the first block

    for (int i = 0; i < 10; i++)
    { //removing file entry data
        memmove(dr, buf + (size * i), size);
        if (strcmp(dr->name, pathname) == 0 && dr->type == 1)
        {
            //field to default
            memset(dr, 0, size);
            dr->type = 0;
            strcpy(dr->name, "empty");
            memcpy((buf + (size * i)), dr, size);
            LBAwrite(buf, 1, CurrentDirLBA);
        }
    }
    fdDir *DE;

    DE = malloc(size);

    dr = malloc(size);

    for (int i = 0; i < 10; i++)
    {

        memmove(dr, buf + (size * i), size);
        if (strcmp(dr->name, "empty") == 0 || strcmp(dr->name, "") == 0 || strlen(dr->name) == 0 && strcmp(dr->name, "NULL") != 0 && strcmp(dr->name, "LAST") != 0 && dr->type == 0)
        {
            memmove(DE, buf + (size * (i + 1)), size);
            if (strcmp(DE->name, "empty") != 0 || strcmp(DE->name, "") != 0 || strcmp(DE->name, "NULL") != 0 || strcmp(DE->name, "LAST") != 0 && DE->type != 0)
            {

                memcpy(buf + (size * (i + 1)), dr, size);
                memset(dr, 0, 48);
                memcpy(buf + (size * i), dr, size);
                memcpy(buf + (size * i), DE, size);
            }
        }
    }
    LBAwrite(buf, 1, CurrentDirLBA);
    free(buf);
    return 0;
}

