/*
* Name1, RedID1, Name2, RedID2
* Jason Lee, 823622872, Galanafai Windross, 823181886
*/

#include "page_table.hpp"
#include <string>
#include <sstream>
#include <vector>
#include <iostream>
#include <math.h>

/*
function called in main to insert a page to the page table. inserts root level
*/

void pageInsert(PageTable *pagetable, unsigned int virtualAddress, unsigned int frame)
{

    /* insert the page to page table*/
    pagetable->levelRootNode->tablePtr = pagetable;
    pagetable->levelRootNode->currDepth = 0; //set root depth to 0
    pageInsert(pagetable->levelRootNode, virtualAddress, frame);
}

/*
recrusive function called to insert a levels into   to the page table.
*/
void pageInsert(std::shared_ptr<level> prevLevel, unsigned int address, unsigned int frame)
{

    int size = prevLevel->tablePtr->entryCount[prevLevel->currDepth];                                                                                                      // get size of vpn
    unsigned int index = virtualAddressToPageNum(address, prevLevel->tablePtr->bitmaskArray[prevLevel->currDepth], prevLevel->tablePtr->shiftArray[prevLevel->currDepth]); // get vpn for depth
    if (prevLevel->currDepth == prevLevel->tablePtr->levelDepths.size() - 1)                                                                                               // this is leaf node where we do mapping
    {
        //save to leaf vpn to pages array in page table
        if (prevLevel->mapping.empty()) // map doesnt exist so create 1
        {

            prevLevel->mapping.resize(size);      //resize map to the size of entries
            prevLevel->tablePtr->entries += size; // add to num of entries of level to page table
        }
        if (prevLevel->mapping[index] == nullptr)
        {                                                        // create new level in index
            prevLevel->mapping[index] = std::make_shared<Map>(); //create a new Map with smart POINTER
            prevLevel->mapping[index]->valid = true;             //set index to valid
            prevLevel->mapping[index]->pfn = frame;              //set mapping for leaf to pfn
            prevLevel->tablePtr->frameCounter++;                 //increase frame counter since new map created
        }
    }

    else //create a new level and new entries and set to invalid
    {

        if (prevLevel->nextLevelPtr.empty()) //nextlevel arr empty so we crate 1
        {

            prevLevel->nextLevelPtr.resize(size); //resize nextlevel entries to entry count for current depth
            prevLevel->tablePtr->entries += size; // add to num of entries of level to page table
        }
        if (prevLevel->nextLevelPtr[index] == nullptr)
        {                                                               // create new level in index
            prevLevel->nextLevelPtr[index] = std::make_shared<level>(); //create a new level with smart POINTER

            prevLevel->nextLevelPtr[index]->tablePtr = prevLevel->tablePtr;       //set back to table ptr to connect
            prevLevel->nextLevelPtr[index]->currDepth = prevLevel->currDepth + 1; // increate curretn depth
        }

        pageInsert(prevLevel->nextLevelPtr[index], address, frame); //do recursion
    }
}

//get vpn and pfn mapping
std::shared_ptr<Map> pageLookup(PageTable *pageTable, unsigned int virtualAddress)
{
    std::shared_ptr<level> node = pageTable->levelRootNode; //set not to root node of page table

    int i = 0;

    while (i < pageTable->levelDepths.size() - 1) //i here is the level
    {

        //convert to indexes
        unsigned int index = virtualAddressToPageNum(virtualAddress, node->tablePtr->bitmaskArray[i], node->tablePtr->shiftArray[i]);
        if (node->nextLevelPtr[index] == nullptr) //word isnt finished since child is none existent
        {
            return nullptr; //return null since not found
        }
        //increase to next part of index
        node = node->nextLevelPtr[index];
        i++;
    }
    //node on leaf level
    int leafIndex = i;
    unsigned int index = virtualAddressToPageNum(virtualAddress, node->tablePtr->bitmaskArray[leafIndex], node->tablePtr->shiftArray[leafIndex]); //get vpn
    //were now on leaf node so we check for
    if (node->mapping[index] != nullptr)
    {
        return node->mapping[index]; //return pfn for vpn
    }
    //std::cout<<"test" <<std::endl;
    return nullptr; //didnt find any mapping
}

unsigned int virtualAddressToPageNum(unsigned int virtualAddress, unsigned int mask, unsigned int shift) //get vpn with current mask and shift that we have stored in page table array
{

    unsigned int vpn = (mask & virtualAddress) >> shift; //this shift is to get logical page i.e. 0000 0000 0100 0000 0000 0000 0000 0000    becom 1 with shift of 22
    return vpn;                                          /* return the page */
}

//get offset bits for virtaul address passed int
unsigned int virtualAddressToOffset(unsigned int virtualAddress, unsigned int offsetMask)
{
    unsigned int offset = offsetMask & virtualAddress;
    return offset;
}

//calcualte mask for each level depth. its calculated for each elvel depth in main
unsigned int calculateMask(unsigned int shift, int depth)
{
    unsigned int mask = 0x0;

    for (int j = 0; j < depth; j++) //get mask dynamically

    {

        mask = mask << 1;
        mask = mask | 1;
    }
    mask = mask << shift;

    return mask;
}

//lookup vpn for each level and store each vpn in pages array and return array oif pages
unsigned int *vpnLookup(unsigned int *pages, PageTable *pageTable, unsigned int virtualAddress)
{
    std::shared_ptr<level> node = pageTable->levelRootNode; // copy of level root node which is sharedf ptr

    int i = 0; // level index

    while (i < pageTable->levelDepths.size() - 1) //i here is the level
    {

        //convert to indexes
        unsigned int index = virtualAddressToPageNum(virtualAddress, node->tablePtr->bitmaskArray[i], node->tablePtr->shiftArray[i]);
        //word isnt finished since child is none existent
        pages[i] = index;
        //increase to next part of index
        node = node->nextLevelPtr[index];
        i++;
    }
    int leafIndex = i;
    unsigned int index = virtualAddressToPageNum(virtualAddress, node->tablePtr->bitmaskArray[leafIndex], node->tablePtr->shiftArray[leafIndex]); //index for leaf
    // we're now on leaf node so we check for
    pages[leafIndex] = index; //set pages leaf index to index
    return pages;             //return pointer of pages
}