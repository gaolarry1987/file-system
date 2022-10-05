#include "FSAPI.h"
#include "string.h"


#define blockSize 512
int initRoot(){
    fdDir* dr;
    int size= sizeof(*dr);
    dr = malloc(size);
    int numberOfEntries=0;
    char *buf = malloc(blockSize);
    memset(buf,0,blockSize);
    for(int i=0;i<(blockSize/size);i++){
        memset(dr->name,0,20);//clears out garbage data IMPORTANT!
        dr->d_reclen=48;
        dr->dirEntryPosition=0;
        dr->directoryStartLocation=6;
        dr->type=0;
        dr->numBytes=0;
        if(i==0){
            strcpy(dr->name,".");
            dr->type=2;
        }
        else if(i==1){
            strcpy(dr->name,"..");
            dr->type=2;
        }
        else if(i == 9){
            strcpy(dr->name,"NULL");
            dr->type=2;
            dr->directoryStartLocation=6;//points to root
        }
        //testing dirloop
        else{
            strcpy(dr->name,"empty");
        }
        memcpy(buf+(i*size),dr,size);
    }

    LBAwrite(buf,1,6);
    free(buf);
    return 1;
}




int dirLBA(){
    /*
    Global array CurrentDir holds the absolute path of the currect directory you are currently in, ex /root/dirA/dirB
    This array cycles through the array using "\" as a delimeter until the very end and returns the LBA of the current directory, example LBA of dirB
    */
  
    int LBA;
    fdDir* DE;
    int size;
    size= sizeof(*DE);
    DE = malloc(size);
    char *buf = malloc(blockSize);
    memset(buf,0,blockSize);
    int CurrBlk=6;//hardcodes because search always starts at root which starts at block 6
    //THE CRUX AND POSSIBLE REASON WHY AN INODE MAP MIGHT BE NEEDED IS BECAUSE THIS SEARCHES EVERY DIRECTORY TO THE BOTTOM DEPTH TO FIND THE LBA OF THE NEEDED DIRECTORY SO IT HAS TO 
    //LBA READ MULTIPLE TIMES WHICH IS INNEFFICIENT
    //Note as of now this entire functions assumes each dir is only 1 block long will need to change logic to determine how many blocks in the current dir that is being searched
    // Extract the first token


    
    //char* path = malloc(sizeof(CurrentDir)+1);
    //strcpy(path,CurrentDir);
    //path+=1;
     
    char * token = strtok(CurrentDir, "/");//delimeter is "/""
 
    while( token != NULL ) {
    LBAread(buf,1,CurrBlk);
  
      for(int i=0;i<10;i++){
      
        memmove(DE,buf+(size*i),size);
        if(DE->name==token){
            CurrBlk=DE->directoryStartLocation;
            i=10;//exits from for loop
        }    
    }
      token = strtok(NULL, " ");

   }
   LBA=CurrBlk;
   printf("\ndirLBA is done\n");
   return LBA;

   }

// . itself .. parent

void initChildDir(int x,int y){
  
    fdDir* dr;
    int size;
    size= sizeof(*dr);
    dr = malloc(size);
    strcpy(dr->name,"");
    int numberOfEntries=0;
    char *buf = malloc(blockSize);
    memset(buf,0,blockSize);
    for(int i=0;i<(blockSize/size);i++){
       
        memset(dr->name,0,20);//clears out garbage data IMPORTANT!
        dr->d_reclen=48;
        dr->dirEntryPosition=0;
        dr->directoryStartLocation=6;
        dr->type=0;
        dr->numBytes=0;
        if(i==0){
            strcpy(dr->name,".");
            dr->type=2;
            dr->directoryStartLocation=x;
            
        }
        else if(i==1){
            strcpy(dr->name,"..");
            dr->type=2;
            dr->directoryStartLocation=y;
        }
        else if(i == 9){
            strcpy(dr->name,"NULL");
            dr->type=2;
            dr->directoryStartLocation=6;//points to root, maybe theres a better value for NULL cap directory entry
        }
       //test
       else{
            strcpy(dr->name,"empty");
        }
       memcpy(buf+(i*size),dr,size);
    }
    

   
    LBAwrite(buf,1,x);
    free(buf);
}

void initBlk(int x){
    fdDir* dr;
    int size;
    size= sizeof(*dr);
    dr = malloc(size);
    strcpy(dr->name,"");
    int numberOfEntries=0;

    char *buf = malloc(blockSize);
    memset(buf,0,blockSize);
    for(int i=0;i<(blockSize/size);i++){
        memset(dr->name,0,20);//clears out garbage data IMPORTANT!
        if(i == 9){
            strcpy(dr->name,"NULL");
            dr->type=2;
            dr->directoryStartLocation=6;//points to root, maybe theres a better value for NULL cap directory entry
        }
       
        dr->d_reclen=48;
        dr->directoryStartLocation=x;
        //strcpy(dr->name, pathname);
        dr->type=0;
        dr->numBytes=512;
        memmove(buf+(i*size),dr,size);
    }
    LBAwrite(buf,1,x);

    free(buf);
}


