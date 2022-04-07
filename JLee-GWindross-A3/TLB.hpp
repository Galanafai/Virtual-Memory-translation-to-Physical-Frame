/*
* Name1, RedID1, Name2, RedID2
* Jason Lee, 823622872, Galanafai Windross, 823181886
*/

#ifndef TLB_HPP
#define TLB_HPP

#include <algorithm>
#include <vector>
#include <unordered_map>
#include <map>

struct TLB //structure for level
{
    int size; //size of TLB
    bool hit; //stores if hit or miss should be an vecotr of hits in main or page table IDK
    std::vector<unsigned int> TLBVPN; //array of LRU VPNS with max size size
    //int * LRUVPN;
    std::unordered_map<unsigned int, unsigned int> vpnTopfn; //map of vpn to pfn. just store frame

    int lruSize = 10;
    std::map<unsigned int, unsigned int> LRU; //map of vpn to acess time. just store frame
    int accessTime;
};


unsigned int findReplacement(TLB * cache);  //returns vpn in LRU of vpn with the least amount of access time


#endif