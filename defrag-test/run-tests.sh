#!/usr/pkg/bin/bash

if [[ ! -d /mnt/test ]]; then
	echo "this script works with a small disk partition mounted on /mnt/test, which was not found."
	exit 1
fi

while [[ -z $partsize ]]; do
  echo "please enter the size of the partition mounted at /mnt/test/,  in Mb (natural number)"
  read partsize
done


echo "creating nfrags and defrag executables"
cc -o nfrags nfrags.c
cc -o defrag defrag.c


echo "This script will erase all files in /mnt/test. Are you sure you want to continue? Type yes to continue"
read a
if [[ $a != "yes" ]]; then
	exit 2
fi


echo "erasing /mnt/test"
rm /mnt/test/*

echo "creating 10 small files"
for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do
dd if=/dev/urandom of=/mnt/test/f$i bs=4096 count=$((1*$partsize)) 2>/dev/null;
done

echo "erasing uneven files to created fragmented areas"
for i in 1  2 3 4  5  7; do 
  rm /mnt/test/f$i
done

echo "creating fragmented file /mnt/test/frag"
dd if=/dev/urandom of=/mnt/test/frag bs=4096 count=$((10*$partsize)) 2>/dev/null;

echo "checking that we have multiple fragments"
fragments=$(./nfrags /mnt/test/frag | grep fragments | awk "{ print \$1}")

if (( $fragments > 1)) ; then 
	echo "found $fragments fragments"
else
	echo "bad luck, file not fragmented. Start again!"
	exit 3
fi

echo "Defragmenting file"
./defrag /mnt/test/frag

echo "checking that we have one fragment"
fragments=$(./nfrags /mnt/test/frag | grep fragments | awk "{ print \$1}")

if (( $fragments == 1 )) ; then 
	echo "found $fragments fragment, as expected"
else
	echo "Boom, test failed, we have $fragments fragments"
	exit 4
fi

echo recreating fragmented file
rm /mnt/test/frag
fragments=$(./nfrags /mnt/test/frag | grep fragments | awk "{ print \$1}")
dd if=/dev/urandom of=/mnt/test/frag bs=4096 count=$((10*$partsize)) 2>/dev/null;
echo "will now fill the partition"
#creating big files first
success=0
i=0
while [ $success -eq "0" ]; do
  if [[ ! -f /mnt/test/bigfile_$i ]]; then
    dd if=/dev/zero of=/mnt/test/bigfile_$i bs=4096 count=$((30*$partsize)) 2>/dev/null;
    success=$?
  fi
  i=$(( $i+1 ));
done
#filling space left
success=0
i=0
while [ $success -eq "0" ]; do
  if [[ ! -f /mnt/test/smallfile_$i ]]; then
    dd if=/dev/zero of=/mnt/test/smallfile_$i bs=4096 count=$((1*$partsize)) 2>/dev/null;
    success=$?
  fi
  i=$(( $i+1 ));
done
echo "partition full, will erase a file that does not leave enough place to defragment file, and defragment file"
rm /mnt/test/smallfile0
./defrag /mnt/test/frag

if  ! $? ; then
	echo "return status of defrag is different from zero as expected";
fi

echo "checking that we have multiple fragments as defrag should not succeed"
fragments=$(./nfrags /mnt/test/frag | grep fragments | awk "{ print \$1}")

if (( $fragments > 1)) ; then 
	echo "OK.. found $fragments fragments"
else
	echo "bad luck, file not fragmented. Start again!"
	exit 3
fi
echo "makeing sufficient place, and trying defragmentation again, which should succeed"
rm /mnt/test/bigfile_0
./defrag /mnt/test/frag

echo "checking that we have only one fragment as expected"
fragments=$(./nfrags /mnt/test/frag | grep fragments | awk "{ print \$1}")

if (( $fragments == 1)) ; then 
	echo "OK.. found $fragments fragments"
else
	echo "bad luck, file still fragmented. Start again!"
	exit 3
fi

echo "checking that defragmenting a defragmented file does nothing"
./defrag /mnt/test/frag

