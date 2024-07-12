#!/bin/bash
processName='estimate-server'
env=$1
if [ ! -n "$env" ];then
  echo "need input environment value:[dev/test/prd]"
  exit 1
fi
export EST_ENV="$env"
git pull origin main
if [ "$env" == "dev" ];then
  export EST_DIR="$HOME"/CLionProjects/estimate-server
else
  export EST_DIR="$HOME"/estimate-server
fi
aws s3 sync s3://cher8-algo/rank/recommend/ model/
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_MAKE_PROGRAM=/usr/bin/make -DCMAKE_C_COMPILER=/usr/bin/gcc -DCMAKE_CXX_COMPILER=/usr/bin/g++ -DCMAKE_PREFIX_PATH="$HOME"/.local -G "CodeBlocks - Unix Makefiles" -S "$EST_DIR" -B build
cd build && make -o2
if [ $? -eq 0 ]
then
    echo "make compile success"
else
    echo "make compile fail"
    exit 1
fi
cd ..
PID=$(ps -ef|grep $processName|grep -v grep|awk '{printf $2}')
if [ $? -eq 0 ] && [ ${#PID} -gt 0 ]; then
    echo "process id : $PID"
    kill  ${PID}
    sleep 11
    if [ $? -eq 0 ]; then
        echo "kill $processName success"
    else
        echo "kill $processName fail"
        exit 1
    fi
else
    echo "process $processName not exist"
fi
./build/estimate-server &