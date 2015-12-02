#!/bin/bash
deviceId="300032001147343339383037"
accessTkn="da837cbd013221af2cac61eea03e15d8459c49ea"
funcName="makeMove"
cmd="NONE"
if [ "$1" == "forward" ] 
then
  cmd="forward"
  addDistance="Y"
fi

if [ "$1" == "backward" ]
then
  cmd="backward"
  addDistance="Y"
fi

if [ "$1" == "stop" ]
then
  cmd="stop"
fi

if [ "$1" == "turnLeft" ]
then
  cmd="turnLeft"
  addDistance="Y"
fi

if [ "$1" == "turnRight" ]
then
  cmd="turnRight"
  addDistance="Y"
fi

if [ "$1" == "setSpeed" ]
then
  cmd="setSpeed,$2"
fi

if [ "$1" == "calibrateTurning" ]
then
  cmd="calibrateTurning,$2"
fi

if [ "$addDistance" == "Y" ] && [ -n "$2" ] && [ -n "$3" ]
then
  if [ "$3" == "m" ] || [ "$3" == "mm" ] || [ "$3" == "cm" ] || [ "$3" == "degrees" ]
  then
    cmd+=",$2,$3"
  else
    echo "Must be \"mm\", \"cm\", \"m\" or \"degrees\"."
  fi
fi

if [ "$cmd" != "NONE" ]
then
  curl https://api.particle.io/v1/devices/$deviceId/$funcName -d arg=$cmd -d access_token=$accessTkn
fi

