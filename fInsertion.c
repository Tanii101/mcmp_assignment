#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <float.h>

#define MAX_COORDS 4096 // Maximum number of coordinates

// Global variables
double coords[MAX_COORDS][2]; //  coordinates read from file
double distanceMatrix[MAX_COORDS][MAX_COORDS]; //  calculated distance matrix
int numOfCoords; // umber of coordinates read from file

// Function prototypes
void readCoordinates(const char* filename);
void calculateDistanceMatrix();
void farthestInsertion(const char* outputFilename);

int main(int argc, char* argv[]) {
    if (argc != 3) {
        printf("Usage: %s <coordinate_file_name> <output_file_name>\n", argv[0]);
        return 1;
    }

    const char* inputFilename = argv[1];
    const char* outputFilename = argv[2];

    readCoordinates(inputFilename);
    calculateDistanceMatrix();
    farthestInsertion(outputFilename);

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


void farthestInsertion(const char* outputFilename) {
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

    // Initialize the tour starting with vertex 0
    tour[0] = 0;
    visited[0] = 1;
    int tourSize = 1;

    // Find the farthest vertex from vertex 0 to start the tour
    double maxDist = 0;
    int farthest = -1;
    for (int i = 1; i < numOfCoords; i++) {
        double dist = distanceMatrix[0][i];
        if (dist > maxDist) {
            maxDist = dist;
            farthest = i;
        }
    }
    tour[1] = farthest;
    visited[farthest] = 1;
    tourSize++;

    // Continue with the farthest insertion heuristic
    while (tourSize < numOfCoords) {
        maxDist = 0;
        farthest = -1;

        // Find the farthest unvisited vertex from the current tour
        for (int i = 0; i < numOfCoords; i++) {
            if (!visited[i]) {
                for (int j = 0; j < tourSize; j++) {
                    double dist = distanceMatrix[i][tour[j]];
                    if (dist > maxDist) {
                        maxDist = dist;
                        farthest = i;
                    }
                }
            }
        }

        // Find the best position to insert the farthest vertex
        double minIncrease = DBL_MAX;
        int positionToInsert = 0;
        for (int i = 0; i < tourSize; i++) {
            int next = (i + 1) % tourSize;
            double increase = distanceMatrix[tour[i]][farthest] + distanceMatrix[farthest][tour[next]] - distanceMatrix[tour[i]][tour[next]];
            if (increase < minIncrease) {
                minIncrease = increase;
                positionToInsert = i + 1;
            }
        }

        // Insert the farthest vertex into the tour
        for (int i = tourSize; i > positionToInsert; i--) {
            tour[i] = tour[i - 1];
        }
        tour[positionToInsert] = farthest;
        visited[farthest] = 1;
        tourSize++;
    }

    // Open the output file to write the tour
    FILE* file = fopen(outputFilename, "w");
    if (file == NULL) {
        perror("Error opening output file");
        free(tour);
        free(visited);
        exit(EXIT_FAILURE);
    }

    // Write the count of elements in the tour on the first line, including the return to the start
    fprintf(file, "%d\n", tourSize + 1);

    // Write the tour to the output file, ensuring it starts and ends with 0
    for (int i = 0; i < tourSize; i++) {
        fprintf(file, "%d ", tour[i]);
    }
    // Ensure the tour ends with the starting point (0)
    fprintf(file, "%d\n", tour[0]);

    fclose(file);
    free(tour);
    free(visited);
}
