#CC = g++-4.7
CC = g++
CFLAGS = -O3

all: randtrack randtrack_global_lock randtrack_tm randtrack_list_lock

randtrack: list.h hash.h defs.h randtrack.cc
	$(CC) $(CFLAGS) randtrack.cc -o randtrack

randtrack_global_lock: list.h hash.h defs.h randtrack_global_lock.cc
	$(CC) $(CFLAGS) randtrack_global_lock.cc -lpthread -o randtrack_global_lock
	
randtrack_tm: list.h hash.h defs.h randtrack_tm.cc
	$(CC) $(CFLAGS) randtrack_tm.cc -lpthread -fgnu-tm -o randtrack_tm

randtrack_list_lock: list.h hash.h defs.h randtrack_list_lock.cc
	$(CC) $(CFLAGS) randtrack_list_lock.cc -lpthread -o randtrack_list_lock

clean:
	rm -f *.o randtrack randtrack_global_lock randtrack_tm randtrack_list_lock
