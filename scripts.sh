#!/bin/bash

dirName=${1-‘pwd ‘}
declare -A map
for  file in "$( find  $dirName  -type f )"; do #file="WorkerLog_1.txt,WorkerLog_2.txt...WornerLog_N.txt"
	for myfile in $file ;do # mfile[i] = WorkerLog_i.txt
		exec 6<&0
		exec < "$myfile"
		while read -r line; do
			command=($( awk -F '[ :]' '{print $10}' <<< "$line"))
			if [ "$command" == "search" ] ; then
				var=($( awk -F '[ :]' '{print $13}' <<< "$line"))
				docs=($( awk -F '[ :]' '{print $16}' <<< "$line"))
				mystr=${line##*:}
				if [ ! -z $docs ]; then
					VAR=( $mystr )
					numOfFiles=${#VAR[@]}
					
					if [ " ${map[$var]} " ]; then
						oldNum=${map[$var]}
						let newNum=$oldNum+$numOfFiles
						map[$var]=$newNum
					elif [[ ! " ${map[@]} " =~ " ${var} " ]]; then
						map[$var]+=$numOfFiles
					fi
					myarr+=($var)
				fi
			fi
		done
	done
done

  ############## KEY MOST FREQUENTLY FOUND ##################

  uniq=( $(printf '%s\n' "${myarr[@]}" | sort -u) )
	echo "total num of keywords searched: ${#uniq[@]}"

  ######################### MAX WORD ########################
	max=0

	for word in  ${!map[@]}; do 

		if [ "${map[$word]}" -gt "$max" ]; then
			max=${map[$word]}
			maxWord=$word
		fi
	done
	zero=0
	if [ "$max" -eq "$zero" ]; then 
		echo "keyword most frequently found: no results available"
	else
		echo "keyword most frequently found: $maxWord $max"
	fi
##########################  MIN WORD ########################
	if [ ! "$max" -eq "$zero" ]; then
		min=$max
		for word in  ${!map[@]}; do 
			if [ "${map[$word]}" -lt "$min" ]; then
				min=${map[$word]}
				minWord=$word
			fi
		done
		echo "keyword least frequently found: $minWord $min"
	else
		echo "keyword least frequently found: no results available"
	fi
	
	

	
