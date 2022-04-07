/*
* Name1, RedID1, Name2, RedID2
* Jason Lee, 823622872, Galanafai Windross, 823181886
*/

#include "TLB.hpp"

unsigned int findReplacement(TLB *cache) // find least accesed time value vpn
{
    unsigned int leastAccessedVpn = cache->LRU.begin()->first; // first key in cache
    unsigned int leastAccessVal = cache->LRU.begin()->second;  // first value in cache

    for (auto const &x : cache->LRU)
    {
        if (x.second < leastAccessVal)
        {                               // found lower acces time in map
            leastAccessVal = x.second;  // value
            leastAccessedVpn = x.first; // key
        }
    }

    return leastAccessedVpn; // return vpn with lowest access time which is the key
}