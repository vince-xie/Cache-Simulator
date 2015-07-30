/*
//  c-sim.c
//
//  Created by Vince Xie on 12/5/14.
//  Copyright (c) 2014 Vincent Xie. All rights reserved.
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "c-sim.h"

/*
creates a cold cache (every line invalid)
time: O(n), space: O(n), where n = the number of lines
*/
CacheLine *create_cold_cache(){
    int i;
    CacheLine *cache;
    cache = malloc(sizeof(CacheLine) * num_lines);
    i = 0;
    for(; i < num_lines; i++){
        cache[i].valid = 0;
        cache[i].tag = (char *)malloc(sizeof(char) * 81);
        strcpy(cache[i].tag, "-");
        cache[i].set = i / assoc;
        cache[i].recent = 0;
        cache[i].dirty = 0;
        cache[i].tag_length = 1;
    }
    return cache;
}

/*
sets number of bits needed for the set index and the block offset
time: O(log(m) + log(n)), space: O(1), where m = block size and n = number of sets
*/
void set_subs(){
    int i = 1;
    for(; i < block_size; i*=2){
        block_sub++;
    }
    if(i != block_size && block_sub != 0){
        printf("ERROR: Block size is not a power of 2. \n");
        exit(1);
    }
    i = 1;
    for(; i < num_lines / assoc; i*=2){
        set_sub++;
    }
    if(i != num_lines / assoc && set_sub != 0){
        printf("ERROR: Associativity is not a power of 2. \n");
        exit(1);
    }
}

/*
gets tag length of each address using the block size and number of sets
time: O(1), space: O(1), where n = associativity
*/
int get_tag_length(char address[]){
    return strlen(address) - set_sub - block_sub;
}

/*
converts an address to binary
time: O(n), space: O(n), where n = length of the address
*/
char *convert_to_binary(char address[]){
    char *binary = (char *)malloc(sizeof(char) * 81);
    int i = 2;
    for(; i < strlen(address); i++){
        switch(address[i]){
            case '0': strcat(binary,"0000"); break;
            case '1': strcat(binary,"0001"); break;
            case '2': strcat(binary,"0010"); break;
            case '3': strcat(binary,"0011"); break;
            case '4': strcat(binary,"0100"); break;
            case '5': strcat(binary,"0101"); break;
            case '6': strcat(binary,"0110"); break;
            case '7': strcat(binary,"0111"); break;
            case '8': strcat(binary,"1000"); break;
            case '9': strcat(binary,"1001"); break;
            case 'a': strcat(binary,"1010"); break;
            case 'b': strcat(binary,"1011"); break;
            case 'c': strcat(binary,"1100"); break;
            case 'd': strcat(binary,"1101"); break;
            case 'e': strcat(binary,"1110"); break;
            case 'f': strcat(binary,"1111"); break;
        }
    }
    return binary;
}

/*
gets set index from binary address
time: O(n), space: O(1), where n = log(number of sets)
*/
int get_index(char address[], int tag_length){
    int index = 0;
    int exp = 1;
    int i = strlen(address) - 1 - block_sub;
    for(; i >= tag_length; i--){
        if(address[i] == '1'){
            index += exp;
        }
        exp = exp<<1;
    }
    return index;
}

/*
updates the most recently used set, allowing us to find the least used line
time: O(n), space: O(1), where n = associativity
*/
void update_recents(int new, int index){
    int i = index * assoc;
    for(; i < index * assoc + assoc; i++){
        cache[i].recent++;
    }
    cache[new].recent = 0;
}

/*
performs a write-through write operation using an address read from the file
time: O(n), O(1), where n = associativity
*/
void write_to_cache(char address[]){
    int highest = 0;
    int index = get_index(address, get_tag_length(address)) * assoc;
    int highest_index = index;
    int i = index;
    memory_writes++;
    for(; i < index + assoc; i++){
        if(strncmp(cache[i].tag, address, cache[i].tag_length) == 0){
            if(cache[i].valid == 1){
                cache_hits++;
                strcpy(cache[i].tag, address);
                cache[i].tag_length = get_tag_length(address);
                update_recents(i, cache[i].set);
                return;
            }
        }
    }
    cache_misses++;
    memory_reads++;
    index = get_index(address, get_tag_length(address)) * assoc;
    i = index;
    for(; i < index + assoc; i++){
        if(cache[i].valid == 0){
            cache[i].valid = 1;
            strcpy(cache[i].tag, address);
            cache[i].tag_length = get_tag_length(address);
            update_recents(i, cache[i].set);
            return;
        }
    }
    index = get_index(address, get_tag_length(address)) * assoc;
    i = index;
    for(; i < index + assoc; i++){
        if(cache[i].recent > highest){
            highest = cache[i].recent;
            highest_index = i;
        }
    }
    cache[highest_index].tag_length = get_tag_length(address);
    strcpy(cache[highest_index].tag, address);
    update_recents(highest_index, cache[highest_index].set);
}

