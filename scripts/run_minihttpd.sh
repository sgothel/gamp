#!/bin/sh

/usr/sbin/mini_httpd -D -p 8080 -d `pwd` -l `pwd`/mini_httpd.log

