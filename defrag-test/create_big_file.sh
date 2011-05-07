#!/usr/pkg/bin/bash
  
dd if=/dev/zero of=/mnt/test/big_file_1 bs=1024 count=4;
# at least 4 fragments
dd if=/dev/zero of=/mnt/test/big_file_2 bs=1024 count=10;
# at least 10 fragments
dd if=/dev/zero of=/mnt/test/big_file_2 bs=1024 count=100;
# at least 100 fragments
