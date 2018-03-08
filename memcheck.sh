valgrind -q --leak-check=full --log-file=valgrind.out ./$1; cat valgrind.out;
