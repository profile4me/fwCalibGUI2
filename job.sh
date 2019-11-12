#!/bin/bash

app1=$1
submitDir=$2
offset=$3
app2=$4
treesDir=$5

. /cvmfs/hades.gsi.de/install/5.34.34/hydra2-5.2/defall.sh
line=$(($offset+$SLURM_ARRAY_TASK_ID))
hld=`cat ${submitDir}/task.list | sed -n "${line}p" | awk '{print $1}'`
srcID=`cat ${submitDir}/task.list | sed -n "${line}p" | awk '{print $2}'`
dst=`basename "$hld" | sed "s;\(.*\).hld;$submitDir\/\1.root;"`
comm1="${app1} ${hld} ${submitDir}"
comm2="${app2} ${dst} ${treesDir} ${srcID}" 

echo "comm1: $comm1"
[[ ! -f $dst ]] && command $comm1 || echo "dst file already exists"
echo "comm2: $comm2"
[[ ! -f ${treesDir}/{srcID}.root ]] && command $comm2 || echo "tree already filled"


