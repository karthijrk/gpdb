#!/bin/bash

log() {
    echo -e "$@"
}

error() {
    echo >&2 "$@"
    exit 1
}

main() {
    install_tools
    check_config
    run_instances
}

install_tools() {
    # ec2 toolset
    yum install -y java-1.8.0-openjdk
    wget http://s3.amazonaws.com/ec2-downloads/ec2-api-tools.zip
    mkdir /usr/local/ec2
    unzip ec2-api-tools.zip -d /usr/local/ec2
}

check_config() {
    # Error out when secret keys are not set
    if [[ -z $AWS_ACCESS_KEY_ID ]]; then
        error " AWS_ACCESS_KEY_ID must be specified."
    fi
    if [[ -z $AWS_SECRET_ACCESS_KEY ]]; then
        error "AWS_SECRET_ACCESS_KEY must be specified."
    fi
    if [[ -z $AWS_KEYPAIR ]]; then
        error "AWS_KEYPAIR must be specified."
    fi

    # Display values for non-secret configuration values
    if [[ -z $AWS_REGION ]]; then
        error "AWS_REGION must be specified."
    else
        log "AWS_REGION=${AWS_REGION}"
    fi
    if [[ -z $AMI ]]; then
        error "AMI must be specified."
    else
        log "AMI=${AMI}"
    fi
    if [[ -z $INSTANCE_TYPE ]]; then
        error "INSTANCE_TYPE must be specified."
    else
        log "INSTANCE_TYPE=${INSTANCE_TYPE}"
    fi
    if [[ -z $VPC_ID ]]; then
        error "VPC_ID must be specified."
    else
        log "VPC_ID=${VPC_ID}"
    fi
    if [[ -z $SUBNET_ID ]]; then
        error "SUBNET_ID must be specified."
    else
        log "SUBNET_ID=${SUBNET_ID}"
    fi
    if [[ -z $SSH_USER ]]; then
        error "SSH_USER must be specified."
    else
        log "SSH_USER=${SSH_USER}"
    fi
    if [[ -z $TENANCY ]]; then
        error "TENANCY must be specified."
    else
        log "TENANCY=${TENANCY}"
    fi
}

run_instances() {
    log "Starting instances"

    INSTANCE_IDS=($(
      ec2-run-instances $IMAGE_ID \
        -n 1 \
        --tenancy ${TENANCY} \
        --show-empty-fields \
        -k $AWS_KEYPAIR \
        --instance-type $INSTANCE_TYPE \
        #--subnet $SUBNET_ID \
        --associate-public-ip-address true \
        #${MAPPINGS} |
      grep INSTANCE |
      cut -f2
    ))

    log "Done."

}


main "$@"
