#!/bin/bash

#	analysisDST starts as: ./analysisDST $HLD_DIR/be1906201071308.hld ${submitDir}
#	genTree starts as: ./genTree $DST_DIR/be1906201071308.root ${treesDir} 12
#		where 12 - srcId



resources="--mem=150 --time=00:05:00"
submitDir="/lustre/nyx/hades/user/dborisen/dst_cache"
treesDir="${submitDir}/trees"
logDir="${submitDir}/log"
app1="analysisDST"
app2="genTree"

rsync task.list ${submitDir}
rsync ${app1} ${submitDir}
rsync ${app2} ${submitDir}

# parse task.list
IFS=$'\r\n' GLOBIGNORE='*' command eval  'strArr=($(< task.list))'
i=0
while [[ $i -lt ${#strArr[@]} ]]
do
	[[ ! -z `echo ${strArr[$i]} | sed -n '/#[0-9]\{3\}/p'` ]] && {
		offset=$(($i+2))
		j=$(($i+1))
		while [[ -z "`echo ${strArr[$j]} | sed -n '/#[0-9]\{3\}/p'`" && $j -lt ${#strArr[@]} ]]
		do
			# printf "%s\n" `echo ${strArr[$j]} | sed -n '/#[0-9]\{3\}/p'`
			# printf "\t%d\n" $j
			j=$(($j+1))
		done
		comm="sbatch --array=0-$(($j-$i-2)) ${resources} -D ${submitDir} --output=${logDir}/slurm-%A-%a.out job.sh ${submitDir}/${app1} ${submitDir} ${offset} ${submitDir}/${app2} ${treesDir}" 
		# echo $comm 
		command $comm
	}
	i=$j
done
