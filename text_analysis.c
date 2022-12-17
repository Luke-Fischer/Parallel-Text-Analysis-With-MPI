#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

char *charArr;
int arrCount;
const int MAX_INT = 500;
int comm_sz;

struct boundary {
   int startIndex;
   int numComputes;
};

void printNumCount(int letterFreq[26]){
    for(int i = 0; i < 26; i++){
        printf("%d\n",letterFreq[i]);
    }
}

void printHistogram(int letterFreq[26]){
    int scaling = 0;
    for(int i = 0; i < 26; i++){
        if(letterFreq[i] > 40){
            scaling = 1;
        }
    }
    if(scaling == 0){
        for(int i = 0; i < 26; i++){
            for(int j = 0; j < letterFreq[i]; j++){
                printf("%c", i + 97);
            }
            if(letterFreq[i] != 0){
                printf("\n");
            }
        }
    printf("\n");
    }
    else{
        //find max
        int max = 0;
        for(int i = 0; i < 26; i++){
            if(letterFreq[i] > max){
                max = letterFreq[i];
            }
        }
        int scaled = max / 40;
        for(int i = 0; i < 26; i++){
            int numOutputs = letterFreq[i] / scaled + 1;
            for(int j = 0; j < numOutputs; j++){
                printf("%c", i + 97);
                if (j == 39){
                    break;
                }
            }
            printf("\n");
        }
    }
}

int main(int argc, char *argv[]){
    FILE *fp;
    int letterFreq[26];
    int my_rank;
    int histogram = 0;
    int saveToFile = 0;
    arrCount = 0;

    //Start up MPI
    MPI_Init(&argc, &argv);

    //Get number of processes
    MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

    //Get my rank among all the processes
    MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

    
    //Initiaize array
    for(int i = 0; i < 26; i++){
        letterFreq[i] = 0;
    }

    if(my_rank == 0){
        if(argc == 3){
            if(strcmp(argv[1], "-l") == 0){
            histogram = 1;        
            }
        }
        if(argc == 3){
            if(strcmp(argv[1], "-s") == 0){
            saveToFile = 1;        
            }
        }
        if(argc == 4){
            if(strcmp(argv[1], "-s") == 0){
            saveToFile = 1;        
            }
            if(strcmp(argv[1], "-l") == 0){
            histogram = 1;        
            }
            if(strcmp(argv[2], "-l") == 0){
            histogram = 1;        
            }
            if(strcmp(argv[2], "-s") == 0){
            saveToFile = 1;        
            }
        }
    }
    fp = fopen(argv[argc - 1], "r");
    //checking if the file exist or not
    if (fp == NULL) {
        if(my_rank == 0){
            fprintf(stderr, "File Not Found!\n");
        }
        MPI_Finalize();
        exit(0);
    }

    if(comm_sz < 2){
        if(my_rank == 0){
            printf("Must provide at least one worker node!\n");
        }
        MPI_Finalize();
        exit(0);
    }
  
    // calculating the size of the file
    fseek(fp, 0L, SEEK_END);
    long int fileSize = ftell(fp);
    if(fileSize < comm_sz - 1){
        if(my_rank == 0){
            fprintf(stderr, "Number of characters in file is less than number of nodes provided!\n");
        }
        MPI_Finalize();
        exit(0);
    }

    //Point back to the beginning of the file
    rewind(fp);

    //Allocate memory to hold all the characters in the text file
    charArr = malloc((sizeof(char) * (fileSize)) + 1);
    int array[100];

    //worker nodes node
    if(my_rank != 0){
        //offset
        MPI_Recv(array, 100, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //array size
        MPI_Recv(&arrCount, 1, MPI_INT, 0, 1, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        //char array
        MPI_Recv(charArr, arrCount, MPI_CHAR, 0, 2, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        int chunkSize = 0;
        if(my_rank + 1 < comm_sz){
            chunkSize = array[my_rank + 1] - array[my_rank];
        }
        else{
            chunkSize = arrCount -1 - array[my_rank] + 1;
        }

        int alpha[26];
        for(int i = 0; i < 26; i++){
            alpha[i] = 0;
        }

        for(int i = array[my_rank]; i < array[my_rank] + chunkSize; i++){
        //Calculate character frequencies
            //An upper case Char
            if(charArr[i] >= 65 && charArr[i] <= 90){
                alpha[charArr[i] - 65]++;
            }
            //A lower case char
            else if (charArr[i] >= 97 && charArr[i] <= 122){
                alpha[charArr[i] - 97]++;
            }
        }

        //Send data back to control node
        MPI_Send(alpha, 26, MPI_INT, 0, 0, MPI_COMM_WORLD);

    }
    else if (my_rank == 0){  
        //Load data into character array
        while ((charArr[arrCount] = fgetc(fp)) != EOF){
            arrCount++;
        }
        int sectionMin = arrCount / (comm_sz - 1);
        int currIndex = 0;
        //Less characters in file than there are working nodes
        if(comm_sz - 1 > arrCount){
            for(int i = 0; i < 26; i++){
                if(charArr[i] >= 65 && charArr[i] <= 90){
                    letterFreq[charArr[i] - 65]++;
                }
                //A lower case char
                else if (charArr[i] >= 97 && charArr[i] <= 122){
                    letterFreq[charArr[i] - 97]++;
                }
            }
        }
        else{
            //Gather information from worker nodes
            for(int q = 1; q < comm_sz; q++){
                //Compute the boundary domain for each worker nodes
                array[q] = currIndex;
                if((q + 1) == comm_sz){
                    array[q+1] = arrCount - 1;
                }
                currIndex = currIndex + sectionMin;
            }
            for(int q = 1; q < comm_sz; q++){
                MPI_Send(array, 100, MPI_INT, q, 0, MPI_COMM_WORLD);
                MPI_Send(&arrCount, 1, MPI_INT, q, 1, MPI_COMM_WORLD);
                MPI_Send(charArr, arrCount, MPI_CHAR, q, 2, MPI_COMM_WORLD);
            }    
            for(int q = 1; q < comm_sz; q++){
                int alphaArr[26];
                MPI_Recv(alphaArr, 26, MPI_INT, q, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
                for(int i = 0; i < 26; i++){
                    letterFreq[i] = letterFreq[i] + alphaArr[i];
                }
            }
        }

        if(histogram == 1){
            printHistogram(letterFreq);
            if(saveToFile == 1){
                freopen("out.txt", "w", stdout);
                printHistogram(letterFreq);
            }
        }
        if(histogram == 0){
            printNumCount(letterFreq);
            if(saveToFile == 1){
                freopen("out.txt", "w", stdout);
                printNumCount(letterFreq);
            }
        }
    }

    MPI_Finalize();

    //Free data
    fclose(fp);
    free(charArr);

    return 0;
}