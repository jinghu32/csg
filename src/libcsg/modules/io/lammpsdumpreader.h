/* 
 * Copyright 2009-2018 The VOTCA Development Team (http://www.votca.org)
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 */

#ifndef _VOTCA_CSG_LAMMPSDUMPREADER_H
#define	_VOTCA_CSG_LAMMPSDUMPREADER_H

#include <string>
#include <iostream>
#include <fstream>
#include <votca/csg/topologyreader.h>
#include <votca/csg/trajectoryreader.h>

namespace votca { namespace csg {
using namespace votca::tools;

using namespace std;

/**
    \brief class for reading lammps dump files

    This class provides the TrajectoryReader + Topology reader interface
    for lammps dump files

*/
class LAMMPSDumpReader : 
    public TrajectoryReader, public TopologyReader
{
    public:
        LAMMPSDumpReader() {}
        ~LAMMPSDumpReader() {}
        
        /// open a topology file
        bool ReadTopology(string file, Topology &top);

        /// open a trejectory file
        bool Open(const string &file);
        /// read in the first frame
        bool FirstFrame(Topology &top);
        /// read in the next frame
        bool NextFrame(Topology &top);

        void Close();
        
    private:

        void ReadTimestep(Topology &top, string itemline);
        void ReadBox(Topology &top, string itemline);
        void ReadNumAtoms(Topology &top, string itemline);
        void ReadAtoms(Topology &top, string itemline);
  
        ifstream _fl;
        string _fname;
        bool _topology;
        int _natoms;
};

}}

#endif	// _VOTCA_CSG_LAMMPSDUMPREADER_H


