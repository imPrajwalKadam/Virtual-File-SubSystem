#define _CRT_SECURE_NO_WARNINGS 
#include<stdio.h>
#include<stdlib.h>
#include<string.h>  
#include<unistd.h>
#define MAXINODE 50

#define READ 1
#define WRITE 2

#define MAXFILESIZE 1024

#define REGULAR 1
#define SPECIAL 2


#define START 0
#define CURRENT 1
#define END 2

typedef struct superblock
{
     int TotalIndoes;
     int FreeIndoes;
}SUPERBLOCK,*PSUPERBLOCK;

typedef struct inode
{
     char filename[50];
     int iNodeNumber;
     int FileSize;
     int FileActualSize;
     int FileType;
     char *Buffer;
     int LinkCount;
     int ReferenceCount;
     int Permission;
     struct inode *next;
}INODE,*PINODE,**PPINODE;

typedef struct filetable
{
     int ReadOffset;
     int WriteOffset;
     int Count;
     int Mode;
     PINODE ptrinode;//Pointer to inode
}FILETABLE,*PFILETABLE;



/////////////////////////////////////////////////
struct ufdt //User File Discripter Table
{
     FILETABLE ptrfiletable[50];
}ufdtobj;

//Global variables
SUPERBLOCK SUPERBLOCKobj;
PINODE Head = NULL;

void man(char *name)
{
     if(name == NULL)
     return;
     if(stricmp(name,"create")== 0)
     {
          printf("Description : Used to create new regular file\n"); 
          printf("Usage : create File_name Permission\n"); 
     }
     else if(stricmp(name,"read") == 0)
     {
          printf("Description : Used to read data from regular file\n"); 
          printf("Usage : read File_name No_Of_Bytes_To_Read\n"); 
     }
     else if(stricmp(name,"write") == 0)
     {
          printf("Description : Used to write into regular file\n");
          printf("Usage : write File_name\n After this enter the data that we want to write\n");
     }
     else if(stricmp(name,"ls") == 0)
     {
           printf("Description : Used to list all information of files\n");
           printf("Usage : ls\n");
     }
     else if(stricmp(name,"stat") == 0) 
     {
          printf("Description : Used to display information of file\n");
          printf("Usage : stat File_name\n"); 
     }
     else if(stricmp(name,"fstat") == 0)
     {
          printf("Description : Used to display information of file\n"); 
          printf("Usage : stat File_Descriptor\n"); 
     }
     else if(stricmp(name,"truncate") == 0)
     {
          printf("Description : Used to remove data from file\n");
          printf("Usage : truncate File_name\n");
     }
     else if(stricmp(name,"open") == 0)
     {
           printf("Description : Used to open existing file\n");
           printf("Usage : open File_name mode\n"); 
     }
     else if(stricmp(name,"close") == 0)
     {
          printf("Description : Used to close opened file\n");
          printf("Usage : close File_name\n");
     }
     else if(stricmp(name,"closeall") == 0)
     {
          printf("Description : Used to close all opened file\n"); 
          printf("Usage : closeall\n"); 
     }
     else if(stricmp(name,"lseek") == 0)
     {
          printf("Description : Used to change file offset\n");
          printf("Usage : lseek File_Name ChangeInOffset StartPoint\n"); 
     }
     else if(stricmp(name,"lseek") == 0)
     {
          printf("Description : Used to change file offset\n"); 
          printf("Usage : lseek File_Name ChangeInOffset StartPoint\n");
     }
     else if(stricmp(name,"rm") == 0)
     {
          printf("Description : Used to delete the file\n"); 
          printf("Usage : rm File_Name\n"); 
     }
     else
     {
          printf("ERROE : No mainnual entry available\n");
     }
}
void DisplayHelp()
{
        printf("ls : To List out all files\n"); 
        printf("clear : To clear console\n"); 
        printf("open : To open the file\n"); 
        printf("close : To close the file\n"); 
        printf("closeall : To close all opened files\n"); 
        printf("read : To Read the contents from file\n"); 
        printf("write :To Write contents into file\n"); 
        printf("exit : To Terminate file system\n"); 
        printf("stat : To Display information of file using name\n"); 
        printf("fstat :To Display information of file using file descriptor\n"); 
        printf("truncate : To Remove all data from file\n"); 
        printf("rm : To Delet the file\n"); 
}
int getFDFromName(char *name)
{
     int i = 0;
     while(i < 50)
     {
          if(ufdtobj.ptrfiletable[i] != NULL)
          {
               break;
          }
          i++;    
      }
      if(i == 0)
      {
           return -1;
      }
      else 
      {
           return i;
      }
}

