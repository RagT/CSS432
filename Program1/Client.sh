#Client.sh
#script to run client socket program
PORTNUM=51020
REPETITIONS=20000
#Server ip passed as first argument to script
SERVERIP=$1

#loop through all 12 test cases
for nBuf in 15 30 60 100
do 
	bufSize=$((1500/$nBuf))
  echo ""
	for type in 1 2 3
	do
		echo "nBuf = $nBuf, bufSize = $bufSize, test type = $type"
		./Client $PORTNUM $REPETITIONS $nBuf $bufSize $SERVERIP $type
    #Wait to make sure server is ready
    sleep 1
	done
done	
	