#!/usr/pkg/bin/bash
  
dd if=/dev/zero of=/mnt/test/big_file_1 bs=1024 count=100;
dd if=/dev/zero of=/mnt/test/big_file_2 bs=1024 count=200;
dd if=/dev/zero of=/mnt/test/big_file_3 bs=1024 count=100;
dd if=/dev/zero of=/mnt/test/big_file_4 bs=1024 count=100;
dd if=/dev/zero of=/mnt/test/big_file_5 bs=1024 count=200;
