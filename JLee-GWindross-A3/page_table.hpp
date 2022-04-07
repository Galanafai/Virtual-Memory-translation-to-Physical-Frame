/*
* Name1, RedID1, Name2, RedID2
* Jason Lee, 823622872, Galanafai Windross, 823181886
*/

#ifndef PAGE_TABLE_HPP
#define PAGE_TABLE_HPP

#define MACHINE_SIZE 32 /* 32 bit system */
#define MAX_BIT_USE 28  /* due to 32 bit requirements */
#define BYTE_SIZE 8     /* 8 bits per byte */

#include <vector>
#include <unordered_map>
#include <memory>
#include <unistd.h>
#include <getopt.h>
#include <stdlib.h>
#include <iostream>
#include <string.h>
#include <fstream>
#include <math.h>
#include <climits>

struct Map // structure for Map
{
    int pfn;            // store frame
    bool valid = false; // store if frame is valid or not
};

struct level //structure for level
{

    class PageTable *tablePtr;                        //ptr to point back to page table
    std::vector<std::shared_ptr<Map>> mapping;        //note we used Smart ptr to combat memory leakage. Automatic deallocation when out of scope. array of Map *
    std::vector<std::shared_ptr<level>> nextLevelPtr; //note we used Smart ptr to combat memory leakage. Automatic deallocation when out of scope. array of level * for next level
    unsigned int page;                                //store vpn for current level page
    int currDepth;                                    //depth of current level
    //std::vector<level *> nextLevelPtr;
};

class PageTable //page table struct
{

public:
    /**
     * root node of the dictionary tree
     */
    struct std::shared_ptr<level> levelRootNode = std::make_shared<level>();
    unsigned int offSetMask;                //mask of all offset
    int frameCounter = 0;                   //start frame counter at 0 when page table is created
    int levelCount;                         // total level size for translation uses ex ( 4 + 8 + 8)
    std::vector<int> levelDepths;           //vector for each level depth;
    std::vector<unsigned int> bitmaskArray; //array of saved bitmask for each level to get the correctr vpn
    std::vector<int> shiftArray;            //array of shifts for each level to shift and get correct vpn and mapping
    std::vector<int> entryCount;            /* dynamically allocate size for entry count */
    //Map *pageLookup(unsigned int virtualAddress);
    int entries = 0; // num of entries for current vpn
    ~PageTable()
    {
        levelRootNode = nullptr; //dealocate levelroot for memory protections
        //pages = ;
    }
};

/*
Used to add new entries to the page table when we have discovered that a 
page has not yet been allocated (pageLookup returns NULL). Frame is the 
frame index which corresponds to the page number of the virtual address.  
Use a frame number of 0 the first time this is called, and increment the 
frame index by 1 each time a new page→frame map is needed. If you 
wish, you may replace void with int or bool and return an error code if 
unable to allocate memory for the page table. HINT: If you are inserting a 
page, you do not always add nodes at every level. The Map structure may 
already exist at some or all of the levels.  
All other interfaces may be developed as you see fit.  
Translation Lookaside Buffer (TLB) 
As part of the MMU translation simulation, you are required to implement a simulation 
of TLB for caching the virtual page to physical frame mappings from the page table, 
each TLB entry caches one mapping of a Virtual Page Number to a Physical Frame 
Number (VPN → PFN). 
The TLB size (max number of mappings or entries) can be designated by an optional 
command line argument (see -c in the user interface specification below). The default 
size is 32 if the argument is not specified. 
Your TLB cache will use an approximation of the least recently used (LRU) cache 
replacement policy. When TLB is full (number of mappings cached reaches the TLB 
*/
void pageInsert(PageTable *pagetable, unsigned int virtualAddress, unsigned int frame);

/*
add new levels/ Recursive function 
*/

void pageInsert(std::shared_ptr<level> prevlevel, unsigned int address, unsigned int frame);

/*
Given a virtual address, apply the given bit mask and shift right by the 
given number of bits. Returns the virtual page number.  This function can 
be used to access the page number of any level by supplying the 
appropriate parameters.  
Example: Suppose the level two pages occupied bits 22 through 27, and 
we wish to extract the second level page number of address 0x3c654321. 
virtualAddressToPageNum(0x3c654321, 0x0FC00000, 22) should return 
0x31 (decimal 49).  Remember, this is computed by taking the bitwise and 
of 0x3c654321 and 0x0FC00000, which is 0x0C400000.  We then shift 
right by 22 bits.  The last five hexadecimal zeros take up 20 bits, and the 
bits higher than this are 1100 0110 (C6). We shift by two more bits to 
have the 22 bits, leaving us with 11 0001, or 0x31.
*/
unsigned int virtualAddressToPageNum(unsigned int virtualAddress, unsigned int mask, unsigned int shift);

/* get the offset of VA */
unsigned int virtualAddressToOffset(unsigned int virtualAddress, unsigned int offsetMask);



//find page and vpn mapping  to pfn
std::shared_ptr<Map> pageLookup(PageTable *pageTable, unsigned int virtualAddress);

int calcualteTotalBytes(PageTable *pageTable);

/* calcaute mask for each bitmask array */
unsigned int calculateMask(unsigned int shift, int depth);

//look for vpns of inserted or found page.
unsigned int *vpnLookup(unsigned int *pages, PageTable *pageTable, unsigned int virtualAddress);

#endif