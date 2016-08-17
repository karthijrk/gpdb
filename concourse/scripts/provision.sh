#!/bin/bash

set +x

source $(dirname "$0")/common.sh

main() {
    check_config
	check_tools

    run_instances
#    set_networking

    print_addresses
}

check_tools() {
  if ! command -v ec2-run-instances >/dev/null 2>&1; then
    error "Amazon EC2 API Tools not installed (see http://aws.amazon.com/developertools/351)"
  fi
}


run_instances() {
    log "Starting instances"

    INSTANCE_IDS=($(
      ec2-run-instances $AMI \
        -n 1 \
        --tenancy ${TENANCY} \
        --show-empty-fields \
        -k $AWS_KEYPAIR \
        --instance-type $INSTANCE_TYPE \
        --associate-public-ip-address true |
      grep INSTANCE |
      cut -f2
    ))
	echo ${INSTANCE_IDS} > instance_info/instance_ids.txt

    log "Created instances: ${INSTANCE_IDS[*]}"

	local INSTANCE_ID=${INSTANCE_IDS}
	ec2-create-tags ${INSTANCE_ID} -t Name=${INSTANCE_NAME}

	wait_until_status "running"
    wait_until_check_ok
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

set_networking() {
  log "Enabling enhanced networking"

  ec2-stop-instances ${INSTANCE_IDS[*]}

  wait_until_status "stopped"

  for INSTANCE_ID in ${INSTANCE_IDS[*]}; do
    log "Enabling enhanced networking for ${INSTANCE_ID}"

    ec2-modify-instance-attribute $INSTANCE_ID --sriov simple
  done

  ec2-start-instances ${INSTANCE_IDS[*]}

  wait_until_status "running"
  wait_until_check_ok
}

print_addresses() {
  log "Provision complete\n\n"

  log "INSTANCE\tPUBLIC IP\tPRIVATE IP"

  ec2-describe-instances --show-empty-fields ${INSTANCE_IDS[*]} | grep INSTANCE | cut -f2,17,18
}

main "$@"
