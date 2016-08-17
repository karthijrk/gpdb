#!/bin/bash

log() {
    echo -e "$@"
}

error() {
    echo >&2 "$@"
    exit 1
}

check_config() {
    # Error out when secret keys are not set
    if [[ -z $AWS_ACCESS_KEY ]]; then
        error "\$AWS_ACCESS_KEY must be specified."
    fi
    if [[ -z $AWS_SECRET_KEY ]]; then
        error "\$AWS_SECRET_KEY must be specified."
    fi
    if [[ -z $AWS_KEYPAIR ]]; then
        error "\$AWS_KEYPAIR must be specified."
    fi

    # Display values for non-secret configuration values
    if [[ -z $AMI ]]; then
        error "\$AMI must be specified."
    else
        log "\$AMI=${AMI}"
    fi
    if [[ -z $INSTANCE_TYPE ]]; then
        error "\$INSTANCE_TYPE must be specified."
    else
        log "\$INSTANCE_TYPE=${INSTANCE_TYPE}"
    fi
    if [[ -z $VPC_ID ]]; then
        error "\$VPC_ID must be specified."
    else
        log "\$VPC_ID=${VPC_ID}"
    fi
    if [[ -z $SUBNET_ID ]]; then
        error "\$SUBNET_ID must be specified."
    else
        log "\$SUBNET_ID=${SUBNET_ID}"
    fi
    if [[ -z $SSH_USER ]]; then
        error "\$SSH_USER must be specified."
    else
        log "\$SSH_USER=${SSH_USER}"
    fi
    if [[ -z $IMAGE_NAME ]]; then
        error "\$IMAGE_NAME must be specified."
    else
        log "\$IMAGE_NAME=${IMAGE_NAME}"
    fi
    if [[ -z $TENANCY ]]; then
        error "\$TENANCY must be specified."
    else
        log "\$TENANCY=${TENANCY}"
    fi
}

main() {
    check_config
}


main "$@"