/*
performs a write-through read operation using an address read from the file
time: O(n), O(1), where n = associativity
*/
void read_from_cache(char address[]){
    int index = get_index(address, get_tag_length(address)) * assoc;
    int highest_index = index;
    int highest = 0;
    int i = index;
    for(; i < index + assoc; i++){
        if(cache[i].valid == 1){
            if(strncmp(cache[i].tag, address, cache[i].tag_length) == 0){
                cache_hits++;
                update_recents(i, cache[i].set);
                return;
            }
        }
    }
    cache_misses++;
    memory_reads++;
    index = get_index(address, get_tag_length(address)) * assoc;
    i = index;
    for(; i < index + assoc; i++){
        if(cache[i].valid == 0){
            cache[i].valid = 1;
            cache[i].tag_length = get_tag_length(address);
            strcpy(cache[i].tag, address);
            update_recents(i, cache[i].set);
            return;
        }
    }
    index = get_index(address, get_tag_length(address)) * assoc;
    i = index;
    for(; i < index + assoc; i++){
        if(cache[i].recent > highest){
            highest = cache[i].recent;
            highest_index = i;
        }
    }
    cache[highest_index].tag_length = get_tag_length(address);
    strcpy(cache[highest_index].tag, address);
    update_recents(highest_index, cache[highest_index].set);
}

/*
performs a write-back write operation using an address read from the file
time: O(n), O(1), where n = associativity
*/
void write_to_cache_wb(char address[]){
    int index = get_index(address, get_tag_length(address)) * assoc;
    int highest_index = index;
    int highest = 0;
    int i = index;
    for(; i < index + assoc; i++){
        if(cache[i].valid == 1){
            if(strncmp(cache[i].tag, address, cache[i].tag_length) == 0){
                cache_hits++;
                cache[i].tag_length = get_tag_length(address);
                cache[i].dirty = 1;
                update_recents(i, cache[i].set);
                return;
            }
        }
    }
    cache_misses++;
    memory_reads++;
    index = get_index(address, get_tag_length(address)) * assoc;
    i = index;
    for(; i < index + assoc; i++){
        if(cache[i].valid == 0){
            cache[i].valid = 1;
            cache[i].tag_length = get_tag_length(address);
            strcpy(cache[i].tag, address);
            cache[i].dirty = 1;
            update_recents(i, cache[i].set);
            return;
        }
    }
    index = get_index(address, get_tag_length(address)) * assoc;
    i = index;
    for(; i < index + assoc; i++){
        if(cache[i].recent > highest){
            highest = cache[i].recent;
            highest_index = i;
        }
    }
    if(cache[highest_index].dirty == 1){
        memory_writes++;
    }
    cache[highest_index].tag_length = get_tag_length(address);
    strcpy(cache[highest_index].tag, address);
    cache[highest_index].dirty = 1;
    update_recents(highest_index, cache[highest_index].set);
}

/*
performs a write-back read operation using an address read from the file
time: O(n), O(1), where n = associativity
*/
void read_from_cache_wb(char address[]){
    int index = get_index(address, get_tag_length(address)) * assoc;
    int highest_index = index;
    int highest = 0;
    int i = index;
    for(; i < index + assoc; i++){
        if(cache[i].valid == 1){
            if(strncmp(cache[i].tag, address, cache[i].tag_length) == 0){
                cache_hits++;
                update_recents(i, cache[i].set);
                return;
            }
        }
    }
    memory_reads++;
    cache_misses++;
    index = get_index(address, get_tag_length(address)) * assoc;
    i = index;
    for(; i < index + assoc; i++){
        if(cache[i].valid == 0){
            cache[i].valid = 1;
            cache[i].dirty = 0;
            cache[i].tag_length = get_tag_length(address);
            strcpy(cache[i].tag, address);
            update_recents(i, cache[i].set);
            return;
        }
    }
    index = get_index(address, get_tag_length(address)) * assoc;
    i = index;
    for(; i < index + assoc; i++){
        if(cache[i].recent > highest){
            highest = cache[i].recent;
            highest_index = i;
        }
    }
    if(cache[highest_index].dirty == 1){
        memory_writes++;
    }
    cache[highest_index].tag_length = get_tag_length(address);
    strcpy(cache[highest_index].tag, address);
    cache[highest_index].dirty = 0;
    update_recents(highest_index, cache[highest_index].set);
}

