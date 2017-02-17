SERVER=$1
PORT=$2
BYTES=67108864

for LENGTH in 1458 1459 1460 1461 1462
do
    MESSAGES=$(($BYTES / $LENGTH))

    for TRY in 1 2 3 4 5 6 7 8 9 10
    do
        echo "Try: $TRY, Running -l: $LENGTH, -n: $MESSAGES, with -D"
        ./ttcp -t -l$LENGTH -n$MESSAGES $SERVER -p$PORT -D
        echo
    done
done