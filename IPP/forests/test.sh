#!/bin/bash

if [ "$#" -ne 2 ]
then
    echo "Niepoprawna ilość parametrów!"
    exit 1
fi

if ! [ -x "$1" ]
then
    echo "podaj poprawną ścieżkę do pliku wykonywalnego"
    exit 1
fi

if ! [ -d "$2" ]
then
    echo "podaj poprawną ścieżkę folderu z testami"
    exit 1
fi


for f in $2/*.in
do
    echo $f:
    valgrind --error-exitcode=15 --leak-check=full --show-leak-kinds=all --track-origins=yes --errors-for-leak-kinds=all --log-file="result.val" ./$1 < $f > result.out 2> result.err
    
    if ! grep -q "LEAK" result.val && diff ${f%in}out result.out > /dev/null && diff ${f%in}err result.err > /dev/null
    then
        echo "OK!"
        
    else
        echo "BŁĄD!"
    fi


done

rm -f result.out result.err result.val