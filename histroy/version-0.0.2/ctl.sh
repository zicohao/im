#!/bin/bash

IM_PATH=$(pwd)
server=$IM_PATH/bin/server
client=$IM_PATH/bin/client
conf=$IM_PATH/conf/im.conf
echo "`date +%Y-%m-%d,%H:%m:%s`:server.conf 载入成功 ···" >> $IM_PATH/log/sys.log

function usage() {
	printf "Usage: %s [-s(start) | -q(stop) | -r(restart)]\n" "$0"
}
	
function startServer() {
	pid=$(pidof $server)
	if [ $? -eq 0 ];then
        echo  "`date +%Y-%m-%d,%H:%m:%s`:server is already runing ···"
        echo  "`date +%Y-%m-%d,%H:%m:%s`:server is already runing ···" >> $IM_PATH/log/sys.log
	else
		ip=$(awk -F: '/ip/{print $2}' $conf)
		port=$(awk -F: '/port/{print $2}' $conf)
		$server $ip $port &
        echo "`date +%Y-%m-%d,%H:%m:%s`:server start success ···"
        echo "`date +%Y-%m-%d,%H:%m:%s`:server start success ···" >> $IM_PATH/log/sys.log
	fi
}

function UNconnect() {
    pid=$(pidof $client)
    if [$? -eq 0 ];then
        kill -9 $pid
        echo "`date +%Y-%m-%d,%H:%m:%s`:client stop success ···"
        echo "`date +%Y-%m-%d,%H:%m:%s`:client stop success ···" >> $IM_PATH/log/sys.log
    else
        echo "`date +%Y-%m-%d,%H:%m:%s`:client is already stop ···"
        echo "`date +%Y-%m-%d,%H:%m:%s`:client is already stop ···" >> $IM_PATH/log/sys.log
    fi
}

function connect() {
	pid=$(pidof $client)
	if [ $? -eq 0 ];then
        echo  "`date +%Y-%m-%d,%H:%m:%s`:client is already runing ···"
        echo  "`date +%Y-%m-%d,%H:%m:%s`:client is already runing ···" >> $IM_PATH/log/sys.log
	else
		ip=$(awk -F: '/ip/{print $2}' $conf)
		port=$(awk -F: '/port/{print $2}' $conf)
		$client $ip $port
        echo "`date +%Y-%m-%d,%H:%m:%s`:client connect success ···"
        echo "`date +%Y-%m-%d,%H:%m:%s`:client connect success ···" >> $IM_PATH/log/sys.log
	fi
}

function stopServer() {
	pid=$(pidof $server)
	if [ $? -eq 0 ];then
		kill -9 $pid
        echo "`date +%Y-%m-%d,%H:%m:%s`:server stop success ···"
        echo "`date +%Y-%m-%d,%H:%m:%s`:server stop success ···" >> $IM_PATH/log/sys.log
	else
        echo "`date +%Y-%m-%d,%H:%m:%s`:server is already stop ···"
        echo "`date +%Y-%m-%d,%H:%m:%s`:server is already stop ···" >> $IM_PATH/log/sys.log
	fi
}

function restartServer() {
	stopServer
    echo "`date +%Y-%m-%d,%H:%m:%s`:server is already stop ···"
    echo "`date +%Y-%m-%d,%H:%m:%s`:server is already stop ···" >> $IM_PATH/log/sys.log
	startServer
    echo "`date +%Y-%m-%d,%H:%m:%s`:server is already restart ···"
    echo "`date +%Y-%m-%d,%H:%m:%s`:server is already restart ···" >> $IM_PATH/log/sys.log
}


if [ $# -ne 1 ] ; then
	usage
fi 

case $1 in 
	-s | 'start' )
	startServer
	;;
	-q | 'stop' )
	stopServer
	;;
	-r | 'restart')
	restartServer
	;;
	-c | 'connect')
	connect
	;;
	-uc | 'UNconnect')
	UNconnect
	;;
esac
