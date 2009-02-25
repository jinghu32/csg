// 
// File:   moleculeinfo.cc
// Author: ruehle
//
// Created on April 18, 2007, 6:35 PM
//

#include "molecule.h"
#include <iostream>

void Molecule::AddBead(Bead *bead, const string &name)
{
    _beads.push_back(bead);
    _bead_names.push_back(name);
    _beadmap[name] = _beads.size()-1;
}

int Molecule::getBeadByName(const string &name)
{
    map<string, int>::iterator iter = _beadmap.find(name);
    if(iter == _beadmap.end()) {
        std::cout << "cannot find: <" << name << "> in " << _name << "\n";
        return -1;        
    }
    //assert(iter != _beadmap.end());
    //return (*iter).second;
    return _beadmap[name];
}