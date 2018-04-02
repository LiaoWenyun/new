/*This example code is for reading from test.img and obtain data with byte ordering of Big Endian. Also try to directly print binary data as a string and make modification.*/

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

long get_number(char* address,int byte){
    long outcome=0;
    memcpy(&outcome,address+byte,4);
    outcome=htonl(outcome);
    return outcome;
}

int main(int argc, char* argv[]) {

    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    int status = fstat(fd, &buffer);

    //tamplate:   pa=mmap(addr, len, prot, flags, fildes, off);
    //c will implicitly cast void* to char*, while c++ does NOT
    char* address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    int blocksize=0;
    memcpy(&blocksize,address+8,2);
  // printf("directly getting block size: %d\n",blocksize);
    blocksize=htons(blocksize);
    printf("block size : %d\n",blocksize);


    long block_count;
    block_count=get_number(address,10);
    printf("Block count: %ld\n",block_count);


    long start=0;
    start=get_number(address,14);
    printf("FAT starts: %ld\n",start);

    long FAT_block=0;
    FAT_block=get_number(address,18);
    printf("FAT Blocks: %ld\n",FAT_block);

    long root_start=0;
    root_start=get_number(address,22);
    printf("Root directory start: %ld\n",root_start);

    long root_block=0;
    root_block=get_number(address,26);
    printf("Root directory blocks: %ld\n",root_block);


    long starting_byte=(long)blocksize* start;
    long ending_byte=starting_byte + (long)blocksize* FAT_block;
    long i;
    long byte=0;
    int count_free=0;
    int count_reserved=0;
    int count_allocated=0;
    
    for(i=starting_byte; i<ending_byte;i=i+4){
            memcpy(&byte,address+i,4);
            byte=htonl(byte);
            //printf("byte: %ld",byte);
            if(byte==0x00000000){
                count_free++;
            }else if(byte==0x00000001){
                count_reserved++;
            }else{
                count_allocated++;
                }
            
        
    }
    //memcpy(&byte,address+starting_byte+8,4);
    
    printf("\n");
    printf("FAT information:\n");
    printf("free Blocks: %d\n",count_free);
    printf("reserved Blocks: %d\n",count_reserved);
    printf("allocated Blocks: %d\n",count_allocated);


    munmap(address,buffer.st_size);
    close(fd);
}
