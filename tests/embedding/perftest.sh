#!/bin/bash

skipm="" #detectors to skip
m="PIPE ITS TPC"  #detectors to simulate
g="pythia8hi"       #generator for background
gs="pythia8"      #generator for signal

for nb in {10,100}; do
 
  mkdir -p $nb
  rm -rf $nb/*
  cd $nb

  mkdir -p background
  rm -rf background/*
  cd background

  #time to create the background events
  start=`date +%s`
  #time o2-sim -n $nb -m $m -g $g | tee -a log.txt
  time o2-sim -n $nb -g $g | tee -a log.txt
  end=`date +%s`
  timebg=`expr $end - $start`
  echo wall time for background event generation "$timebg" | tee -a log.txt

  cd ..

  for ns in {10,100,1000,10000}; do

    mkdir -p signal$ns
    rm -rf signal$ns/*
    cd signal$ns

    #time to create the signal events
    start=`date +%s`
    #time o2-sim -n $ns -m $m -g $gs --embedIntoFile ./../background/o2sim.root | tee -a log.txt
    time o2-sim -n $ns -g $gs --embedIntoFile ./../background/o2sim.root | tee -a log.txt
    end=`date +%s`
    times=`expr $end - $start`
    echo wall time for signal event generation "$times" | tee -a log.txt

    #time for digitization
    start=`date +%s`
    time o2-sim-digitizer-workflow -b --simFile ./../background/o2sim.root --simFileS o2sim.root -n $ns | tee -a log.txt
    end=`date +%s`
    timed=`expr $end - $start`
    echo wall time for digitization "$timed" | tee -a log.txt

    cd ..

    echo number of background events "$nb" | tee -a ../log.txt
    echo number of signal event "$ns" | tee -a ../log.txt
    echo wall time for background event generation "$timebg" | tee -a ../log.txt
    echo wall time for signal event generation "$times" | tee -a log.txt ../log.txt
    echo wall time for digitization "$timed" | tee -a ../log.txt
    echo | tee -a ../log.txt
 
  done
  cd ..
done



