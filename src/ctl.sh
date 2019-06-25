#!/bin/bash

SERVER_PATH=$(pwd)
server=$SERVER_PATH/bin/server
client=$SERVER_PATH/bin/client
conf=$SERVER_PATH/conf/im.conf
echo "`date +%Y-%m-%d,%H:%m:%s`:server.conf 载入成功 ···" >> $SERVER_PATH/bin/log/sys.log

function usage() {
	printf "Usage: %s [-s(start) | -q(stop) | -r(restart)]\n" "$0"
}
	
function startServer() {
	pid=$(pidof $server)
	if [ $? -eq 0 ];then
        echo  "`date +%Y-%m-%d,%H:%m:%s`:server is already runing ···"
        echo  "`date +%Y-%m-%d,%H:%m:%s`:server is already runing ···" >> $SERVER_PATH/bin/log/sys.log
	else
		ip=$(awk -F: '/ip/{print $2}' $conf)
		port=$(awk -F: '/port/{print $2}' $conf)
		$server $port &
        echo "`date +%Y-%m-%d,%H:%m:%s`:server start success ···"
        echo "`date +%Y-%m-%d,%H:%m:%s`:server start success ···" >> $SERVER_PATH/bin/log/sys.log
	fi
}

function connect() {
	pid=$(pidof $client)
	if [ $? -eq 0 ];then
        echo  "`date +%Y-%m-%d,%H:%m:%s`:client is already connect ···"
        echo  "`date +%Y-%m-%d,%H:%m:%s`:client is already connect ···" >> $SERVER_PATH/bin/log/sys.log
	else
		ip=$(awk -F: '/ip/{print $2}' $conf)
		port=$(awk -F: '/port/{print $2}' $conf)
		$client $ip $port
        echo "`date +%Y-%m-%d,%H:%m:%s`:client start success ···"
        echo "`date +%Y-%m-%d,%H:%m:%s`:client start success ···" >> $SERVER_PATH/bin/log/sys.log
	fi
}

function stopServer() {
	pid=$(pidof $server)
	if [ $? -eq 0 ];then
		kill -9 $pid
        echo "`date +%Y-%m-%d,%H:%m:%s`:server stop success ···"
        echo "`date +%Y-%m-%d,%H:%m:%s`:server stop success ···" >> $SERVER_PATH/bin/log/sys.log
	else
        echo "`date +%Y-%m-%d,%H:%m:%s`:server is already stop ···"
        echo "`date +%Y-%m-%d,%H:%m:%s`:server is already stop ···" >> $SERVER_PATH/bin/log/sys.log
	fi
}

function restartServer() {
	stopServer
    echo "`date +%Y-%m-%d,%H:%m:%s`:server is already stop ···"
    echo "`date +%Y-%m-%d,%H:%m:%s`:server is already stop ···" >> $SERVER_PATH/bin/log/sys.log
	startServer
    echo "`date +%Y-%m-%d,%H:%m:%s`:server is already restart ···"
    echo "`date +%Y-%m-%d,%H:%m:%s`:server is already restart ···" >> $SERVER_PATH/bin/log/sys.log
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
esac
