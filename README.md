# Cache-Simulator

This simulates L1 level cache with a LRU (least recently used) replacement algorithm.

c-sim [-h] [cache size] [associativity] [block size] [write policy] [trace file]

where:
* < cachesize > is the total size of the cache. This should be a power of 2. Also, it should always be true that
< cachesize > = number of sets × < setsize > × < blocksize >.
* < associativity > is one of: direct - simulate a direct mapped cache, assoc - simulate a fully associative cache, assoc:n - simulate an n − way associative cache. n should be a power of 2.
* < blocksize > is an power of 2 integer that specifies the size of the cache block.
* < writepolicy > is one of: wt - simulate a write through cache, wb - simulate a write back cache.
* < tracefile > is the name of the text file which contains memory access traces. 

Example Traces: (The first is a write, while the second is a read.)

0x37c852: W 0xbfd4b18c 

0x37cfff: R 0x39dfc0

#### Process
  The cache simulator first reads in inputs from the command line from the user. After it checks to see if all of the inputs are valid, it then goes and starts reading from the file. After checking the inputs, the simulator finds the number of sets, lines, bits needed for the set index, and bits needed for the block offset. The bits for the set index and the bits for the block offset are set using set_subs(). After that, since it needs a cache to read and write to, it calls create_cold_cache() to create a new empty cache. Next, whenever the simulator reads in a new line, the first thing it does it convert it to binary using convert_to_binary(). If the write condition is “wt”, it calls either write_to_cache() or read_to_cache() depending on whether or not we are reading or writing to the cache. If the write condition is “wb”, write_to_cache_wb() or read_to_cache_wb() depending on whether or not we are reading or writing to the cache.

  These functions are where everything is implemented and the number of cache hits and misses, memory reads and writes are incremented. Since this is the bulk of the logic, these will be explained later on below. These functions all implement the same helper functions, get_tag_length(), get_index(), and update_recents(). The simulator uses get_tag_length() to get the number of tag bits that are allocated for the tag. It calculates this by subtracting the number of bits needed for the set index and the number of bits needed for the block offset from the length of the binary address. 
  
  The simulator uses get_index() to find the set index that we are going to look for the data in. This uses the bits of the address between the tag and the block offset to calculate the set number. The simulator takes these bits and translates them into decimal and then returns it as the index. The simulator uses update_recents() to update every cache line in the set as to which one was most recently used. This allows the simulator to later find out which line was last used in order to figure out which line to evict.
Now, back to the write-through implementation. Starting with write_to_cache(), this is implemented by calculating the set index using get_index(). First, it checks the set for a line where the tags match. This will give us a cache hit if there is a match. It then writes the data into the line and then into lower memory, giving us a memory write. If there is no cache hit, we get cache miss and a memory read, and it looks through the set for a line that is empty (not valid). If it finds an empty cache line, it then writes data into the line and then into lower memory, giving us a memory write. If there is no empty cache line, then it looks to evict the last used line in the set, writing data into the line and then into lower memory, giving us a memory write. With this implementation, there will always be a memory write.

  Next, with read_from_cache(). It starts off the same way, by calculating the set index using get_index() and checking the set for a line where the tags match. This will give us a cache hit if there is a match and then returns the data. If there is a cache miss, it again looks through the set for a line that is empty (not valid). If it finds an empty cache line, it then reads data into the line from lower memory, giving us a memory read. If there is no empty cache line, then it looks to evict the last usedline in the set, reading data into the line from lower memory, giving us a memory read.

Supplementary Information:

  As for implementations of associativity, direct vs. associative cache mappings
go, this implementation works for all of them. This is because all of the bits calculated for the tag, set index are relative. If it is associative, then all of the lines will be in the same set. This means that get_index() will always be returning 0 because set_subs() will calculate 0 for the number of bits set aside for the set index because 20 = 1 set. As such, the whole cache will be seen as the same set. If it is direct or associative, then there will be more bits set aside for the set index as again, calculated by get_index() and set_subs() using log2(number of sets) and log2(block size).
As for the replacement algorithm, I used LRU (least recently used) to figure out which lines to evict. Each line has its own integer recent field, which is set to 0 whenever the line is used and incremented every time another line is used. This allows us to easily find which line is least recently used by looking for the largest recent.

   The write back function for write, write_to_cache_wb() is almost exactly like write_to_cache(). The only difference is that we have a dirty bit to worry about. This dirty bit allows the simulation to delay writes to lower memory until it is necessary. Every line starts out with a dirty bit of 0, meaning it is still the original data. However, whenever the data is modified or written over, we change the dirty bit to 1. Therefore, when the simulation finds a line to evict and the dirty bit is 1, we get a memory write. This implementation is usually more efficient because of the fact that it is not writing to lower memory with every write, lowering the amount of overhead latency associated with accessing lower memory.
The write back function for read, read_from_cache_wb() is also almost exactly like read_from_cache(). Again, when finding a line to evict, the simulation checks to see if it is dirty. If it is dirty, we get a memory write, because it writes the data into lower memory. After a read, it then marks it as not dirty.
