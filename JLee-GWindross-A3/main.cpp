/*
* Name1, RedID1, Name2, RedID2
* Jason Lee, 823622872, Galanafai Windross, 823181886
*/

#include "page_table.hpp"
#include "TLB.hpp"
#include "tracereader.hpp"
#include "output_mode_helpers.hpp"

/*
* Main logic for running and printing to stdout
* Usage:
*    make
*    replace between the curly braces with the types specified after colon
*    ./pagingwithtlb {traceFileName:string} -n {addressesToProcess:int} -o {outputMode:string} -c {cacheSize:int} l1:int l2:int ln:int
*/
int main(int argc, char **argv)
{

  bool givenNMemoryReferences = false;   // if the -n argument is used
  long firstNMemoryReferences = INT_MAX; /* processes all addresses if not specified set to maximum int value in c++ defualt */
  long tlbCacheCapacity = 0;             /* default is 0 if not specified */
  std::string outputMode = "summary";    /* summary mode is default */
                                         /* create new instance of page table struct */
  PageTable *pt = new PageTable();       // create page table
  bool TLBSet = false;                   // bool for TLB depedngs on cmd line arguments

  int opt; // operand for our getopt

  while ((opt = getopt(argc, argv, "n:c:o:")) != -1) //get optional and command line arhuments
  {
    switch (opt)
    {
    case 'n': // how many addresses to read
              //if(atol(optarg) > 0){
      firstNMemoryReferences = atol(optarg);
      //}
      break;

    case 'c': // desired cache size
      tlbCacheCapacity = atol(optarg);
      if (tlbCacheCapacity < 0)
      {
        printf("Cache capacity must be a number, greater than or equal to 0"); // can't have a negative cache size
        exit(-1);
      }
      else
      {
        TLBSet = true; // use the cache, as a size was specified
      }
      break;

    case 'o': // which type of output mode
      outputMode = optarg;
      break;

    default:
      /* print something about the usage and exit */
      exit(1); /* BADFLAG is an error # defined in a header */
    }
  }

  int idx = optind; // index through the arguments provided
  std::string fileName = "";
  int levelsIdx = idx;

  if (idx < argc) // there is more to process
  {
    fileName = argv[idx];
    for (int i = idx; i < argc; i++)
    {
      std::string currentArgument = argv[i];
      if (currentArgument.compare("-n") == 0) // -n arguments were provided
      {
        firstNMemoryReferences = atol(argv[i + 1]);
        givenNMemoryReferences = true;
        levelsIdx += 2; // process after the -n
      }
      else if (currentArgument.compare("-o") == 0) // -o arguments were provided
      {
        outputMode = argv[i + 1];
        levelsIdx += 2; // process after the -o
      }
      else if (currentArgument.compare("-c") == 0) // -c arguments were provided
      {
        tlbCacheCapacity = atol(argv[i + 1]);
        TLBSet = true;  // tlb is going to be used
        levelsIdx += 2; // process after the -c
      }
    }
    /* this loop gets us our different amount of bits per level */
    for (int i = levelsIdx + 1; i < argc; i++)
    {
      pt->levelDepths.push_back(atoi(argv[i]));
    }
  }

  for (int idxDepths = 0; idxDepths < pt->levelDepths.size(); idxDepths++)
  {
    if (pt->levelDepths[idxDepths] < 1)
    {
      printf("Level %i page table must be at least 1 bit", idxDepths); // can't have zero or negative sizes
      return 0;
    }
  }

  /* for determining if file openable */
  bool notLoadable = false;
  std::ifstream f1;
  f1.open(fileName.c_str());
  if (!(f1.is_open()))
  {
    notLoadable = true;
  }

  /* if the file by given name is not able to be opened */
  if (notLoadable == true)
  {
    std::cout << "Unable to open "
              << "<<" << fileName << ">>" << std::endl;
    exit(1);
  }

  int offsetSize = 0;

  /* summarize the level sizes */
  for (int i : pt->levelDepths)
  {
    // std::cout << "level sizes: " << i << std::endl;
    offsetSize += i;
  }

  int allLevels = offsetSize;
  offsetSize = 32 - offsetSize;

  //check if size of vpn is too big
  if (allLevels > MAX_BIT_USE)
  {
    printf("Too many bits used in page tables");
    return 0;
  }

  int levelCount = pt->levelDepths.size(); //set the num of levels to how many level arguments we god
  // printf("and we have a levelCount of %i\n", levelCount);

  /* load up page table level count */
  pt->levelCount = levelCount; //sace to level count
  /* loading up the page tables entry count vector */
  for (int i = 0; i < levelCount; i++)
  {
    pt->entryCount.push_back(pow(2, pt->levelDepths[i]));
  }

  int x = MACHINE_SIZE; // x would be 32
  for (int i : pt->levelDepths)
  {
    x = x - i;                   //get the shift shift for each level
    pt->shiftArray.push_back(x); //save to page table
  }

  /* filling bitmask array in page table*/
  for (int i = 0; i < pt->shiftArray.size(); i++)
  {
    pt->bitmaskArray.push_back(calculateMask(pt->shiftArray[i], pt->levelDepths[i]));
  }

  /* booleans for which output mode to print */
  bool bitmasks = false; //bool for output modes bitmasks
  bitmasks = outputMode.compare("bitmasks") == 0;

  bool virtual_to_physical = false; //bool for output mode virtual2physical
  virtual_to_physical = outputMode.compare("virtual2physical") == 0;

  bool v2p_tlb_pt = false; //bool for output mode v2p_tlb_pt
  v2p_tlb_pt = outputMode.compare("v2p_tlb_pt") == 0;

  bool vpn2pfn = false; //bool for output mode vpn2pfn
  vpn2pfn = outputMode.compare("vpn2pfn") == 0;

  bool offset_print = false; //bool for output mode offset
  offset_print = outputMode.compare("offset") == 0;

  bool report_summary_mode = true; //report summary will be defualt case

  if (bitmasks || virtual_to_physical || v2p_tlb_pt || vpn2pfn || offset_print)
  {
    report_summary_mode = false; // if any output mode specified remote summary will be false
  }

  /* case that -o is bitmasks */
  if (bitmasks)
  {
    printf("Bitmasks\n");
    if (levelCount > 1)
    {
      for (int i = 0; i < levelCount; i++)
      {
        std::cout << "level " << i << " mask ";
        printf("%08X\n", pt->bitmaskArray[i]);
      }
    }
    return 0;
  }

  p2AddrTr *p2 = new p2AddrTr();  //create new trace reader
  struct TLB *cache = new TLB;    //create new TLB
  cache->size = tlbCacheCapacity; //set TLB to specifidee tlb capacity if given
  FILE *ifp;                      /* trace file */

  ifp = fopen(fileName.c_str(), "rb");        /* we have the name of the file above. aborted already if wrong */
  unsigned long ip = 0;                       /* instructions processed */
  p2AddrTr trace;                             /* traced address */
  int amountLooped = 0;                       //virtual time + iterator for while loop
  int amountOfLoops = firstNMemoryReferences; /* loop full file if not given first n reference count -n */

  if (givenNMemoryReferences)
  {
    amountOfLoops = firstNMemoryReferences; /* This is how many times we'll get next addresses */
  }

  int frameCounter = 0; //start frame counter
  unsigned int offsetMask = calculateMask(0, offsetSize);
  pt->offSetMask = offsetMask;

  int numPageHit = 0;    //sabbe num of page gits
  int numPageMiss = 0;   //save num of page misses
  int numTlbHit = 0;     //save num of TLB hit
  int numTlbMiss = 0;    //save num of TL
  int pageTableSize = 0; //pagetable size iterator
  int entryCount = 0;    // increate each time

  while (!feof(ifp) && amountLooped < amountOfLoops) /* continue looping until we have hit the n memory vals desired */
  {

    bool tlbHit = false; //for tlb miss and hit
                         // int totaladr = 0

    bool pageHit = false; //bool for if page was hit
    /* get next address and process */
    if (NextAddress(ifp, &trace))
    {
      unsigned int vpn = AddressDecodeReturned(&trace) >> offsetSize; // get vpn
      if (TLBSet)                                                     //logic that runs if tlb was set
      {

        if (cache->TLBVPN.size() == 0) //add first entry to cache and check if cache is set in cmd line
        {

          cache->TLBVPN.push_back(vpn);                                    //pushback first vpn to tlb
          pageInsert(pt, AddressDecodeReturned(&trace), pt->frameCounter); //insert first vpn to the page along with its mapping
          cache->vpnTopfn[vpn] = pt->frameCounter;                         //insert vpn to hashmap within cache
          cache->LRU[vpn] = amountLooped;                                  //use amount loop as acces time
        }
        else
        {

          //search to find if vpn is in tlb or not
          //if not in tlb do page miss
          // if size is full do rearagning and if not simply push back

          for (int i = 0; i < (cache->TLBVPN.size()); i++) // looks at most 10 recent acsessed pages
          {

            if (vpn == cache->TLBVPN[i])
            {
              //this is a tlb hit. we break and rotate to the front of LRU
              numTlbHit++;   // increrase tlb hit
              tlbHit = true; //set tlb hit to true

              cache->LRU[vpn] = amountLooped; //update acces time and isnertt key and value to LRU

              break;
            }
          }

          if (tlbHit == false) // do page look up
          {

            numTlbMiss++; // increa tlb miss
            //we got to the end so we check if size is full or not
            if (cache->TLBVPN.size() == cache->size) //size is full
            {
              //if cache is full we need to do rearanging with LRU policy
              //delete lastaccessed  element and push curr vpn to the front
              unsigned int vpnToReplace = findReplacement(cache); //get vpn to replace
              for (int i = 0; i < cache->TLBVPN.size(); i++)
              {
                if (cache->TLBVPN[i] == vpnToReplace)
                {                                                   // remnove this index from TLB
                  cache->TLBVPN.erase(cache->TLBVPN.begin() + (i)); //erase this vpn index; already removed in LRU in find replacement
                }
              }
              cache->TLBVPN.insert(cache->TLBVPN.begin(), vpn); //inset new vpn to fron of cache
              cache->LRU[vpn] = amountLooped;                   //update acces time and isnertt key and value to LRU
            }
            else if (cache->TLBVPN.size() < cache->size)
            { //tlb isnt full and wasnt hit

              cache->LRU[vpn] = amountLooped; //update acces time and isnertt key and value to LRU

              cache->TLBVPN.insert(cache->TLBVPN.begin(), vpn); //insert vpn to front of cache
            }

            if (pageLookup(pt, AddressDecodeReturned(&trace)) != nullptr) // page was  found in page table
            {
              numPageHit++;                                                                //increase page hit
              pageHit = true;                                                              //set bool to true
              cache->vpnTopfn[vpn] = (pageLookup(pt, AddressDecodeReturned(&trace)))->pfn; //map cache vpn with frame
            }

            else
            {                                                                              //we create a new page since tlb miss and page miss
              numPageMiss++;                                                               // increa page miss
              pageInsert(pt, AddressDecodeReturned(&trace), pt->frameCounter);             //inset completly new page
              cache->vpnTopfn[vpn] = (pageLookup(pt, AddressDecodeReturned(&trace)))->pfn; //map cache vpn with frame
            }
          }
        }
        if (cache->LRU.size() > cache->lruSize) //lru is at capacity
        {
          unsigned int vpnToReplace = findReplacement(cache); //get LRU vpn to replace
          cache->LRU.erase(vpnToReplace);                     //delete vpn from LRU since we found it with least access time
        }
        if (v2p_tlb_pt) //show v to p translanation for evey address and TLB
        {
          unsigned int offset = virtualAddressToOffset(AddressDecodeReturned(&trace), pt->offSetMask);          //set offset
          unsigned int PA = (pageLookup(pt, AddressDecodeReturned(&trace))->pfn * pow(2, offsetSize)) + offset; //calculate phsycial address
          report_v2pUsingTLB_PTwalk(AddressDecodeReturned(&trace), PA, tlbHit, pageHit);                        //call output method in helpers
        }
      }
      else // tlb is not set so different logic
      {
        if (pageTableSize == 0) //insert root since page size is 0
        {
          pageTableSize++;                                                 //increase the pageTable size
          numPageMiss++;                                                   //initial miss so increase the miss
          pageInsert(pt, AddressDecodeReturned(&trace), pt->frameCounter); //inset page
        }
        else
        {
          if (pageLookup(pt, AddressDecodeReturned(&trace)) != nullptr) // page was  found in page table
          {
            numPageHit++;   //increase page hit
            pageHit = true; //set page hit to true
          }
          else
          {                //page miss
            numPageMiss++; // increa page miss

            pageInsert(pt, AddressDecodeReturned(&trace), pt->frameCounter);
          }
        }

        /* offset used for the reporting functions */
        unsigned int offset = virtualAddressToOffset(AddressDecodeReturned(&trace), pt->offSetMask);
        if (virtual_to_physical)
        { //add frame and offset of

          unsigned int PA = (pageLookup(pt, AddressDecodeReturned(&trace))->pfn * pow(2, offsetSize)) + offset; //calculate phsycial address
          report_virtual2physical(AddressDecodeReturned(&trace), PA);                                           //call output mode helpers
        }
        if (offset_print)
        {
          hexnum(offset); //call output mode helpers to print offset
        }
        if (vpn2pfn)
        {

          unsigned int *pages = new unsigned int[pt->levelDepths.size()];                                      //pages array = num of levels
          pages = vpnLookup(pages, pt, AddressDecodeReturned(&trace));                                         //get the vpn for current addr
          report_pagemap(pt->levelDepths.size(), pages, (pageLookup(pt, AddressDecodeReturned(&trace))->pfn)); //call output ode helper
          pages = nullptr;                                                                                     // save from memory leak and ealocate since out of scope
        }
      }

      amountLooped++;
    }
  }

  /* clean up and return success */
  fclose(ifp);
  int totalBytes = (pt->entries) * BYTE_SIZE;

  int framesAllocated = amountLooped - numPageHit - numTlbHit;
  int pageSize = pow(2, offsetSize);

  if (report_summary_mode)
  {
    report_summary(pageSize, numTlbHit, numPageHit, amountLooped, framesAllocated, totalBytes);
  }

  /* remove and cleanup used pointers */
  delete pt;
  delete p2;
  delete cache;

  return 0;
}
