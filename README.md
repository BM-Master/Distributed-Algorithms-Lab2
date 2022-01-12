# Distributed Algorithms - Readers and Writers problem
This code is only to guide you, it is strictly forbidden to use as a solution for your own work

## Background

The goal here is simulate the access to a resource (database or something) by two entities, readers and writers. 
K concurrent readers can access to read the resource but only if there is no writer yet.
Only one writer at time can access to write the resource, just one no more.

The time for read its 1/4 of the time for write.

User access to the resource X times, and each time they decide what action to take (read or write).

The total of users is a natural number N.

## How to compile? 📦

gcc readers-writers.c -lpthreads

or with a custom name for the output file
gcc readers-writers.c -o "FileName" -lpthreads

# Execution

./out N K writeTime X

And you are done.

## Author ✒️

* **Bastián Martínez - Computer Science Student** - *Code* 

## License 📄

This file is under GNU General Public License v3.0.

## Contact!

* For any comment, bastian.ig.mar@gmail.com :) 📢
