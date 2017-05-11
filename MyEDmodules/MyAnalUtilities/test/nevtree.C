
#include "TChain.h"
#include <vector>
#include <string>
#include <iostream>

#include "utils.C"

void nevtree(std::string glob, std::string treename) {
    TChain* mychain = new TChain(treename.c_str()) ;

    std::vector<string> rootfiles;
    expandGlob(glob, rootfiles);

    for (size_t i=0; i<rootfiles.size(); i++)
      mychain->Add(rootfiles[i].c_str());

    std::cout << "tree # entries = " << mychain->GetEntries() << std::endl ; 
}

