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

#include <stdexcept>
#include <votca/csg/interaction.h>
#include <votca/csg/topology.h>
#include <votca/tools/rangeparser.h>

namespace votca {
namespace csg {

Topology::~Topology() {
  Cleanup();
  if (_bc)
    delete (_bc);
  _bc = NULL;
}

void Topology::Cleanup() {
  _beads.clear();
  _molecules.clear();
  _residues.clear();
  _interactions.clear();
  // cleanup _bc object
  if (_bc)
    delete (_bc);
  _bc = new OpenBox();
}

shared_ptr<Bead> Topology::CreateBead(byte_t symmetry, string name,
                                      shared_ptr<BeadType> type, int resnr,
                                      double m, double q) {

  auto b = new Bead(std::make_shared<Topology>(*this), _beads.size(), type,
                    symmetry, name, resnr, m, q);
  auto shared_b = std::make_shared<Bead>(*b);
  _beads.push_back(shared_b);
  return shared_b;
}

shared_ptr<Molecule> Topology::CreateMolecule(string name) {
  auto mol =
      new Molecule(std::make_shared<Topology>(*this), _molecules.size(), name);
  auto shared_mol = std::make_shared<Molecule>(*mol);
  _molecules.push_back(shared_mol);
  return shared_mol;
}

shared_ptr<Residue> Topology::CreateResidue(string name, int id) {
  auto res = new Residue(std::make_shared<Topology>(*this), id, name);
  auto shared_res = std::make_shared<Residue>(*res);
  _residues.push_back(shared_res);
  return shared_res;
}

shared_ptr<Residue> Topology::CreateResidue(string name) {
  auto res =
      new Residue(std::make_shared<Topology>(*this), _molecules.size(), name);
  auto shared_res = std::make_shared<Residue>(*res);
  _residues.push_back(shared_res);
  return shared_res;
}

shared_ptr<Molecule> Topology::MoleculeByIndex(int index) {
  return _molecules[index];
}

/// \todo implement checking, only used in xml topology reader
void Topology::CreateMoleculesByRange(string name, int first, int nbeads,
                                      int nmolecules) {
  auto mol = CreateMolecule(name);
  int beadcount = 0;
  int res_offset = 0;

  for (auto bead : _beads) {
    // xml numbering starts with 1
    if (--first > 0)
      continue;
    // This is not 100% correct, but let's assume for now that the resnr do
    // increase
    if (beadcount == 0) {
      res_offset = (bead)->getResnr();
    }
    mol->AddBead(bead);
    if (++beadcount == nbeads) {
      if (--nmolecules <= 0)
        break;
      mol = CreateMolecule(name);
      beadcount = 0;
    }
  }
}

void Topology::CreateMoleculesByResidue() {
  // first create a molecule for each residue
  for (auto res : _residues) {
    CreateMolecule(res->getName());
  }

  // add the beads to the corresponding molecules based on their resid
  for (auto bead : _beads) {
    MoleculeByIndex(bead->getResnr())->AddBead(bead);
  }

  /// \todo sort beads in molecules that all beads are stored in the same order.
  /// This is needed for the mapping!
}

void Topology::CreateOneBigMolecule(string name) {
  auto mi = CreateMolecule(name);
  for (auto bead : _beads) {
    mi->AddBead(bead);
  }
}

void Topology::Add(shared_ptr<Topology> top) {
  int res0 = ResidueCount();

  for (auto bead : top->_beads) {
    auto type = GetOrCreateBeadType(bead->getType()->getName());
    CreateBead(bead->getSymmetry(), bead->getName(), type,
               bead->getResnr() + res0, bead->getM(), bead->getQ());
  }

  for (auto res : top->_residues) {
    CreateResidue(res->getName());
  }

  for (auto mol : top->_molecules) {
    auto mi = CreateMolecule(mol->getName());
    for (int i = 0; i < mi->BeadCount(); i++) {
      mi->AddBead(mi->getBead<Bead>(i));
    }
  }
}

void Topology::CopyTopologyData(shared_ptr<Topology> top) {
  _bc->setBox(top->getBox());
  _time = top->_time;
  _step = top->_step;

  // cleanup old data
  Cleanup();

  // copy all residues
  for (auto res : top->_residues) {
    CreateResidue(res->getName());
  }

  // create all beads
  for (auto bead : top->_beads) {
    auto type = GetOrCreateBeadType(bead->getType()->getName());
    auto bn = CreateBead(bead->getSymmetry(), bead->getName(), type,
                         bead->getResnr(), bead->getM(), bead->getQ());

    bn->setOptions(bead->Options());
  }

  // copy all molecules
  for (auto mol : top->_molecules) {
    auto mi = CreateMolecule(mol->getName());
    for (int i = 0; i < mol->BeadCount(); i++) {
      int beadid = mol->getBead<Bead>(i)->getId();
      mi->AddBead(_beads[beadid]);
    }
  }
}

void Topology::RenameMolecules(string range, string name) {
  RangeParser rp;
  RangeParser::iterator i;

  rp.Parse(range);
  for (i = rp.begin(); i != rp.end(); ++i) {
    if ((unsigned int)*i > _molecules.size())
      throw runtime_error(
          string("RenameMolecules: num molecules smaller than"));
    getMolecule(*i - 1)->setName(name);
  }
}

void Topology::RenameBeadType(string name, string newname) {
  for (auto bead : _beads) {
    auto type = GetOrCreateBeadType(bead->getType()->getName());
    if (wildcmp(name.c_str(), bead->getType()->getName().c_str())) {
      type->setName(newname);
    }
  }
}

void Topology::SetBeadTypeMass(string name, double value) {
  for (auto bead : _beads) {
    if (wildcmp(name.c_str(), bead->getType()->getName().c_str())) {
      bead->setM(value);
    }
  }
}

void Topology::CheckMoleculeNaming(void) {
  map<string, int> nbeads;

  for (auto mol : _molecules) {
    map<string, int>::iterator entry = nbeads.find(mol->getName());
    if (entry != nbeads.end()) {
      if (entry->second != mol->BeadCount())
        throw runtime_error("There are molecules which have the same name but "
                            "different number of bead "
                            "please check the section manual topology handling "
                            "in the votca manual");
      continue;
    }
    nbeads[mol->getName()] = mol->BeadCount();
  }
}

void Topology::AddBondedInteraction(std::shared_ptr<Interaction> ic) {
  map<string, int>::iterator iter;
  iter = _interaction_groups.find(ic->getGroup());
  if (iter != _interaction_groups.end())
    ic->setGroupId((*iter).second);
  else {
    int i = _interaction_groups.size();
    _interaction_groups[ic->getGroup()] = i;
    ic->setGroupId(i);
  }
  _interactions.push_back(ic);
  _interactions_by_group[ic->getGroup()].push_back(ic);
}

list<shared_ptr<Interaction>>
Topology::InteractionsInGroup(const string &group) {
  map<string, list<shared_ptr<Interaction>>>::iterator iter;
  iter = _interactions_by_group.find(group);
  if (iter == _interactions_by_group.end())
    return list<shared_ptr<Interaction>>();
  return iter->second;
}

shared_ptr<BeadType> Topology::GetOrCreateBeadType(string name) {

  auto iter = _beadtype_map.find(name);
  if (iter == _beadtype_map.end()) {
    auto bt =
        new BeadType(make_shared<Topology>(*this), _beadtypes.size(), name);
    auto shared_bt = make_shared<BeadType>(*bt);
    _beadtypes.push_back(shared_bt);
    _beadtype_map[name] = shared_bt->getId();
    return shared_bt;
  } else {
    return _beadtypes[(*iter).second];
  }
  throw runtime_error("Unable to get or create bead type");
}

vec Topology::BCShortestConnection(const vec &r_i, const vec &r_j) const {
  return _bc->BCShortestConnection(r_i, r_j);
}

vec Topology::getDist(int bead1, int bead2) const {
  return BCShortestConnection(getBead(bead1)->getPos(),
                              getBead(bead2)->getPos());
}

double Topology::BoxVolume() { return _bc->BoxVolume(); }

void Topology::RebuildExclusions() { _exclusions.CreateExclusions(this); }

BoundaryCondition::eBoxtype Topology::autoDetectBoxType(const matrix &box) {
  // set the box type to OpenBox in case "box" is the zero matrix,
  // to OrthorhombicBox in case "box" is a diagonal matrix,
  // or to TriclinicBox otherwise
  if (box.get(0, 0) == 0 && box.get(0, 1) == 0 && box.get(0, 2) == 0 &&
      box.get(1, 0) == 0 && box.get(1, 1) == 0 && box.get(1, 2) == 0 &&
      box.get(2, 0) == 0 && box.get(2, 1) == 0 && box.get(2, 2) == 0) {
    return BoundaryCondition::typeOpen;
  } else if (box.get(0, 1) == 0 && box.get(0, 2) == 0 && box.get(1, 0) == 0 &&
             box.get(1, 2) == 0 && box.get(2, 0) == 0 && box.get(2, 1) == 0) {
    return BoundaryCondition::typeOrthorhombic;
  } else {
    return BoundaryCondition::typeTriclinic;
  }
  return BoundaryCondition::typeOpen;
}

double Topology::ShortestBoxSize() {
  vec _box_a = getBox().getCol(0);
  vec _box_b = getBox().getCol(1);
  vec _box_c = getBox().getCol(2);

  // create plane normals
  vec _norm_a = _box_b ^ _box_c;
  vec _norm_b = _box_c ^ _box_a;
  vec _norm_c = _box_a ^ _box_b;

  _norm_a.normalize();
  _norm_b.normalize();
  _norm_c.normalize();

  double la = _box_a * _norm_a;
  double lb = _box_b * _norm_b;
  double lc = _box_c * _norm_c;

  return min(la, min(lb, lc));
}
} // namespace csg
} // namespace votca
