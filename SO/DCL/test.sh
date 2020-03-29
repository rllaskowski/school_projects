make

echo 

for test in tests/*.a
do
    echo ${test%.a}

    ./dcl $(cat ${test%a}key) < $test > res.out

    if diff res.out ${test%a}b > /dev/null
    then
        echo "OK!"
    else
        echo "BLAD!"
        exit 1
    fi
done
rm res.out