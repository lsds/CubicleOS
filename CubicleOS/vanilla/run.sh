gcc main.c sqlite3.c -DSQLITE_OMIT_LOAD_EXTENSION -DSQLITE_THREADSAFE=0 -O2
./a.out --size 100 -mmap 0 --stats testing
