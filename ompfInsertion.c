// ompfInsertion.c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>
#include <omp.h>

#define MAX_COORDS 4096

// Global variables
double coords[MAX_COORDS][2];
double distanceMatrix[MAX_COORDS][MAX_COORDS];
int numOfCoords;
int* tour;
int* visited;
int tourSize;

// Function prototypes
void readCoordinates(const char* filename);
void calculateDistanceMatrix();
void parallelFarthestInsertion(const char* outputFilename);
void initializeTour();
void cleanupTour();

int main(int argc, char* argv[]) {
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

void parallelFarthestInsertion(const char* outputFilename) {
    // Initialize the tour with two farthest vertices
    double maxDist = 0.0;
    int farthest_i = 0;
    int farthest_j = 0;

    // Find two farthest points to start the tour
    for (int i = 0; i < numOfCoords; ++i) {
        for (int j = i + 1; j < numOfCoords; ++j) {
            double dist = distanceMatrix[i][j];
            if (dist > maxDist) {
                maxDist = dist;
                farthest_i = i;
                farthest_j = j;
            }
        }
    }

    // Initialize tour with two farthest points
    tour[0] = farthest_i;
    tour[1] = farthest_j;
    visited[farthest_i] = 1;
    visited[farthest_j] = 1;
    tourSize = 2;

    // Complete the tour
    while (tourSize < numOfCoords) {
        maxDist = 0.0;
        int farthestVertex = -1;

        // Parallelize the search for the farthest unvisited vertex
        #pragma omp parallel
        {
            double localMaxDist = 0.0;
            int localFarthestVertex = -1;

            #pragma omp for nowait
            for (int i = 0; i < numOfCoords; ++i) {
                if (!visited[i]) {
                    double distToTour = 0.0;
                    // Find distance from vertex i to the nearest vertex in the tour
                    for (int j = 0; j < tourSize; ++j) {
                        double dist = distanceMatrix[i][tour[j]];
                        if (dist > distToTour) {
                            distToTour = dist;
                        }
                    }
                    if (distToTour > localMaxDist) {
                        localMaxDist = distToTour;
                        localFarthestVertex = i;
                    }
                }
            }

            // Critical section to update the global maximum distance
            #pragma omp critical
            {
                if (localMaxDist > maxDist) {
                    maxDist = localMaxDist;
                    farthestVertex = localFarthestVertex;
                }
            }
        }

        // Find the best position to insert the farthest vertex
        double minIncrease = DBL_MAX;
        int insertPosition = -1;
        for (int i = 0; i < tourSize; ++i) {
            int next = (i + 1) % tourSize;
            double increase = distanceMatrix[tour[i]][farthestVertex] + distanceMatrix[farthestVertex][tour[next]] - distanceMatrix[tour[i]][tour[next]];
            if (increase < minIncrease) {
                minIncrease = increase;
                insertPosition = next;
            }
        }

        // Insert the farthest vertex into the tour
        for (int i = tourSize; i > insertPosition; --i) {
            tour[i] = tour[i - 1];
        }
        tour[insertPosition] = farthestVertex;
        visited[farthestVertex] = 1;
        tourSize++;
    }

    // Write the tour to the output file
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
