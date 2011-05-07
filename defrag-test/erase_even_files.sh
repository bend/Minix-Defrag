#!/usr/pkg/bin/bash

i=0
while [[ -f /mnt/test/file_$i  ]] ; do
  rm /mnt/test/file_$i;
  i=$(( $i+2 ));
done

