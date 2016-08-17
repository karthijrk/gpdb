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
        error " AWS_ACCESS_KEY must be specified."
    fi
    if [[ -z $AWS_SECRET_KEY ]]; then
        error "AWS_SECRET_KEY must be specified."
    fi
    if [[ -z $AWS_KEYPAIR ]]; then
        error "AWS_KEYPAIR must be specified."
    fi

	log "Configuration"

    # Display values for non-secret configuration values
    if [[ -z $AWS_REGION ]]; then
        error "AWS_REGION must be specified."
    else
        log "AWS_REGION=${AWS_REGION}"
        export EC2_URL="https://ec2.${AWS_REGION}.amazonaws.com"
        log "EC2_URL=${EC2_URL}"
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
    if [[ -z $INSTANCE_NAME ]]; then
        error "INSTANCE_NAME must be specified."
    else
        log "INSTANCE_NAME=${INSTANCE_NAME}"
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
    if [[ -z $WAIT_SECS ]]; then
        error "WAIT_SECS must be specified."
    else
        log "WAIT_SECS=${WAIT_SECS}"
        export WAIT=${WAIT_SECS}
    fi
    if [[ -z $RETRIES ]]; then
        error "RETRIES must be specified."
    else
        log "RETRIES=${RETRIES}"
    fi

	log "=============================="
}

wait_until_status() {
  local STATUS=$1

  log "Waiting for status: ${STATUS}"

  local N=0
  until [[ $N -ge $RETRIES ]]; do
    local COUNT=$(
      ec2-describe-instances --show-empty-fields ${INSTANCE_IDS[*]} |
      grep INSTANCE |
      cut -f6 |
      grep -c ${STATUS}
    ) || true

    log "${COUNT} of ${#INSTANCE_IDS[@]} instances ${STATUS}"

    [[ "$COUNT" -eq "${#INSTANCE_IDS[@]}" ]] && break

    N=$(($N+1))
    sleep $WAIT
  done

  if [[ $N -ge $RETRIES ]]; then
    error "Timed out waiting for instances to reach status: ${STATUS}"
  fi
}

wait_until_check_ok() {
  local STATUS=$1

  log "Waiting for instances to pass status checks"

  local N=0
  until [[ $N -ge $RETRIES ]]; do
    local COUNT

    COUNT=$(
      ec2-describe-instance-status --show-empty-fields ${INSTANCE_IDS[*]} |
      grep -e "\bINSTANCE\b" |
      cut -f6,7 |
      xargs -n 2 |
      grep -c "ok ok"
    ) || true

    log "${COUNT} of ${#INSTANCE_IDS[@]} instances pass status checks"

    [[ "$COUNT" -eq "${#INSTANCE_IDS[@]}" ]] && break

    COUNT=$(
      ec2-describe-instance-status --show-empty-fields ${INSTANCE_IDS[*]} |
      grep -e "\bINSTANCE\b" |
      cut -f6,7 |
      xargs -n 2 |
      grep -c "impaired"
    ) || true

    if [[ "$COUNT" -gt 0 ]]; then
      error "${COUNT} of ${#INSTANCE_IDS[@]} failed to pass status checks"
    fi

    N=$(($N+1))
    sleep $WAIT
  done

  if [[ $N -ge $RETRIES ]]; then
    error "Timed out waiting for instances to pass status checks"
  fi
}
