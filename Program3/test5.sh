SERVER=$1
PORT=$2
LENGTH=64
MESSAGES=1048576
for TRY in 1 2 3 4 5 6 7 8 9 10
do
    echo "Try: $TRY, Running -l: $LENGTH, -n: $MESSAGES, without -D"
    netstat -st | grep segments
    ./ttcp -t -l$LENGTH -n$MESSAGES $SERVER -p$PORT > /dev/null
    netstat -st | grep segments
    echo
done