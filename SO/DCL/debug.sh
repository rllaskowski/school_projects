test=tests/_test_0_1

make debug

gdb -q --args ./dcl < $(cat ${test}.key)

