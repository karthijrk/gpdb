#!/bin/bash

set +x

source $(dirname "$0")/common.sh

main() {
    INSTANCE_IDS=($(cat instance_ids/instance_ids.txt))
    log "===== ${INSTANCE_IDS} ==="
}

main "$@"
