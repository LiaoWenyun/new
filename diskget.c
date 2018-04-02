/*This example code is for reading from test.img and obtain data with byte ordering of Big Endian. Also try to directly print binary data as a string and make modification.*/

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>



typedef struct file{
    char st;
    long File_size;
    char File_name[31];
    long starting_block;
    long number_blocks;
}File;

char* address;
int blocksize=0;
long block_count;
long FAT_start=0;
long FAT_block=0;
long root_start=0;
long root_block=0;



long get_number(char* address,int byte,int num_byte){
    long outcome=0;
    memcpy(&outcome,address+byte,num_byte);
    outcome=htonl(outcome);
    return outcome;
}


void parse(File *the_file, int entry){
    char stat;
    char file_status;
    int root_entry_start= entry;
    
    memcpy(&stat,address+root_entry_start,1);
    if(stat==0x03){
        file_status='F';
    }else if(stat==0x00){
        file_status ='N';
    }else if(stat==0x05){file_status='D';}
    the_file->st=file_status;

    long  start_b;
    start_b=get_number(address,root_entry_start+1,4);
    the_file->starting_block=start_b;

    long  num_b;
    num_b=get_number(address,root_entry_start+5,4);
    the_file->number_blocks=num_b;


    int i;
    int j=0;
    char name_array[31];
    int count=0;
    for (i=0;i<31;i++){
        char file_char;
        memcpy(&file_char,address+root_entry_start+26+i,1);
        if (file_char!=0x00){
            name_array[j]=file_char;  
            j++;
        }
    }

    int k;
    int c=0;
    int x;
    for (k=0;k<31;k++){
        x=name_array[k]-'a';
        if(x!=-84){
            the_file->File_name[c]=name_array[k];
            c++;
        }     
    }
}
    



int main(int argc, char* argv[]) {
     
    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    int status = fstat(fd, &buffer);
    FILE *file_to_write;
    file_to_write = fopen(argv[2], "w");

    //memory map
    address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    memcpy(&blocksize,address+8,2);
    blocksize=htons(blocksize);
    
    block_count=get_number(address,10,4);

    FAT_start=get_number(address,14,4);
   
    FAT_block=get_number(address,18,4);
    
    root_start=get_number(address,22,4);
   
    root_block=get_number(address,26,4);
   

    int root_entry_start= (int)root_start*blocksize ;
    int root_entry_finish=(int)root_entry_start+(int)root_block*blocksize;
    int entry_start= root_entry_start;
    int count=0;
    int array[50];
    while(entry_start<=root_entry_finish){
        File *temp_file = malloc(sizeof(*temp_file));
        parse(temp_file, entry_start);
        if(strcmp(temp_file->File_name,argv[3])==0){
         
            int fat_entry=FAT_start*blocksize;
            int content_block;
            int next_block;
            array[0]=temp_file->starting_block;
            for(content_block=0;content_block<10;content_block++){
                if(array[content_block]>0){
                    memcpy(&next_block,address+fat_entry+4*array[content_block],4);
                    next_block=htonl(next_block);
                    array[content_block+1]=next_block;
                   // printf("%d\n",array[content_block]);
                } else{
                    array[content_block+1]=-1;
                }
            }
            

            char charector;
            int init;
            int l;
            for(l=0;l<50;l++){
                if(array[l]>0){
                    for(init=0; init<512;init++){
                        memcpy(&charector,address+array[l]*512+init,1);
                        //printf("%c", charector);
                        fputc (charector, file_to_write);
                    }
                }else {
                    break;
                }
                
            }
            
            fclose(file_to_write);
             munmap(address,buffer.st_size);
             close(fd);
             return 0;
            
        }
        free(temp_file);
        entry_start=entry_start+64;
        
    }

    printf("File not found.\n");

    munmap(address,buffer.st_size);
    close(fd);
    return 0;
}
