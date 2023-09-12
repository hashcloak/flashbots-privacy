#!/bin/bash
#
#  shaper.sh
#  ---------
#  A utility script for traffic shaping using tc
#
#  Usage
#  -----
#  shape.sh start - starts the shaper
#  shape.sh stop - stops the shaper
#  shape.sh restart - restarts the shaper
#  shape.sh show - shows the rules currently being shaped
#
#  tc uses the following units when passed as a parameter.
#    kbps: Kilobytes per second
#    mbps: Megabytes per second
#    kbit: Kilobits per second
#    mbit: Megabits per second
#    bps: Bytes per second
#  Amounts of data can be specified in:
#    kb or k: Kilobytes
#    mb or m: Megabytes
#    mbit: Megabits
#    kbit: Kilobits
#
#  AUTHORS
#  -------
#  Aaron Blankstein
#  Jeff Terrace
#  
#  MOFIFIED BY:
#  ------------
#  Hernan Vanegas
#  With colaboration of (https://gist.github.com/trongthanh/1196596/0cabc49e4e8a6165697225c81701c41dd31171f5)
#  
#  Original script written by: Scott Seong

# Rate to throttle to
RATE=$2

# Peak rate to allow
PEAKRATE=$2

IF=lo

# Average to delay packets by
LATENCY=$3


start() {
    tc qdisc add dev $IF root handle 1: htb default 12 
    tc class add dev $IF parent 1:1 classid 1:12 htb rate $RATE ceil $PEAKRATE 
    tc qdisc add dev $IF parent 1:12 netem delay $LATENCY
}

stop() {
    tc qdisc del dev $IF root
}

restart() {
    stop
    sleep 1
    start
}

show() {
    tc -s qdisc ls dev $IF
}

case "$1" in

start)

echo -n "Starting bandwidth shaping: "
start
echo "done"
;;

stop)

echo -n "Stopping bandwidth shaping: "
stop
echo "done"
;;

restart)

echo -n "Restarting bandwidth shaping: "
restart
echo "done"
;;

show)

echo "Bandwidth shaping status for $IF:"
show
echo ""
;;

*)

pwd=$(pwd)
echo "Usage: shaper.sh {start|stop|restart|show}"
;;

esac 
exit 0