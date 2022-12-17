# Parallel-Text-Analysis-With-MPI
Parallelizes with MPI to compute character frequency in a text file

# Usage
compile: make

execute: mpiexec -n 5 ./text_analysis -l -s wonderland.txt

Where:
  -n (x) is the number of MPI nodes to be used in program execution
  - -l flag indicates the output to be a letter frequency histogram (otherwise the output will be a simple count of each letter frequency)
  - -s flag indicates the output to be displayed in stdout and a text file "out.txt"
  - fileName(wonderland.txt) is the text file you are trying to analyse
  
An example file "wonderland.txt" (the script from Alice in Wonderland)
