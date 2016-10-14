#!/bin/bash -l
set -exo pipefail

main() {

  local INSTALLER_PATH
  INSTALLER_PATH="$(ls gpdb_installer/*.zip)"
  echo "Found installer : ${INSTALLER_PATH}"

  local INSTALLER_NAME
  INSTALLER_NAME="$(basename ${INSTALLER_PATH} .zip)"

  echo "Running the installer..."
  "INSTALLER_NAME=${INSTALLER_NAME}" scripts/install_gpdb.bash
  "INSTALLER_NAME=${INSTALLER_NAME} VERIFY=1" scripts/install_gpdb.bash

  echo "Starting GPDB cluster..."
  scripts/start_gpdb.bash
  "VERIFY=1" scripts/start_gpdb.bash
}

main "$@"
