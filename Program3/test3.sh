SERVER=$1
PORT=$2
LENGTH=32768
MESSAGES=2048
for TRY in 1 2 3 4 5 6 7 8 9 10
do
    echo "Try: $TRY, Running -l: $LENGTH, -n: $MESSAGES, without -D"
    ./ttcp -t -l$LENGTH -n$MESSAGES $SERVER -p$PORT
    echo
done