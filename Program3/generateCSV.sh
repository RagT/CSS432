cat test2.out | grep ttcp-rbyte > test2.txt
cat test2withD.out | grep ttcp-rbyte > test2withD.txt
cat test3.out | grep ttcp-rbyte > test3.txt
cat test4.out | grep ttcp-rbyte > test4.txt
cat test4withD.out | grep ttcp-rbyte > test4withD.txt
cat test5.out | grep ttcp-rbyte > test5.txt
cat test5withD.out | grep ttcp-rbyte > test5withD.txt


for file in *.txt; do
    sed -i -e 's/ttcp-rbytes=//g' "${file}" #replace "ttcp-rbyte=" with nothing
    sed -i -e 's/ time=/,/g' "${file}"      #replace " time=" with ","
    sed -i -e 's/ Mbps=/,/g' "${file}"      #replace " Mbps=" with ","
    sed -i -e 's/ I\/Ocalls=/,/g' "${file}" #replace " I/Ocalls=" with ","
    echo 'ttcp-rbytes,time,Mbps,I/O calls' | cat - "${file}" > temp && mv temp "${file}" #Add this line on top of the file
    mv "${file}" "${file/%.txt/.csv}"       #rename it to .csv
done


if [ "$(uname)" == "Darwin" ]; then # if on a mac, remove the files that end with "-e"
    rm *-e
fi