# Parallel-Text-Analysis-With-MPI
Parallelizes with MPI to compute character frequency in a text file

compile: make
execute: mpiexec -n 5 ./A2 -l -s wonderland.txt

A few assumptions about command arguments:
- Assignment description said that the worker nodes should do the array computations - therefore, my program requires more than 1 node (-n 2..etc)
- My machine maxed out at 6 nodes
- For file sizes that are LESS than the number of worker nodes provided I outputted an error message to bring a larger file or decrease the number of nodes
- Does not support number of nodes greater than 100 (i don't even know if this is possible)

These assumptions are all implemented with command argument error checking.