echo "This script will erase all files in /mnt/test. Are you sure you want to continue? Type yes to continue"
read a
if [[ $a != "yes" ]] then
	exit
fi

echo "jkljklmhjkldhfljfhdjlsfhdjqsfhjqsfjsdlfhjqfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjskfhdjskfhdkjsfhdjskfhdjsfdhjsks" > /dev/urandom

echo "erasing /mnt/test"
rm /mnt/test/*

echo "creating 10 small files"
for i in 0 1 2 3 4 5 6 7 8 9; do
dd if=/dev/urandom of=/mnt/test/f$i bs=4096 count=1;
done

echo "erasing uneven files to created fragmented areas"
for i in 1  3  5  7; do 
  rm /mnt/test/f$i
done

echo "creating fragmented file /mnt/test/frag"
dd if=/dev/urandom of=/mnt/test/frag bs=4096 count=10;
