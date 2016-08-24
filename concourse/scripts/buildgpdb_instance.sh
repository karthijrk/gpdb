#!/bin/bash

set +x
set -e

source $(dirname "$0")/common.sh

create_pem_file {
  KEYPAIR_FILE=$(mktemp /tmp/$AWS_KEYNAME.pem.XXX)
  echo $AWS_KEYPAIR > KEYPAIR_FILE
}

main() {
    INSTANCE_IDS=($(cat instance_ids/instance_ids.txt))

    check_config
    check_tools

    # Get Ip address
    ec2-describe-instances --help
    IPS=$(ec2-describe-instances --show-empty-fields ${INSTANCE_IDS[*]} | grep INSTANCE | cut -f17)

    # Create .pem file
    create_pem_file

    # run ssh to publish ec2 hostname name
    ssh -i "${KEYPAIR_FILE}" -t -t ${SSH_USER}@${IPS[*]} "hostname"
    log "Connection successful"
}

main "$@"
