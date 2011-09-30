
#include "TFile.h"
#include <stdio.h>
#include <iostream>

void nev(std::string file) {
    TFile* file0 = new TFile(file.c_str()) ;
    std::cout << "#Entries = " << Events->GetEntries() << std::endl ; 
}

