#!/usr/pkg/bin/bash

i=0
while [ "$i" -lt "49" ] ; do
  rm /mnt/test/file_$i;
  i=$(( $i+2 ));
done