/*
prints the cache (for testing purposes)
time: O(n), space: O(1), where n = number of lines
*/
void print_cache(){
    int i = 0;
    int j = 0;
    for(; i < num_lines; i++){
        printf("index: %d set: %d valid: %d recent: %d tag length: %d tag:", i, cache[i].set, cache[i].valid, cache[i].recent, cache[i].tag_length);
        for(; j < cache[i].tag_length; j++){
            printf("%c", cache[i].tag[j]);
        }
        printf("\n");
    }
}

/*
frees memory allocated for the cache
time: O(n), space: O(1), where n = number of lines
*/
void free_cache(){
    int i = 0;
    for(; i < num_lines; i++){
        free(cache[i].tag);
    }
    free(cache);
}

int main(int argc, const char * argv[]) {
    char *c;
    char ip[20];
    char wr[2];
    char address[20];
    char *binary;
    FILE *file;
    if(strcmp(argv[1], "-h") == 0){
        printf("Usage: c-sim [-h] <cache size> <associativity> <block size> <write policy> <trace file> \n < cachesize > is the total size of the cache. This should be a power of 2. Also, it should always be true that < cachesize > = number of sets × < setsize > × < blocksize >. \n    For direct-mapped caches, < setsize > = 1. For n − way associative caches, < setsize > = n. ");
	printf("\n    Given the above formula, together with < cachesize >, < setsize >, and < blocksize >, you can always compute the number of sets in your cache. \n < associativity > is one of: \n     – direct - simulate a direct mapped cache. \n");
	printf("     – assoc - simulate a fully associative cache. \n     – assoc:n - simulate an n − way associative cache. n should be a power of 2. \n < blocksize > is an power of 2 integer that specifies the size of the cache block. \n < writepolicy > is one of: \n     – wt - simulate a write through cache. \n     – wb - simulate a write back cache. \n < tracefile > is the name of a file that contains memory access traces. \n");
        return 0;
    }
    if(argc < 6 || argc > 6){
        printf("ERROR: Usage: c-sim [-h] <cache size> <associativity> <block size> <write policy> <trace file> \n");
        return 1;
    }
    size = atoi(argv[1]);
    block_size = atoi(argv[3]);
    assoc = 0;
    if(strcmp(argv[2], "direct") == 0){
        assoc = 1;
    }
    else if(strcmp(argv[2], "assoc") == 0){
        assoc = size / block_size;
    }
    else{
        c = strtok(argv[2], ":");
        c = strtok(NULL, ":");
        if(c == NULL){
            printf("ERROR: invalid associativity \n");
            return 1;
        }
        assoc = atoi(c);
    }
    if(strcmp(argv[4], "wt") == 0){
        strcpy(write_policy, "wt");
    }
    else if(strcmp(argv[4], "wb") == 0){
        strcpy(write_policy, "wb");
    }
    else{
        printf("ERROR: invalid write policy \n");
    }
    file = fopen(argv[5], "r");
    if (file == NULL) {
        fprintf(stderr, "ERROR: cannot open input file. \n");
        return 1;
    }
    num_lines = size / block_size;
    cache = create_cold_cache();
    if(num_lines / assoc  * assoc * block_size != size || size == 0){
        fprintf(stderr, "ERROR: Invalid size, block size, or associativity. Cache size must greater than 0 and equal to num_sets * assoc * block size \n");
        return 1;
    }
    set_subs();
    while (fscanf(file, "%s %s %s", ip, wr, address) != EOF) {
        if(strcmp(ip, "#eof") == 0){
            break;
        }
        if(strcmp(write_policy, "wt") == 0){
            binary = convert_to_binary(address);
            if(strcmp(wr, "W") == 0){
                write_to_cache(binary);
            }
            else if(strcmp(wr, "R") == 0){
                read_from_cache(binary);
            }
        }
        else{
            binary = convert_to_binary(address);
            if(strcmp(wr, "W") == 0){
                write_to_cache_wb(binary);
            
            }
            else if(strcmp(wr, "R") == 0){
                read_from_cache_wb(binary);
            }
        }
    }
    printf("Cache hits: %d\n", cache_hits);
    printf("Cache misses: %d\n", cache_misses);
    printf("Memory reads: %d\n", memory_reads);
    printf("Memory writes: %d\n", memory_writes);
    fclose(file);
    free_cache();
    return 0;
}
