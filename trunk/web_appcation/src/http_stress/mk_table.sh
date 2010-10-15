#!/bin/sh

if test $# -ne 3; then
	echo "usage: number ip port"
	exit
fi

number=$1
ip=$2
port=$3

i=1
echo "$number" > ../conf/route.table
while test $i -le $number; do
	printf "$i\t$ip\t$port\n" >> ../conf/route.table
	i=`expr $i + 1`
done

