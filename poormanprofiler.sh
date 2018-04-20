#!/bin/bash
nsamples=100
sleeptime=0

echo Profiler starting
rm poormanprofiler/*

for x in $(seq 1 $nsamples)
do
	echo Sample ${x}
	sudo gdb -ex "thread apply all bt" --batch -p $(pidof CAM) >> poormanprofiler/gdbo
	sleep $sleeptime
done

echo Seperating
awk 'BEGIN{a="a"}/^Thread [0-9]* \(Thread /{a = $2}/\#[0-9]* /{print >> "poormanprofiler/thread"a}' poormanprofiler/gdbo
rm poormanprofiler/threada

echo Sorting
for x in $(ls poormanprofiler/thread*)
do
	awk '{$1=""}1' "$x" | awk '{$1=$1}1' | sort | uniq -c | sort -r -n -k 1,1 > "$x.done"
done

