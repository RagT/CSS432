SERVER=$1
PORT=$2
MESSAGES=1048576

for LENGTH in 64 128 256 512 1024 2048 4096 8192
do
    for TRY in 1 2 3 4 5 6 7 8 9 10
    do
        echo "Try: $TRY, Running -l: $LENGTH, -n: $MESSAGES, without -D"
        ./ttcp -t -l$LENGTH -n$MESSAGES $SERVER -p$PORT
        echo
    done

    MESSAGES=$(($MESSAGES / 2))
done
