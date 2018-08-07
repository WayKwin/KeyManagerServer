#!/bin/bash
USR=`whoami`
PID=`ps -u $USR | grep keymngserver | awk '{print $1}'`
if [ -z $pid ]
then
  echo "KeyMngServer haven't started"
  exit 1
fi
kill -10 $PID
