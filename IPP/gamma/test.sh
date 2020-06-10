#!/bin/bash

if [ $# -ne 2 ]
then
  echo "Podano złą liczbę argumentów"
  exit 1
fi

exec=$1
tests=$2

for file in ${tests}/*.in; do
   echo $file:
  ./$exec < $file > res.out 2> res.err

  diff res.out ${file%in}out &> /dev/null
  out=$?
  diff res.err ${file%in}err &> /dev/null
  err=$?

  valgrind --leak-check=full ./$exec < $file > /dev/null 2> mem.err

  memerr=$(grep "LEAK" mem.err)

  if  [ $out -ne 0 ] || [ $err -ne 0 ]
  then
    echo "WYJSCIE ZLE"
  else
    echo "WYJSCIE OK"
  fi

  if [ "$memerr" != "" ]
  then
    echo "PROBLEM Z PAMIĘCIĄ"
  fi

  echo
done

rm res.out res.err mem.err