int childLBA(char* buf){
    
    int pathType = pathCheck(buf);
   
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



int absolutePathChild(char *path){
    
        int countDelim = charCountInString(path,"/"); //1 less than number of tokens to target parent token
        if(strcmp(path,"/root")==0&&strlen(path)==5){
            return 6;
        }
        char str[sizeof(path)];
     
        //removes the slash from /root
        path+=1;
   
        char* token = strtok(path,"/");  //1st time =root    dir 1
      
        int lba=0; //might conflict with line 213??
        //root/dir1/
        int i=0;
     
         token = strtok(NULL,"/");  //dir1
          //strtok(token,"/");  //dir1
    

         while(token !=NULL){
        
              //if(token==NULL){
                   // break;
               // }
        //for(int i=0;i<countDelim;i++){
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
             token = strtok(NULL,"/");  //dir1
        } 
       
        return lba;
}
int dirLoop(char* token,int lba){
    
    fdDir* DE;
    int size = sizeof(*DE);
    DE=malloc(size);
    char*buf=malloc(blockSize);
    memset(buf,0,sizeof(buf));
   
    LBAread(buf,1,lba); 
    for(int i=0;i<10;i++){
        memcpy(DE,buf+(size*i),size);
      
        if(strcmp(token,DE->name)==0){
          
            int res= DE->directoryStartLocation;
            return res;
        }
        //NEW CODE
        if(i == 20){//the last entry of every block
            if(strcmp(DE->name,"CAP") == 0){//cap is the name of every 10th directory entry meaning theres one more directory block to search
                i = 0;//sets i to zero to restart the for loop to search th enew block
                LBAread(buf,1,DE->directoryStartLocation);//loads tje next block of a directory is a directory spans more than one block
            }
        }
        //IF DIRETORY SPANS MULTIPLE BLOCKS THEN LOADS THE NEXT BLOCK STOREB THE CAP DIRECOTY ENTRY ( EVERY 10TH OR LAST DE) AND STARTS THE SEARCH OVER
    }   
    return -1;
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


pathCheck(char* path){
  
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
    //converts child or relative to absolute path
        char* buf;
        int size = sizeof(CurrentDir)+sizeof(path)+2;   //+1 for / +1 for null terminating
        buf=malloc(size);
        strcpy(buf,CurrentDir);
        strcat(buf,"/");
        strcat(buf,path);
        return buf;
        }
        else if(pathType==1){
            //absolute
            strcpy(CurrentDir,path);
            return CurrentDir; //if absolute do nothing!
    
        }
}
 
void DelFCB(int LBA){//recieves the LBA of the first block of a file 
    FCB* fcb;
    int NextLBA;
    int size = sizeof(fcb);
    fcb = malloc(size);
    LBAread(fcb,1,LBA);
    NextLBA = fcb->FirstDataBlk;
    memset(fcb,0,size);
    LBAwrite(fcb,1,LBA);
    DelFileDataBlk(NextLBA);
    deallocate(LBA);//Frees up the fcb LBA in bitmap
    free(fcb);
}
void DelFileDataBlk(int LBA){
    if(LBA != 0){
        dataBlk* d_blk;
        int NextLBA;
        int size = sizeof(d_blk);
        d_blk = malloc(blockSize); //computer doesnt like this line throws ERR
        LBAread(d_blk,1,LBA);
        NextLBA = d_blk->NextLBA;
        memset(d_blk,0,size);  //error on this line
        deallocate(LBA);  
        DelFileDataBlk(NextLBA);//does recursion work w/o a return value
        free(d_blk);
    }
}

void DelDir(int LBA){
    fdDir * DE;//is there a difference between memmove and memcopy
    char* buf;
    buf = malloc(blockSize);
    int size = sizeof(DE);
    DE=malloc(size);
    LBAread(buf,1,LBA);
    int cap,prevLBA;
    for(int i = 0; i < blockSize/size; i++){
        memcpy(DE,buf+(size*i),size);
        if(DE->type == 1){
            DelFCB(DE->directoryStartLocation);
        }
        else if(strcmp(DE->name,"CAP") == 0){
            i=0;
            cap=DE->directoryStartLocation;
            memset(DE,0,blockSize);
            LBAwrite(buf,1,LBA);
            LBAread(buf,1,cap);
        }
        else if(DE->name== "NULL"){
            memset(buf,0,size);
            LBAwrite(buf,1,LBA);
        }
        else if(DE->type == 2){
            DelDir(DE->directoryStartLocation);
        }
    }
    free(DE);
    free(buf);
}