PINODE Get_Inode(char * name)
{
     PINODE temp = Head;
     int i = 0;

     if(name == NULL)
     {
          return NULL;
     }
     while(temp != NULL)
     {
          if(strcmp(name,temp->filename) == 0)
          {
               break;
          }
          temp = temp->next;
     }
     return temp;
}
void CreateDILB()
{
     int i = 1;
     PINODE temp = Head;
     PINODE newn = NULL;
     while(i <= MAXINODE)
     {
          newn = (PINODE)malloc(sizeof(PINODE));

          newn->LinkCount=newn->ReferenceCount = 0;
          newn->FileType = newn->FileSize = 0;
          newn->Buffer = NULL;
          newn->next = NULL;
          newn->iNodeNumber = i;

          if(temp == NULL)
          {
               Head = newn;
               newn = Head;
          }
          else
          {
               temp->next = newn;
               temp = temp->next;
          }
          i++;
     }
     printf("DILB created Sucessfully");
}
void InitialiseSuperBlock() 
{
     int i = 0;
     while(i < MAXINODE)
     {
          ufdtobj.ptrfiletable[i].ptrinode = NULL;
          i++;
     }
     SUPERBLOCKobj.TotalIndoes = MAXINODE;
     SUPERBLOCKobj.FreeIndoes = MAXINODE;
}


int CreateFile(char *name, int Permission)
{
     int i = 0;
     PINODE temp = Head;
     if((name == NULL) || (Permission == 0) || (Permission > 3)) 
     {
          return -1;
     }
     if(SUPERBLOCKobj.FreeIndoes == 0) 
     {
          return -2;
     }
     (SUPERBLOCKobj.FreeIndoes)--; 

     if(Get_Inode(name) != NULL)
     {
          return -3;
     } 
     while(temp!= NULL) 
     {
          if(temp->FileType == 0)
               break;
          temp = temp->next;
     }
     while (i <  MAXINODE)
     {
          if(ufdtobj.ptrfiletable[i].ptrinode == NULL)
          break;
          i++;
     }
     ufdtobj.ptrfiletable[i] = (PFILETABLE)malloc(sizeof(FILETABLE));
     if(ufdtobj.ptrfiletable[i] == NULL) 
     { 
     return -4; 
     } 
     ufdtobj.ptrfiletable[i].Count = 1; 
     ufdtobj.ptrfiletable[i].Mode = Permission; 
     ufdtobj.ptrfiletable[i].ReadOffset = 0; 
     ufdtobj.ptrfiletable[i].ReadOffset = 0; 
    
     ufdtobj.ptrfiletable[i].ptrinode = temp; 
     strcpy(ufdtobj.ptrfiletable[i].ptrinode->filename,name); 
     ufdtobj.ptrfiletable[i].ptrinode->FileType = REGULAR; 
     ufdtobj.ptrfiletable[i].ptrinode->ReferenceCount = 1; 
     ufdtobj.ptrfiletable[i].ptrinode->LinkCount = 1; 
     ufdtobj.ptrfiletable[i].ptrinode->FileSize = MAXFILESIZE; 
     ufdtobj.ptrfiletable[i].ptrinode->FileActualSize = 0; 
     ufdtobj.ptrfiletable[i].ptrinode->Permission = Permission; 
     ufdtobj.ptrfiletable[i].ptrinode->Buffer=(char*)malloc(MAXFILESIZE); 
     memset(ufdtobj.ptrfiletable[i].ptrinode->Buffer,0,1024); 
     return i; 
}







int rm_File(char * name)
{
     int fd = 0;
     fd = getFDFromName(name);
     if(fd == -1)
     {
          return -1;
     }
     (ufdtobj.ptrfiletable[fd].ptrinode->LinkCount)--; 
    
     if(ufdtobj.ptrfiletable[fd].ptrinode->LinkCount == 0) 
     { 
     ufdtobj.ptrfiletable[fd].ptrinode->FileType = 0; 
     free(ufdtobj.ptrfiletable[fd].ptrinode); 
     } 

     ufdtobj.ptrfiletable[fd].ptrinode = NULL;
     (SUPERBLOCKobj.FreeIndoes)++;
     return 1;
}

