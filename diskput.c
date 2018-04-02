/*This example code is for reading from test.img and obtain data with byte ordering of Big Endian. Also try to directly print binary data as a string and make modification.*/

#include <stdio.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>



long size_of_txt=0;
char* address;
int blocksize=0;
long root_start=0;
long root_block=0;
long FAT_block=0;
long start=0;
long block_count;

long get_number(char* address,int byte){
    long outcome=0;
    memcpy(&outcome,address+byte,4);
    outcome=htonl(outcome);
    return outcome;
}


void store_into_root(int start_b,int n_of_b, char name[]){

    int root_entry_start= (int)root_start*blocksize ;
    int root_entry_finish=(int)root_entry_start+(int)root_block*blocksize;
    int entry_start= root_entry_start;
    char stat;

    while(entry_start<=root_entry_finish){
        memcpy(&stat,address+entry_start,1);
        if(stat==0x00){
            stat=0x03;
            memcpy(address+entry_start,&stat,1);

            memcpy(address+entry_start+4,&start_b,1);
            memcpy(address+entry_start+8,&n_of_b,1);
            printf("file size: %ld\n",size_of_txt);
            size_of_txt=htonl(size_of_txt);
            memcpy(address+entry_start+9,&size_of_txt,4);


            int year =2018;
            int month =4;
            int day =2;
            int hour =12;
            int min =0;
            int sec =0;
            memcpy(address+entry_start+14,&year,1);
            memcpy(address+entry_start+15,&month,1);
            memcpy(address+entry_start+16,&day,1);
            memcpy(address+entry_start+17,&hour,1);
            memcpy(address+entry_start+18,&min,1);
            memcpy(address+entry_start+19,&sec,1);

            memcpy(address+entry_start+21,&year,1);
            memcpy(address+entry_start+22,&month,1);
            memcpy(address+entry_start+23,&day,1);
            memcpy(address+entry_start+24,&hour,1);
            memcpy(address+entry_start+25,&min,1);
            memcpy(address+entry_start+26,&sec,1);

            int c;
            for(c=0;c<strlen(name);c++){
                char a = name[c];
                memcpy(address+entry_start+27+c,&a,1);
            }
            int unused=0xff;
            int count_unuse;
            for(count_unuse=0;count_unuse<6;count_unuse++){
                memcpy(address+entry_start+58+count_unuse,&unused,1);
            }
            

            break;
        }

        entry_start=entry_start+64;
    }


}

int main(int argc, char* argv[]) {

    int fd = open(argv[1], O_RDWR);
    struct stat buffer;
    int status = fstat(fd, &buffer);

    address=mmap(NULL, buffer.st_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

   
    memcpy(&blocksize,address+8,2);
    blocksize=htons(blocksize);

    block_count=get_number(address,10);

   
    start=get_number(address,14);

   
    FAT_block=get_number(address,18);

   
    root_start=get_number(address,22);

   
    root_block=get_number(address,26);
    
    long starting_byte=(long)blocksize* start;
    long ending_byte=starting_byte + (long)blocksize* FAT_block;
    long i;
    long byte=0;
    int count_free=0;
    int count_reserved=0;
    int count_allocated=0;
    int free_blocks[blocksize*FAT_block];
    
    for(i=starting_byte; i<ending_byte;i=i+4){
            memcpy(&byte,address+i,4);
            byte=htonl(byte);
            if(byte==0x00000000){
                free_blocks[count_free]=i;
                count_free++;
            }else{
                continue;
                }
    }
    

    //open txt file 
    FILE *txt_file;
    txt_file =fopen(argv[2],"r");

 
    fseek(txt_file , 0, SEEK_END);
    //size pf the text file in byte
    size_of_txt=ftell(txt_file);
    fseek(txt_file, 0, SEEK_SET);


        //write into fat
     int block_taken = size_of_txt/512;
        if(size_of_txt%512!=0){
        block_taken=block_taken+1;
        }

    int c_b;
   
    for(c_b=0;c_b<(block_taken-1);c_b++){
        int a;
            a =(free_blocks[c_b+1]-starting_byte)/4;
            memcpy(address+free_blocks[c_b]+3,&a,1);

    }
    long aa=0xffffffff;
    memcpy(address+free_blocks[block_taken-1],&aa,4);
    
    //write content into disk 
    char code[size_of_txt];
    long n = 0;
    int cc;
    
    while ((cc = fgetc(txt_file)) != EOF){
        code[n] = (char) cc;
        n++;
    }
    code[n] = '\0'; 

    int kk;
    int ll;
    char co;
    long si=0;
    while(si<size_of_txt){ 
    for(kk=0;kk<block_taken;kk++){
        for(ll=0;ll<512;ll++){
                co=code[si];
                memcpy(address+((free_blocks[kk]-starting_byte)/4)*512+ll,&co,1);
                si++;
                }      
            }
    }


     //write name,status ,etc into root 
     int file_start=(free_blocks[0]-starting_byte)/4;
     store_into_root(file_start,block_taken,argv[2]);
    


    fclose(txt_file);

    munmap(address,buffer.st_size);
    close(fd);
}
