make

# In pix.test there is 8320 = 1040*8 hex digits of PI in rows of 64

./pix 1040 > pix.res 2> /dev/null

if diff pix.res pix.test > /dev/null 
then
    echo "OK"
else 
    echo "ERROR"
fi

rm pix.res