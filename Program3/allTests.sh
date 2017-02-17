SERVER=$1
PORT=$2
function pause(){
   read -p "$*"
}

pause 'Run server with the -D flag, then Press [Enter] key to continue...'

echo "Running Test 2D"
./test2withD.sh $SERVER $PORT &> test2withD.out
echo "Running Test 4D"
./test4withD.sh $SERVER $PORT &> test4withD.out 
echo "Running Test 5D"
./test5withD.sh $SERVER $PORT &> test5withD.out


pause 'Run server without the -D flag, then Press [Enter] key to continue...'

echo "Running Test 2"
./test2.sh $SERVER $PORT &> test2.out
echo "Running Test 4"
./test4.sh $SERVER $PORT &> test4.out
echo "Running Test 5"
./test5.sh $SERVER $PORT &> test5.out 


pause 'Run server without the -D flag, run tcpdump as well, then Press [Enter] key to continue...'

echo "Running Test 3"
./test3.sh $SERVER $PORT &> test3.out