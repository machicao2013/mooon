#!/bin/sh

if test $# -ne 3; then
	echo "usage: number ip port"
	exit
fi

number=$1
ip=$2
port=$3

i=1
echo "$number" > node.table
while test $i -le $number; do
	echo "$i\t$ip\t$port" >> node.table
	i=`expr $i + 1`
done

