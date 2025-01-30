#include <ctime>
#include <iostream>
#include <vector>

struct Inode {
    std::string name;
    std::string fileData;
    bool isDirectory = false;
    int fileSize = 1; // assume every file, even empty, is at least 1 byte in size
    std::time_t creationTime; // include the ctime library to use this type
    std::time_t modTime;
    Inode *parentNode;
    std::vector<Inode *> childNodes;
};

struct HashNode // hash node datastructure
{
    std::string key;
    Inode *iptr;
    HashNode *next;
};

enum HashTableType {
    LINEAR_PROBING,
    QUADRATIC_PROBING,
    CHAINING
};

//*** You can add helper functions if you desire ***//

class FileSim {
  private:
    Inode *root;
    Inode *currentDir;
    int diskSize;
    int currentUsedDiskMemory;

    int makeNode(std::string name, bool isDirectory);
    Inode* pathToNode(std::string path);
    std::string nodeToPath(Inode *node);

    // Hash Table type
    HashTableType hashTableType;
    // Hash Table # of buckets (linked lists)
    int tableSize;
    // Number of elements in the hash table
    int numElements;
    // Threshold for resizing
    float loadFactor = 0.75; 
    // Track the number of collisions
    int numCollisions; 
    // Pointer to an array containing buckets
    HashNode **table;
    // Create a new hash record
    HashNode *createNode(std::string key, Inode *_iptr, HashNode *next);
    // Hash table - insert record
    bool insertItemLinear(std::string _key, Inode *_iptr);
    bool insertItemQuadratic(std::string _key, Inode *_iptr);
    bool insertItemChaining(std::string _key, Inode *_iptr);
    // hash function to map values to key
    unsigned int hashFunction(std::string _key);
    // Hash search
    HashNode *searchItem(std::string key);
    // For handling resizing
    void resizeHashTable(); 

  public:
    // FileSim Features; see writeup for specifics
    FileSim(int _diskSize);
    ~FileSim();

    void setHashTableType(HashTableType type) {
        hashTableType = type;
    }

    int touch(std::string fileName);
    int mkdir(std::string dirName);
    void ls();
    void pwd();
    void tree();

    int cat(std::string path);
    int stat(std::string path);
    int edit(std::string path, std::string newValue);
    int cd(std::string dirPath);
    int rm(std::string path, bool recursive);
    int mv(std::string originPath, std::string destPath);
    void search(std::string key);
    void resetCollisions();
    int getCollisions() const;
    bool insertItem(std::string _key, Inode *_iptr) {
        if (hashTableType == LINEAR_PROBING) {
            return insertItemLinear(_key, _iptr);
        } else if (hashTableType == QUADRATIC_PROBING) {
            return insertItemQuadratic(_key, _iptr);
        } else if (hashTableType == CHAINING) {
            return insertItemChaining(_key, _iptr);
        }
        return false;
    }
    
};
