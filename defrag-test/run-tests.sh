#!/usr/pkg/bin/bash

if [[ ! -d /mnt/test ]]; then
	echo "ERROR: this script works with a small disk partition mounted on /mnt/test, which was not found."
	exit 1
fi

while [[ -z $partsize ]]; do
  echo "INPUT: please enter the size of the partition mounted at /mnt/test/,  in Mb (natural number)"
  read partsize
done


echo "INFO: creating nfrags and defrag executables"
cc -o nfrags nfrags.c
cc -o defrag defrag.c


echo "INPUT: This script will erase all files in /mnt/test. Are you sure you want to continue? Type yes to continue"
read a
if [[ $a != "yes" ]]; then
	exit 2
fi


echo "INFO: erasing /mnt/test"

rm -rf /mnt/test/*

echo "INFO: seeding /dev/urandom"
cat ./random_seed.rnd > /dev/urandom

echo "INFO: creating 10 small files"
for i in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14 15; do
dd if=/dev/urandom of=/mnt/test/f$i bs=4096 count=$((4*$partsize)) 2>/dev/null;
done
echo "INFO: erasing uneven files to created fragmented areas"
for i in 1  2 3 4  5  7; do 
  rm /mnt/test/f$i
done

echo "INFO: creating fragmented file /mnt/test/frag"
dd if=/dev/urandom of=/mnt/test/frag bs=4096 count=$((10*$partsize)) 2>/dev/null;

echo "INFO: checking that we have multiple fragments"
fragments=$(./nfrags /mnt/test/frag | grep fragments | awk "{ print \$1}")

if (( $fragments > 1)) ; then 
	echo "** SUCCESS:  found $fragments fragments as expected "
else
	echo "-- bad luck, file not fragmented. Start again!"
	exit 3
fi

echo "INFO: Copying file to /tmp for comparison of content"
cp /mnt/test/frag /tmp

echo "INFO: Defragmenting file"
./defrag /mnt/test/frag

echo "INFO: Comparing defragmented file to original content"


if diff /mnt/test/frag /tmp/frag >/dev/null 2>&1  ; then
	echo "** SUCCESS: files were identical"
else
	echo "-- Boum!, Content was not identical"
	exit 4
fi;

echo "INFO: Checking that we have one fragment"
fragments=$(./nfrags /mnt/test/frag | grep fragments | awk "{ print \$1}")

if (( $fragments == 1 )) ; then 
	echo "** SUCCESS: found $fragments fragment, as expected"
else
	echo "-- Boom, test failed, we have $fragments fragments"
	exit 4
fi

echo "INFO: recreating fragmented file"
rm /mnt/test/frag
fragments=$(./nfrags /mnt/test/frag | grep fragments | awk "{ print \$1}")
dd if=/dev/urandom of=/mnt/test/frag bs=4096 count=$((10*$partsize)) 2>/dev/null;
echo "INFO: will now fill the partition"
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
#filling space left with small files
success=0
i=0
while [ $success -eq "0" ]; do
  if [[ ! -f /mnt/test/smallfile_$i ]]; then
    dd if=/dev/zero of=/mnt/test/smallfile_$i bs=4096 count=$((1*$partsize)) 2>/dev/null;
    success=$?
  fi
  i=$(( $i+1 ));
done
echo "INFO: partition full, will erase a file that does not leave enough place to defragment file, and defragment file"
rm /mnt/test/smallfile_0
./defrag /mnt/test/frag

if  (( $? != 0 ))  ; then
	echo "** SUCCESS: return status of defrag is different from zero as expected";
fi

echo "INFO: checking that we have multiple fragments as defrag should not succeed"
fragments=$(./nfrags /mnt/test/frag | grep fragments | awk "{ print \$1}")

if (( $fragments > 1)) ; then 
	echo "** SUCCESS: found $fragments fragments"
else
	echo "-- bad luck, file not fragmented. Start again!"
	exit 3
fi
echo "INFO: Making sufficient place, and trying defragmentation again, which should succeed"
rm /mnt/test/bigfile_0
./defrag /mnt/test/frag

echo "INFO: checking that we have only one fragment as expected"
fragments=$(./nfrags /mnt/test/frag | grep fragments | awk "{ print \$1}")

if (( $fragments == 1)) ; then 
	echo "** SUCCESS: found $fragments fragments"
else
	echo "-- bad luck, file still fragmented. Start again!"
	exit 3
fi

echo "INFO: checking that defragmenting a defragmented file does nothing"
./defrag /mnt/test/frag

if [ "$?" -ne "0" ]; then
	echo "** SUCCESS: defrag has a non zero return status as expected"
else
	echo "-- ERROR : defrag defragmented file, but this was not expected"
	exit 3
fi

cat <<EOM
*******************************************************************************
                                / _ \| |/ /                                   
                               | | | | ' /                                   
                               | |_| | . \                                    
                                \___/|_|\_\                                    
                                                                               
			 All tests passed successfully!
		  Code par Bauduin Raphaël et Daccache Benoit
********************************************************************************
EOM
