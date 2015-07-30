/*
//  c-sim.h
//
//  Created by Vince Xie on 12/10/14.
//  Copyright (c) 2014 Vincent Xie. All rights reserved.
*/

#ifndef c_sim_h
#define c_sim_h

typedef struct CacheLine{
    int valid;
    char *tag;
    char *block;
    int set;
    int recent;
    int dirty;
    int tag_length;
} CacheLine;

int cache_hits = 0;
int cache_misses = 0;
int memory_reads = 0;
int memory_writes = 0;
int num_lines = 0;
int size = 0;
int block_size = 0;
int assoc = 0;
int block_sub = 0;
int set_sub = 0;
char write_policy[] = "  ";
CacheLine* cache;

CacheLine *create_cold_cache();
void set_subs();
int get_tag_length(char address[]);
char *convert_to_binary(char address[]);
int get_index(char address[], int tag_length);
void update_recents(int new, int index);
void write_to_cache(char address[]);
void read_from_cache(char address[]);
void write_to_cache_wb(char address[]);
void read_from_cache_wb(char address[]);
void print_cache();
void free_cache();

#endif
