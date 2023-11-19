// cInsertion.c
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#define MAX_COORDS 4096

// Global variables
double coords[MAX_COORDS][2]; // coordinates read from file
double distanceMatrix[MAX_COORDS][MAX_COORDS]; // calculated distance matrix
int numOfCoords; // number of coordinates read from file

// Function prototypes
void readCoordinates(const char* filename);
void calculateDistanceMatrix();
void cheapestInsertion(const char* outputFilename);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <coordinate_file_name> <output_file_name>\n", argv[0]);
        return 1;
    }
    
    const char* inputFilename = argv[1];
    const char* outputFilename = argv[2];

    readCoordinates(inputFilename);
    calculateDistanceMatrix();
    cheapestInsertion(outputFilename);
    
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


void cheapestInsertion(const char* outputFilename) {
    // Array to hold the tour
    int* tour = malloc(numOfCoords * sizeof(int));
    if (!tour) {
        perror("Memory allocation for tour failed");
        exit(EXIT_FAILURE);
    }

    // Boolean array to keep track of visited vertices
    int* visited = calloc(numOfCoords, sizeof(int));
    if (!visited) {
        free(tour);
        perror("Memory allocation for visited failed");
        exit(EXIT_FAILURE);
    }

    // Start with the first vertex in the tour
    tour[0] = 0; // Assuming the first vertex is at index 0
    visited[0] = 1; // Mark the first vertex as visited
    int tourSize = 1; // Current size of the tour
    double totalCost = 0.0; // Initialize total cost

    // We will use DBL_MAX from float.h to represent infinity
    while (tourSize < numOfCoords) {
        double minIncrease = DBL_MAX;
        int minIndex = -1;
        int positionToInsert = -1;

        for (int i = 0; i < numOfCoords; i++) {
            if (!visited[i]) {
                for (int j = 0; j < tourSize; j++) {
                    int current = tour[j];
                    int next = tour[(j + 1) % tourSize];
                    double increase = distanceMatrix[current][i] + distanceMatrix[i][next] - distanceMatrix[current][next];

                    if (increase < minIncrease) {
                        minIncrease = increase;
                        minIndex = i;
                        positionToInsert = (j + 1) % tourSize;
                    }
                }
            }
        }

        // Update total cost with the cost of inserting the new vertex
        totalCost += minIncrease;

        // Insert the vertex in the tour
        for (int i = tourSize; i > positionToInsert; i--) {
            tour[i] = tour[i - 1];
        }
        tour[positionToInsert] = minIndex;
        visited[minIndex] = 1;
        tourSize++;
    }

    // Add the cost of returning to the starting vertex
    totalCost += distanceMatrix[tour[tourSize - 1]][tour[0]];

    // Invert the tour for correct order
    int temp;
    for (int i = 0; i < tourSize / 2; i++) {
        temp = tour[i];
        tour[i] = tour[tourSize - i - 1];
        tour[tourSize - i - 1] = temp;
    }

    // Open the output file to write the total cost and the tour
    FILE* file = fopen(outputFilename, "w");
    if (file == NULL) {
        perror("Error opening output file");
        free(tour);
        free(visited);
        exit(EXIT_FAILURE);
    }

    // Write the count of elements in the tour on the first line
    fprintf(file, "%d\n", tourSize + 1);

    // Write the tour to the file starting and ending with the first vertex
    for (int i = 0; i < tourSize; i++) {
        fprintf(file, "%d ", tour[i]);
    }
    // Complete the loop by writing the starting point at the end
    fprintf(file, "%d\n", tour[0]);

    // Clean up
    fclose(file);
    free(tour);
    free(visited);
}