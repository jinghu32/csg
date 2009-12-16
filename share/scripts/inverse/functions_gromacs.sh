#!/bin/bash
# 
# Copyright 2009 The VOTCA Development Team (http://www.votca.org)
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
#

if [ "$1" = "--help" ]; then
  #we add \$GMXDATA in USES, because gromacs will need it
cat <<EOF
${0##*/}, version %version%
Functions useful for gromacs 4.0

NEEDS:

USES: sed die \$GMXDATA

PROVIDES: get_from_mdp
EOF
  exit 0
fi 

check_deps $0

get_from_mdp() {
  local res
  [[ -n "$1" ]] || { echo What?; exit 1;}
  res=$(sed -n -e "s#[[:space:]]*$1[[:space:]]*=[[:space:]]*\(.*\)\$#\1#p" grompp.mdp | sed -e 's#;.*##') || die "get_from_mdp failed" 
  [[ -n "$res" ]] || die "get_from_mdp: could not fetch $1"
  echo "$res"
}

export -f get_from_mdp