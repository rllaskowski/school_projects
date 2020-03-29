make

test=tests/_test_0_2

./dcl $(cat ${test}.key) < $test.a > res.out

diff res.out $test.b

# rm res.out

echo ''