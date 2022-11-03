@echo off

echo -------
echo -------

set wildcard=*.h *.cpp *.inl *.c

echo TODOS FOUND:
findstr  -n -i -l /d:"./" "TODO" %wildcard% 

echo -------
echo -------
