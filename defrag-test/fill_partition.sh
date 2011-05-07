#!/usr/pkg/bin/bash

i=0
while [ "$i" -lt "512" ] ; do
  dd if=/dev/zero of=/mnt/test/file_$i bs=1024 count=10;
  i=$(( $i+1 ));
done
