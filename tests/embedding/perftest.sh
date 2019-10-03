#!/bin/bash

gb=pythia8hi #generator for background
gs=pythia8 #generator for signal
#m="PIPE TPC ITS" # modules to generate
#me="ZDC" # modules to exclude
#dry=true #only testing for debug

[ -n "$dry" ] && echo "dry test run. nothing is actually done"
echo

for nb in {1,10}; do	
    
    #
    # generate background events
    #
    echo starting background_$nb `date` | tee -a perftest.log.txt
    mkdir background_$nb
    cd    background_$nb
    cmd="{ time o2-sim -n$nb -g $gb "
    if [ -n "$m" ]
    then 
        cmd="$cmd -m $m "
    fi
    if [ -n "$me" ]
    then 
        cmd="$cmd --skipModules $me "
    fi
    cmd="$cmd ; } &> log.txt"
    echo "$cmd" | tee -a ./../perftest.log.txt
    start=`date +%s`
    [ -z "$dry" ] && eval $cmd
    end=`date +%s`
    cd ..
    timebg=`expr $end - $start`
    echo finished background_$nb `date` | tee -a perftest.log.txt
    echo walltime background_$nb "$timebg" | tee -a perftest.log.txt
    echo | tee -a perftest.log.txt

    for ns in {1,10,100,1000}; do

        #
	# generate signal events 
	#
        echo starting background_${nb}_signal_${ns} `date` | tee -a perftest.log.txt
        mkdir background_${nb}_signal_$ns
        cd background_${nb}_signal_$ns
        cmd="{ time o2-sim -n$ns -g $gs "
        if [ -n "$m" ]
        then
            cmd="$cmd -m $m "
        fi
        if [ -n "$me" ]
        then
            cmd="$cmd --skipModules $me "
        fi
        cmd="$cmd  --embedIntoFile ./../background_$nb/sim.root ; } &> log.txt"
        echo "$cmd" | tee -a ./../perftest.log.txt
        start=`date +%s`
        [ -z "$dry" ] && eval $cmd
        end=`date +%s`
        cd ..
        timebg=`expr $end - $start`
        echo finished background_${nb}_signal_${ns} `date` | tee -a perftest.log.txt
        echo walltime background_${nb}_signal_${ns} "$timebg" | tee -a perftest.log.txt
        echo | tee -a perftest.log.txt

	#
	# digitize signal+background events
	#
        echo starting digitization background_${nb}_signal_${ns} `date` | tee -a perftest.log.txt
        #mkdir background_${nb}_singal_$ns
        cd background_${nb}_signal_$ns
        cmd="{ time o2-sim-digitizer-workflow -n$ns --simFile ./../background_$nb/o2sim.root --simFileS o2sim.root ; } &> log.digit.txt"
        echo "$cmd" | tee -a ./../perftest.log.txt
        start=`date +%s`
        [ -z "$dry" ] && eval $cmd
        end=`date +%s`
        cd ..
        timebg=`expr $end - $start`
        echo finished digitization background_${nb}_signal_${ns} `date` | tee -a perftest.log.txt
        echo walltime digitization background_${nb}_signal_${ns} "$timebg" | tee -a perftest.log.txt
        echo | tee -a perftest.log.txt

    done 

done

echo
echo "everything finished." | tee -a perftest.log.txt

exit

