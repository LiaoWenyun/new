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
    int m_year;
    int m_month;
    int m_day;
    int m_hour;
    int m_min;
    int m_sec;  
}File;

char* address;
int blocksize=0;
long block_count;
long start=0;
long FAT_block=0;
long root_start=0;
long root_block=0;
 File *root_file;


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
    

    long  num_b;
    num_b=get_number(address,root_entry_start+5,4);


    long  file_size;
    file_size=get_number(address,root_entry_start+9,4);
     the_file->File_size=file_size;

    int  modify_year;
    memcpy(&modify_year,address+root_entry_start+20,2);
    modify_year=htons(modify_year);
    the_file->m_year=modify_year;

    int  modify_month;
    memcpy(&the_file->m_month,address+root_entry_start+22,1); 		

    int  modify_day;
    memcpy(&the_file->m_day,address+root_entry_start+23,1);
  
    int  modify_hour;
    memcpy(&the_file->m_hour,address+root_entry_start+24,1);
 
    int  modify_min;
    memcpy(&the_file->m_min,address+root_entry_start+25,1);

    int  modify_sec;
    memcpy(&the_file->m_sec,address+root_entry_start+26,1);

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


    //tamplate:   pa=mmap(addr, len, prot, flags, fildes, off);
    //c will implicitly cast void* to char*, while c++ does NOT
    address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    
    memcpy(&blocksize,address+8,2);
    blocksize=htons(blocksize);
    
    block_count=get_number(address,10,4);

    start=get_number(address,14,4);
   
    FAT_block=get_number(address,18,4);
    
    root_start=get_number(address,22,4);
   
    root_block=get_number(address,26,4);
   

    int root_entry_start= (int)root_start*blocksize ;
    int root_entry_finish=(int)root_entry_start+(int)root_block*blocksize;
    int entry_start= root_entry_start;

    while(entry_start<=root_entry_finish){
        File *root_file = malloc(sizeof(*root_file));
        parse(root_file, entry_start);
        if(root_file->st=='F' ||root_file->st=='D'){
            printf("%c%10ld%25s%5d/%02d/%02d %02d:%02d:%02d \n", root_file->st, root_file->File_size,root_file->File_name, root_file->m_year,root_file->m_month,root_file->m_day,root_file->m_hour,root_file->m_min,root_file->m_sec);
        }
        free(root_file);
        entry_start=entry_start+64;
    }

    //free(root_file);
    munmap(address,buffer.st_size);
    close(fd);
    return 0;
}

