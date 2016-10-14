#!/bin/bash -l
set -exo pipefail

function build_gpdb_tpch() {
  pushd gpdb_src/gpAux/extensions/tpch/source
  make
  popd
}

function verify() {
  pushd gpdb_src/gpAux/extensions/tpch/source
  if [[ ! -x "dbgen" ]]; then
    echo "'dbgen' bianry doesn't exist."
    exit 1
  fi

  if [[ ! -f "dists.dss" ]]; then
    echo "File 'dists.dss' doesn't exist."
    exit 1
  fi
}

function _main() {
  build_gpdb_tpch
  verify
}

_main "$@"
