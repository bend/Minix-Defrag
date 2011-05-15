#!/usr/pkg/bin/bash

success=0
i=0
while [ $success -eq "0" ]; do
  if [[ ! -f /mnt/test/file_$i ]]; then
    dd if=/dev/zero of=/mnt/test/file_$i bs=4096 count=3;
    success=$?
  fi
  i=$(( $i+1 ));
done
