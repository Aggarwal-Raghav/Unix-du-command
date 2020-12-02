//du command with -h -c -a --max-depth= flag in C  
//Raghav Aggarwal PES1201800312

//import c in_built libraries
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//used for define the proper size of datatypes in c
#include <limits.h>
//for formatting directory entries
#include <dirent.h>
//contains the stat structure for computing the blocks for each file 
#include <sys/stat.h>
#include <unistd.h>
//for block size, contains blkcnt_t
//blkcnt_t -> used for file block count
#include <sys/types.h>

//setting macros for file size on disk in human readable form
#define gigaByte (1 << 21)      //G is symbol in output for gigabyte 
#define megaByte (1 << 11)      //M is symbol in output for megaabyte 
#define kiloByte (1 << 1)       //K is symbol in output for kiloabyte 
//1 block -> 512 byte (taken from google.com)
#define bytesPerBlock 512
//max buffer size
#define MAX 10

//computing the block size
//blkcnt_t is the datatype in <sys/types.h>
//function to calculate the size of block in human readable form
void blockMath(char *entryName, blkcnt_t numBlocks, int humanFlag) 
{
    int size;
    char label;
    char sizeBuf[MAX];

    // if -h flag is not set
    if (!humanFlag) 
        printf("%-8ld%s\n", (numBlocks + 1) / 2, entryName);

    // if -h flag is given which return output in human readable form
    else 
    {
        if (numBlocks >= gigaByte) 
        {
            size = numBlocks / gigaByte;
            label = 'G';
        }

        else if (numBlocks >= megaByte) 
        {
            size = numBlocks / megaByte;
            label = 'M';
        }

        else if (numBlocks >= kiloByte) 
        {
            size = numBlocks / kiloByte;
            label = 'K';
        }

        else 
        {
            size = numBlocks * bytesPerBlock;
            label = ' ';
        }

        //Writing formatted output to sizeBuf, 10 here specify number of byte written to sizeBuf
        snprintf(sizeBuf, MAX, "%d%c", size, label);
        printf("%-8s%s\n", sizeBuf, entryName);
    }
}

//function is used to traverse in directory and file usint #include<sys/types.h> and it returns the block count for that directory
blkcnt_t TraverseDir(char *entryName, int printFileFlag, int maxDepthFlag, int maxDepthValue, int humanFlag) 
{
    blkcnt_t totalSize = 0;
    struct stat st;
    char buffer[PATH_MAX];
    DIR *dir;
    struct dirent *newEntry;

    //if directory name is wrong or path is not correct
    if (lstat(entryName, &st)) 
    {
        perror(entryName);
        exit(-1);
    }

    //for opening and traversing the directory
    if (S_ISDIR(st.st_mode) && (dir = opendir(entryName))) 
    {
        while ((newEntry = readdir(dir)))
            if (strcmp(newEntry->d_name, ".") && strcmp(newEntry->d_name, "..")) 
            {
                snprintf(buffer, PATH_MAX, "%s/%s", entryName, newEntry->d_name);
                //recursively calling function till maxDepthValue goes to 0
                totalSize += TraverseDir(buffer, printFileFlag, maxDepthFlag, maxDepthValue - 1, humanFlag);
            }
        closedir(dir);
    }

    //adding block size 
    totalSize += st.st_blocks;

    if ((printFileFlag || S_ISDIR(st.st_mode)) && (maxDepthValue >= 0 || !maxDepthFlag))
        blockMath(entryName, totalSize, humanFlag);

    return totalSize;  //returning the number of blocks
}

//this function check the given flag in command line argument and the path given
void run(int argc, char **argv) 
{
    int i, letteridx;
    int totalFlag = 0, maxDepthFlag = 0, maxDepthVal = 0;
    int printFileFlag = 0, fileDirFlag = 0, humanFlag = 0;
    blkcnt_t total = 0;	


    //as i=0 is ./a.out
    for (i = 1; i < argc; i++) 
    {

        //12 is the length of --max-depth=
        if (strncmp(argv[i], "--max-depth=", 12) == 0) 
        {
            //converting array to integer
            maxDepthVal = atoi(&(argv[i][12]));
            maxDepthFlag = 1;
        }

        else if (argv[i][0] == '-') 
        {
            for (letteridx = 1; letteridx < strlen(argv[i]); letteridx++) 
            {
                if (argv[i][letteridx] != 'h' && argv[i][letteridx] != 'c' && argv[i][letteridx] != 'a') 
                {
                    printf("du: invalid option -- '%c'\n", argv[i][letteridx]);
                    printf("Try `du --help' for more information.\n");
                    exit(0);
                }
                if (argv[i][letteridx] == 'a') 
                    printFileFlag = 1;

                if (argv[i][letteridx] == 'c') 
                    totalFlag = 1;

                if (argv[i][letteridx] == 'h') 
                    humanFlag = 1;	
            }
        }
        //if no flags is given in command line argument
        else 
            fileDirFlag = 1;
    }
    //if no file directory is mentioned then look in current directory represented by '.'
    if (!fileDirFlag) 
        total += TraverseDir(".", printFileFlag, maxDepthFlag, maxDepthVal, humanFlag);

    else 
    {
        for (i = 1; i < argc; i++)
        {
            if ((argv[i][0] != '-')) 
                total += TraverseDir(argv[i], printFileFlag, maxDepthFlag, maxDepthVal, humanFlag);
        }
    }
    if (totalFlag)
        //calling function to compute size from blocks
        blockMath("total", total, humanFlag);
}

//taking command line argument in main function
int main(int argc, char **argv) 
{
    //argv is array character pointer listing all the arguments
    //in our case it is the path of the directory
    run(argc, argv);
    printf("\nProgram exited with 0 return value\n");
    return 0;
}
