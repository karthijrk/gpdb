#!/bin/bash

set +x
set -e

source $(dirname "$0")/common.sh

main() {
    INSTANCE_IDS=($(cat instance_ids/instance_ids.txt))

    check_config

    # Get Ip address
    IPS=ec2-describe-instances --show-empty-fields ${INSTANCE_IDS[*]} | grep INSTANCE | cut -f17

    # run ssh to publish ec2 hostname name
    ssh -i "${AWS_KEYPAIR}" -t -t ${SSH_USER}@${IPS[*]} "hostname"
    log "Connection successful"
}

main "$@"