int ReadFile(int fd, char *arr, int isize) 
{
     int readsize = 0;
     if(ufdtobj.ptrfiletable[fd].ptrinode == NULL)
     {
          return -1;
     }
     if(ufdtobj.ptrfiletable[fd].Mode !=READ && ufdtobj.ptrfiletable[fd].Mode !=READ+WRITE) 
     {
          return -2;
     }
     if(ufdtobj.ptrfiletable[fd].ptrinode->Permission != READ && ufdtobj.ptrfiletable[fd].ptrinode->Permission != READ+WRITE)
     {
          return -2;
     } 
      if(ufdtobj.ptrfiletable[fd].ReadOffset == ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize) 
      {
           return -3;
      }
      if(ufdtobj.ptrfiletable[fd].ptrinode->FileType != REGULAR) 
      {
           return -4;
      }
      readsize = (ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize) - (ufdtobj.ptrfiletable[fd].ReadOffset);
      if(readsize < isize)
      {
           strncpy(arr,(ufdtobj.ptrfiletable[fd].ptrinode->Buffer) + (ufdtobj.ptrfiletable[fd].ReadOffset),readsize);
           ufdtobj.ptrfiletable[fd].ReadOffset = ufdtobj.ptrfiletable[fd].ReadOffset + readsize;  
      }
      else
      {
           strncpy(arr,(ufdtobj.ptrfiletable[fd].ptrinode->Buffer) + (ufdtobj.ptrfiletable[fd].ReadOffset),isize);
           (ufdtobj.ptrfiletable[fd].ReadOffset) = (ufdtobj.ptrfiletable[fd].ReadOffset) + isize; 
      } 
      return isize;
}
int WriteFile(int fd, char *arr, int isize)
{
     if(((ufdtobj.ptrfiletable[fd].Mode)!=WRITE)&&((ufdtobj.ptrfiletable[fd].Mode)!=READ+WRITE))
     {
          return -1;
     }
     if(((ufdtobj.ptrfiletable[fd].ptrinode->Permission) !=WRITE) && ((ufdtobj.ptrfiletable[fd].ptrinode->Permission) != READ+WRITE))  
     {
          return -1;
     }
     if((ufdtobj.ptrfiletable[fd].WriteOffset) == MAXFILESIZE) 
     {
          return -2;
     }
     if((ufdtobj.ptrfiletable[fd].ptrinode->FileType) != REGULAR) 
     {
          return -3;
     }
      strncpy((ufdtobj.ptrfiletable[fd].ptrinode->Buffer) + (ufdtobj.ptrfiletable[fd].WriteOffset),arr,isize); 
     (ufdtobj.ptrfiletable[fd].WriteOffset) = (ufdtobj.ptrfiletable[fd].WriteOffset )+ isize;

     (ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize) = (ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize) + isize; 
     return isize;
}
int OpenFile(char * name, int mode)
{
     int i = 0;
     PINODE temp = NULL;

     if(name == NULL || mode <= 0)
     {
          return -1;
     }
     temp = Get_Inode(name); 
     if(temp == NULL) 
     {
          return -2;
     }
     if(temp->Permission < mode)
     {
          return -3;
     }
     while (i < MAXINODE)
     {
          if(ufdtobj.ptrfiletable[i].ptrinode == NULL) 
          break;
          i++;
     }
     ufdtobj.ptrfiletable[i].ptrinode = (PFILETABLE)malloc(sizeof(FILETABLE));
     if(ufdtobj.ptrfiletable[i] == NULL)
     {
          return -1;
     }
      ufdtobj.ptrfiletable[i].Count = 1; 
      ufdtobj.ptrfiletable[i].Mode = mode; 
      if(mode == READ + WRITE)
      {
           ufdtobj.ptrfiletable[i].ReadOffset = 0; 
           ufdtobj.ptrfiletable[i].WriteOffset = 0;
      }
      else if(mode == READ)
      {
           ufdtobj.ptrfiletable[i].ReadOffset = 0; 
      }
      else if(mode == WRITE)
      {
           ufdtobj.ptrfiletable[i].WriteOffset = 0;
      }
      ufdtobj.ptrfiletable[i].ptrinode = temp; 
      (ufdtobj.ptrfiletable[i].ptrinode->ReferenceCount)++;
      return i;
}
void CloseFilebyName(int fd)
{
      ufdtobj.ptrfiletable[fd].ReadOffset = 0;
      ufdtobj.ptrfiletable[fd].WriteOffset = 0; 
      (ufdtobj.ptrfiletable[fd].ptrinode->ReferenceCount)--;
}
int CloseFilebyName(char * Name)
{
     int i = 0;
     i = getFDFromName(Name);
     if(i == -1)
     {
          return -1;
     }
      ufdtobj.ptrfiletable[i].ReadOffset = 0;
      ufdtobj.ptrfiletable[i].WriteOffset = 0; 
      (ufdtobj.ptrfiletable[i].ptrinode->ReferenceCount)--; 

      return 0;
}
void CloseAllFile()
{
     int i = 0;
     while(i < MAXINODE)
     {
          if(ufdtobj.ptrfiletable[i] != NULL) 
          {
               ufdtobj.ptrfiletable[i].ReadOffset = 0; 
               ufdtobj.ptrfiletable[i].WriteOffset = 0; 
               (ufdtobj.ptrfiletable[i].ptrinode->ReferenceCount)--;
               break;
          }
          i++;
     }
}
int LseekFile(int fd, int size, int from)
{
      if((fd<0) || (from > 2)) return -1;
      if(ufdtobj.ptrfiletable[fd].ptrinode == NULL)
      return -1;
      if((ufdtobj.ptrfiletable[fd].Mode == READ) || (ufdtobj.ptrfiletable[fd].Mode == READ+WRITE)) 
      {
           if(from == CURRENT)
           {
                if(((ufdtobj.ptrfiletable[fd].ReadOffset) + size) > ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize)
                {
                     return -1;
                } 
                if(((ufdtobj.ptrfiletable[fd].ReadOffset) + size) < 0) 
                {
                     return -1;
                }
                (ufdtobj.ptrfiletable[fd].ReadOffset) = (ufdtobj.ptrfiletable[fd].ReadOffset) + size; 
           }
           else if(from == START)
           {
                if(size > (ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize))
                {
                     return -1;
                }
                if(size < 0)
                {
                     return -1;
                }
                if(((ufdtobj.ptrfiletable[fd].ReadOffset) + size) < 0) 
                {
                     return -1;
                }
                (ufdtobj.ptrfiletable[fd].ReadOffset) = (ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize) + size; 
           }
      }
      else if(ufdtobj.ptrfiletable[fd].Mode == WRITE) 
       {
            if(from == CURRENT)
            {
                 if(((ufdtobj.ptrfiletable[fd].WriteOffset) + size) > MAXFILESIZE)
                 {
                      return -1;
                 } 
                 if(((ufdtobj.ptrfiletable[fd].WriteOffset) + size) < 0)
                 {
                      return -1;
                 }
                 if(((ufdtobj.ptrfiletable[fd].WriteOffset)+size)>(ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize)) 
                 {
                      (ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize) = (ufdtobj.ptrfiletable[fd].WriteOffset) + size;
                      (ufdtobj.ptrfiletable[fd].WriteOffset)=(ufdtobj.ptrfiletable[fd].WriteOffset) + size; 
                 }
            }     
                 else if(from == START)
                 {
                      if(size > MAXFILESIZE) 
                      {
                           return -1;
                      }
                      if(size < 0)
                      {
                           return -1;
                      }
                      if(size > (ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize))(ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize) = size;
                      (ufdtobj.ptrfiletable[fd].WriteOffset) = size; 
                 }
                 else if(from == END)
                 {
                      if((ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize) + size > MAXFILESIZE)
                      {
                           return -1;
                      } 
                      if(((ufdtobj.ptrfiletable[fd].WriteOffset) + size) < 0) 
                      {
                           return -1;
                      }
                      (ufdtobj.ptrfiletable[fd].WriteOffset) = (ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize) + size; 
                 }
            }
            return 1;
}
void ls_file()
{
     int i =0;
     PINODE temp = Head;
     if(SUPERBLOCKobj.FreeIndoes == MAXINODE) 
     {
          printf("Error : There are no files\n");
          return; 
     } 
      printf("\nFile Name\tInode number\tFile size\tLink count\n");
      printf("---------------------------------------------------------------\n"); 
      while (temp != NULL)
      {
               if(temp->FileType != 0)
               {
                    printf("%s\t\t%d\t\t%d\t\t%d\n",temp->filename,temp->iNodeNumber,temp->FileActualSize,temp->LinkCount); 
               } 
               temp = temp->next;
      }
      printf("------------------------------------------------------------\n");
}
int fstat_file(int fd)
{
     PINODE temp = Head;
     int i = 0;

     if(fd < 0)
     {
          return -1;
     }
     if(ufdtobj.ptrfiletable[fd].ptrinode == NULL) 
     {
          return -2;
     }
      temp = ufdtobj.ptrfiletable[fd].ptrinode; 

      printf("\n---------Statistical Information about file----------\n"); 
      printf("File name : %s\n",temp->filename); 
      printf("Inode Number %d\n",temp->iNodeNumber); 
      printf("File size : %d\n",temp->FileSize); 
      printf("Actual File size : %d\n",temp->FileActualSize); 
      printf("Link count : %d\n",temp->LinkCount); 
      printf("Reference count : %d\n",temp->ReferenceCount);

     if(temp->Permission == 1) 
     printf("File Permission : Read only\n");
     else if(temp->Permission == 2)
     printf("File Permission : Write\n"); 
     else if(temp->Permission == 3)
     printf("File Permission : Read & Write\n"); 
     printf("----------------------------------------------------------\n\n");
     return 0;
} 
int stat_file(char *name) 
{
     PINODE temp = Head; 
     int i = 0;
     if(name == NULL)
     {
          return -1;
     }

     while(name != NULL)
     {
          if(strcmp(name,temp->filename) == 0)
          break;
          temp = temp->next;
     }
     if(temp == NULL)
     return -2;

     printf("\n---------Statistical Information about file----------\n"); 
     printf("File name : %s\n",temp->filename); 
     printf("Inode Number %d\n",temp->iNodeNumber); 
     printf("File size : %d\n",temp->FileSize); 
     printf("Actual File size : %d\n",temp->FileActualSize); 
     printf("Link count : %d\n",temp->LinkCount); 
     printf("Reference count : %d\n",temp->ReferenceCount);

     if(temp->Permission == 1)
     {
          printf("File Permission : Read only\n");
     }
     else if(temp->Permission == 2)
     {
           printf("File Permission : Write\n");
     }
      else if(temp->Permission == 3)
      {
           printf("File Permission : Read & Write\n");
      }
       printf("------------------------------------------------------\n\n"); 
       return 1;
}
int truncate_File(char *name)
{
     int fd = getFDFromName(name); 
     if(fd == -1)
     {
          return -1;
     }
     memset(ufdtobj.ptrfiletable[fd].ptrinode->Buffer,0,1024);
     ufdtobj.ptrfiletable[fd].ReadOffset = 0; 
     ufdtobj.ptrfiletable[fd].WriteOffset = 0; 
     ufdtobj.ptrfiletable[fd].ptrinode->FileActualSize = 0; 
     return 1;
}
int main()
{

     char *ptr = NULL;
     int ret = 0, fd = 0, count = 0;
     char Command[4][80],str[80],arr[1024];
     InitialiseSuperBlock();// WORK = Total inode = ?,free inodes = ?,Total number of block =?,free number of block =?.

     CreateDILB();//in 3rd partitian LinkList of indoes ,50 inodes link list created, thats link list's base address goes to Head pointer

     while(1)
     {
          fflush(stdin);
          strcpy(str,"");
          printf("\nPK's VFS:>");
          fgets(str,80,stdin);
          count = sscanf(str,"%s %s %s %s",Command[0],Command[1],Command[2],Command[3]);
          
          if(count == 1)
          {
               if(stricmp(Command[0],"ls")== 0)
               {
                    ls_file();
               }
               else if(stricmp(Command[0],"closeall") == 0)
               {
               //     CloseAllFile();
                    printf("All File Closed Sucessfully\n");
                    continue;
               }
               else if(strcmp(Command[0],"help")==0)
               {
                    DisplayHelp();
                    continue;
               }
               else if(strcmp(Command[0],"exit")==0)
               {
                    printf("terminating the marvellous Virtual Fuile \n");
                    break;
               }
               else{
                    printf("\nERROR: Command Not Found!!\n"); 
                    continue;
               }     
          }
          else if(count == 2)
          {
               if(stricmp(Command[0],"stat") == 0)
               {
                    ret = stat_file(Command[1]);
                    if(ret == -1)
                    printf("ERROR : Incorrect paramiters\n");
                    if(ret == -2)
                    printf("ERROR : their is no such file\n ");
                    continue;
               }
               else if(strcmp(Command[0],"fstat")==0)
               {
                    ret = fstat_file(atoi(Command[1]));
                    if(ret == -1)
                    printf("ERROR : thir is  no such file\n ");
                    continue;
               }
               else if(strcmp(Command[0],"close") == 0)
               {
                    ret = CloseFilebyName(Command[1]);
                    if(ret == -1)
                    printf("ERROR : their is no such file\n");
                    continue;
               }
               else if(stricmp(Command[0],"rm") == 0)
               {
                    ret = rm_File(Command[1]);//calls rm_File function
                    if(ret == -1)
                    printf("ERROR : their is no such file\n");
                    continue;
               }
               else if(stricmp(Command[0],"man") == 0)
               {
                    man(Command[1]);
               }
               else if(stricmp(Command[0],"write") == 0)
               {
                    fd = getFDFromName(Command[1]);
                    if(fd == -1)
                    {
                         printf("ERROR : Incorrect Paramiters\n");
                         continue;
                    }
                    printf("Enter the data : \n");
                    scanf("%[^'\n']",arr);
                    ret = strlen(arr);
                    if(ret == 0)
                    {
                         printf("ERROR : Incorrect paramiter\n");
                         continue;
                    }
                    ret = WriteFile(fd,arr,ret);
                    if(ret == -1)
                    printf("ERROR : Permission denide\n");
                    if(ret == -2)
                    printf("ERROR : Their is no Sufficient memory to write\n");
                    if(ret == -3)
                    printf("ERROR : it is not reguler file");
               }
               else if(stricmp(Command[0],"trunecat") == 0)
               {
                    if(ret == -1)
                    printf("ERROR : incorrect paramiter\n");
               }
               else
               {
                    printf("Command not found!!\n");
                    continue;
               }
          }
          else if(count == 3)
          {
               if(stricmp(Command[0],"create")==0)
               {
                    ret = CreateFile(Command[1],atoi(Command[2]));
                    if(ret >= 0)
                    printf("File is Sucessfully created with file discripter: %d\n",ret);
                    if(ret == -1)
                    printf("ERROR : Incorrect paramiter\n");
                    if(ret == -2)
                    printf("ERROR : their is no inodes\n");
                    if(ret == -3)
                    printf("ERROR : File is already exists\n");
                    if(ret == -4)
                    printf("ERROR : memory allocation faillure\n");
                    continue;
               }
               else if(stricmp(Command[0],"open") == 0)
               {
                  ret = OpenFile(Command[1],atoi(Command[2]));
                    if(ret >= 0)
                    printf("File is Sucessfully opened with the file discripter: %d\n");
                    if(ret == -1)
                    printf("ERROR : Incorretc paramiter");
                    if(ret == -2)
                    printf("ERROR : File not present\n");
                    if(ret == -3)
                    printf("ERROR : Permission denied\n");
                    continue;
               }
               else if(stricmp(Command[0],"read")==0)
               {
                   fd = getFDFromName(Command[1]);
                   if(fd == -1)
                   {
                   printf("Error : Incorrect parameter\n"); continue;
                   }
                   ptr = (char *)malloc(sizeof(atoi(Command[2]))+1);
                   if(ptr == NULL)
                   {
                   printf("Error : Memory allocation failure\n");
                   continue;
                   }
                   ret = ReadFile(fd,ptr,atoi(Command[2]));
                   if(ret == -1)
                   printf("ERROR : File not existing\n");
                   if(ret == -2)
                   printf("ERROR : Permission denied\n");
                   if(ret == -3)
                   printf("ERROR : Reached at end of file\n");
                   if(ret == -4)
                   printf("ERROR : It is not regular file\n");
                   if(ret == 0)
                   printf("ERROR : File empty\n");
                   if(ret > 0)
                   {
                   write(2,ptr,ret);
                   }
                   continue;
                   }
                   else
                   {
                   printf("\nERROR : Command no t found !!!\n");
                   continue;
               }
          }
          else if(count == 4)
          {
               if(stricmp(Command[0],"lseek") == 0)
               {
               fd = getFDFromName(Command[1]);
               if(fd == -1)
               {
               printf("Error : Incorrect parameter\n"); 
               continue;
               }
               ret = LseekFile(fd,atoi(Command[2]),atoi(Command[3]));
               if(ret == -1)
               {
               printf("ERROR : Unable to perform lseek\n");
               }
               }
               else
               {
               printf("\nERROR : Command not found !!!\n"); 
               continue;
               }
               }
               else   
               {
               printf("\nERROR : Command not found !!!\n");
               continue;
               }
          }
     return 0;          
}
     