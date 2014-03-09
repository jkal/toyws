#!bin/sh
httperf --server localhost --port 9090 --uri /index.html --num-conn 5000 --num-call 10 --rate 200 --timeout 5