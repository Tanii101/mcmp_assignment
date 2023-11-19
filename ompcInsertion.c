// ompcInsertion.c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <omp.h>

#define MAX_COORDS 4096 // Adjust as needed

// Global variables
double coords[MAX_COORDS][2]; // Stores the coordinates read from file
double distanceMatrix[MAX_COORDS][MAX_COORDS]; // Stores the calculated distance matrix
int numOfCoords; // Stores the number of coordinates read from file
int* tour; // Array to store the tour
int* visited; // Array to track visited vertices
int tourSize; // Current size of the tour

// Function prototypes
void readCoordinates(const char* filename);
void calculateDistanceMatrix();
void parallelCheapestInsertion(const char* outputFilename);
void initializeTour(); // Declare the function
void finalizeTour(); // Declare the function

int main(int argc, char* argv[]) {
    printf("Program started.\n");
    if (argc != 3) {
        printf("Usage: %s <coordinate_file_name> <output_file_name>\n", argv[0]);
        return 1;
    }

    const char* inputFilename = argv[1];
    const char* outputFilename = argv[2];

    readCoordinates(inputFilename);
    calculateDistanceMatrix();
    initializeTour();
    parallelCheapestInsertion(outputFilename);
    finalizeTour();
    
    return 0;
}

// Function to read coordinates from file and populate the coords array
void readCoordinates(const char* filename) {
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    numOfCoords = 0;
    while (fscanf(file, "%lf,%lf", &coords[numOfCoords][0], &coords[numOfCoords][1]) == 2) {
        numOfCoords++;
    }

    fclose(file);
}

// Function to calculate the Euclidean distance between two points
double euclideanDistance(double x1, double y1, double x2, double y2) {
    return sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
}

// Function to generate the distance matrix from the coordinates
void calculateDistanceMatrix() {
    for (int i = 0; i < numOfCoords; i++) {
        for (int j = 0; j < numOfCoords; j++) {
            if (i == j) {
                distanceMatrix[i][j] = 0.0;
            } else {
                distanceMatrix[i][j] = euclideanDistance(coords[i][0], coords[i][1], coords[j][0], coords[j][1]);
            }
        }
    }
}

void initializeTour() {
    // Allocate memory for the tour and visited arrays
    tour = malloc(numOfCoords * sizeof(int));
    visited = calloc(numOfCoords, sizeof(int)); // Calloc initializes the array to 0

    if (!tour || !visited) {
        fprintf(stderr, "Memory allocation failed.\n");
        exit(EXIT_FAILURE);
    }

    // Starting vertex
    tour[0] = 0;
    visited[0] = 1; // Mark the first vertex as visited
    tourSize = 1; // Start the tour with one vertex
}

void finalizeTour() {
    // Free the allocated memory for tour and visited arrays
    free(tour);
    free(visited);
}

void parallelCheapestInsertion(const char* outputFilename) {
    // Rest of the tour setup code remains the same

    // Complete the tour
    while (tourSize < numOfCoords) {
        double minIncrease = DBL_MAX;
        int minIndex = -1;
        int positionToInsert = -1;

        // Parallelize the search for the cheapest vertex to insert
        #pragma omp parallel
        {
            double localMinIncrease = DBL_MAX;
            int localMinIndex = -1;
            int localPositionToInsert = -1;

            #pragma omp for nowait
            for (int i = 1; i < numOfCoords; ++i) { // Start from 1 since 0 is already in the tour
                if (!visited[i]) {
                    for (int j = 0; j < tourSize; ++j) {
                        int current = tour[j];
                        int next = tour[(j + 1) % tourSize];
                        double increase = distanceMatrix[current][i] + distanceMatrix[i][next] - distanceMatrix[current][next];
                        if (increase < localMinIncrease) {
                            localMinIncrease = increase;
                            localMinIndex = i;
                            localPositionToInsert = (j + 1) % tourSize;
                        }
                    }
                }
            }

            // Critical section to update the global minimum
            #pragma omp critical
            {
                if (localMinIncrease < minIncrease) {
                    minIncrease = localMinIncrease;
                    minIndex = localMinIndex;
                    positionToInsert = localPositionToInsert;
                }
            }
        }

        // Insert the found vertex into the tour
        for (int i = tourSize; i > positionToInsert; --i) {
            tour[i] = tour[i - 1];
        }
        tour[positionToInsert] = minIndex;
        visited[minIndex] = 1;
        tourSize++;
    }

    // Attempt to write to file
    FILE* file = fopen(outputFilename, "w");
    if (file == NULL) {
        perror("Error opening output file");
        exit(EXIT_FAILURE);
    }

    // Write the count of elements in the tour on the first line, including the return to the start
    fprintf(file, "%d\n", tourSize + 1);

    // Write the final tour to the output file, ensuring it starts and ends with 0
    for (int i = 0; i < tourSize; ++i) {
        fprintf(file, "%d ", tour[i]);
    }
    // Ensure the tour ends with the starting point (0)
    fprintf(file, "%d\n", tour[0]);

    fclose(file);
}
