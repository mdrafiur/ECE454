
#ifndef HASH_H
#define HASH_H

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include "list.h"

#define HASH_INDEX(_addr,_size_mask) (((_addr) >> 2) & (_size_mask))
#define RAND_NUM_UPPER_BOUND   100000

template<class Ele, class Keytype> class hash;

template<class Ele, class Keytype> class hash {
 private:
  unsigned my_size_log;
  unsigned my_size;
  unsigned my_size_mask;
  pthread_mutex_t* mutex; // declare a dynamic array of lock to be allocated later
  list<Ele,Keytype> *entries;
  list<Ele,Keytype> *get_list(unsigned the_idx);

 public:
  void setup(unsigned the_size_log=5);
  void setlock_list();
  void setlock_ele();
  void insert(Ele *e);
  Ele *lookup(Keytype the_key);
  void print(FILE *f=stdout);
  void reset();
  void cleanup();
  void lookup_and_insert_if_absent_list(Keytype the_key);
  void lookup_and_insert_if_absent_element(Keytype the_key);
  void combine(hash<Ele,Keytype>* h_thread, unsigned num_threads);
};

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::setup(unsigned the_size_log){
  my_size_log = the_size_log;
  my_size = 1 << my_size_log;
  my_size_mask = (1 << my_size_log) - 1;
  entries = new list<Ele,Keytype>[my_size];
}

// set a lock inside of hash class for each list
template<class Ele, class Keytype>
void
hash<Ele,Keytype>::setlock_list(){
	mutex = new pthread_mutex_t[my_size];
	int i;
	for (i = 0; i < my_size; i++) {
		pthread_mutex_init(&(mutex[i]), NULL);
	}
}

// set a lock inside of hash class for each possible element
template<class Ele, class Keytype>
void
hash<Ele,Keytype>::setlock_ele(){
	mutex = new pthread_mutex_t[RAND_NUM_UPPER_BOUND];
	int i;
	for (i = 0; i < RAND_NUM_UPPER_BOUND; i++) {
		pthread_mutex_init(&(mutex[i]), NULL);
	}
}

template<class Ele, class Keytype>
list<Ele,Keytype> *
hash<Ele,Keytype>::get_list(unsigned the_idx){
  if (the_idx >= my_size){
    fprintf(stderr,"hash<Ele,Keytype>::list() public idx out of range!\n");
    exit (1);
  }
  return &entries[the_idx];
}

template<class Ele, class Keytype> 
Ele *
hash<Ele,Keytype>::lookup(Keytype the_key){
  list<Ele,Keytype> *l;

  l = &entries[HASH_INDEX(the_key,my_size_mask)];
  return l->lookup(the_key);
}  

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::print(FILE *f){
  unsigned i;

  for (i=0;i<my_size;i++){
    entries[i].print(f);
  }
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::reset(){
  unsigned i;
  for (i=0;i<my_size;i++){
    entries[i].cleanup();
  }
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::cleanup(){
  unsigned i;
  reset();
  delete [] entries;
}

template<class Ele, class Keytype> 
void 
hash<Ele,Keytype>::insert(Ele *e){
  entries[HASH_INDEX(e->key(),my_size_mask)].push(e);
}

// custom function defined for randtrack_list_lock.cc
template<class Ele, class Keytype>
void
hash<Ele,Keytype>::lookup_and_insert_if_absent_list(Keytype the_key) {
	Ele *s;
	// grab the hash index as a variable
	unsigned index = HASH_INDEX(the_key,my_size_mask);
	//printf ("Locking List %d for total size of %d\n", index, my_size);
	list<Ele,Keytype> *l;
	l = &entries[index];
	// This is where we lock the list
	pthread_mutex_lock(&(mutex[index]));
	// if this sample has not been counted before
	if (!(s = l->lookup(the_key))){
		// insert a new element for it into the hash table
		s = new Ele(the_key);
		insert(s);
	}
	// increment the count for the sample
	s->count++;
	// unlock here after we are done
	pthread_mutex_unlock(&(mutex[index]));
	//printf ("Unlocking List %d for total size of %d\n", index, my_size);
}

// custom function defined for randtrack_list_lock.cc
template<class Ele, class Keytype>
void
hash<Ele,Keytype>::lookup_and_insert_if_absent_element(Keytype the_key) {
	Ele *s;
	//printf ("Locking List %d for total size of %d\n", index, my_size);
	list<Ele,Keytype> *l;
	l = &entries[HASH_INDEX(the_key,my_size_mask)];
	// This is where we lock the element (aka the key)
	pthread_mutex_lock(&(mutex[the_key]));
	// if this sample has not been counted before
	if (!(s = l->lookup(the_key))){
		// insert a new element for it into the hash table
		s = new Ele(the_key);
		insert(s);
	}
	// increment the count for the sample
	s->count++;
	// unlock here after we are done
	pthread_mutex_unlock(&(mutex[the_key]));
	//printf ("Unlocking List %d for total size of %d\n", index, my_size);
}

// custom function to combine combine the private thread table into one large table
template<class Ele, class Keytype>
void
hash<Ele,Keytype>::combine(hash<Ele,Keytype>* h_thread, unsigned num_threads) {
	//don't do anything if no threads
	if (num_threads == 1) return;
	int i, j;
	//go through every key until we hit upper bound, count appearance in all threads and combine
	for (i = 0; i < RAND_NUM_UPPER_BOUND; i++) {
		int count = 0;
		// get the count of every thread for that element, if any
		for (j = 0; j < num_threads; j++) {
			list<Ele,Keytype> *l = &(h_thread[j].entries[HASH_INDEX(i,my_size_mask)]);
			Ele *temp;
			if ((temp = l->lookup(i))) count += temp->count;
		}
		if (count > 0) {
			Ele* s = new Ele(i);
			insert(s);
			s->count = count;
		}
	}
}

#endif
