#include <gtest/gtest.h>

#include "test_helpers.hpp"
#include "../code_1/fileSim.hpp"

#include <fstream>
#include <iostream>
#include <string>
#include <cmath>
#include <array>
#include <vector>

using namespace std;

void testCollisionsForHashTableType(FileSim &fileSim, HashTableType tableType) {
    fileSim.setHashTableType(tableType);
    fileSim.resetCollisions();

    const int numDirs = 100;
    const string dirPrefix = "dir_";
    const string filePrefix = "file_";

    for (int i = 0; i < numDirs; i++){
        string dirName = dirPrefix + to_string(i);
        string fileName = filePrefix + to_string(i);

        Inode* dirNode = new Inode();
        dirNode->name = dirName;
        dirNode->isDirectory = true;

        Inode* fileNode = new Inode();
        fileNode->name = fileName;
        fileNode->isDirectory = false;
        fileNode->parentNode = dirNode;

        fileSim.insertItem(dirName, dirNode);
        fileSim.insertItem(fileName, fileNode);
    }

    cout << "Collisions with " << (tableType == LINEAR_PROBING ? "Linear Probing" : (tableType == QUADRATIC_PROBING ? "Quadratic Probing" : "Chaining")) << ": " << fileSim.getCollisions() << endl;
}

class FileSimCollisionTest : public ::testing::Test {
protected:
    FileSim fileSim;

    FileSimCollisionTest() : fileSim(1000) {}  // Initialize with a disk size of 1000
};

// Test for linear probing collisions
TEST_F(FileSimCollisionTest, LinearProbingCollisions) {
    std::cout << "Running test for Linear Probing..." << std::endl;
    testCollisionsForHashTableType(fileSim, LINEAR_PROBING);
}

// Test for quadratic probing collisions
TEST_F(FileSimCollisionTest, QuadraticProbingCollisions) {
    std::cout << "Running test for Quadratic Probing..." << std::endl;
    testCollisionsForHashTableType(fileSim, QUADRATIC_PROBING);
}

// Test for chaining collisions
TEST_F(FileSimCollisionTest, ChainingCollisions) {
    std::cout << "Running test for Chaining..." << std::endl;
    testCollisionsForHashTableType(fileSim, CHAINING);
}
