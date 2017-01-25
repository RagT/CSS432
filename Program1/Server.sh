#Server.sh
#script to run Server socket program
PORTNUM=51020
REPETITIONS=20000

#loop through all 12 test cases
for nBuf in 15 30 60 100
do 
	bufSize=$((1500/$nBuf))
	for test_type in 1 2 3
	do
    echo ""
		echo "Waiting for nBuf = $nBuf, bufSize = $bufSize, test type = $test_type"
		./Server $PORTNUM $REPETITIONS
	done
done
		
