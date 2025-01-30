#include <cstddef>
#include <fstream>
#include <iostream>
#include <sstream>
#include <time.h>
#include <vector>
#include <algorithm>
using namespace std;

#include <filesystem>
namespace fs = std::filesystem;

#include "fileSim.hpp"
#include <vector>



// ===========Lifecycle methods===========

FileSim::FileSim(int _diskSize) : numCollisions(0) {
    diskSize = _diskSize;

    root = new Inode;
    root->parentNode = nullptr;
    root->name = "(root)";
    root->creationTime = time(NULL);
    root->modTime = root->creationTime;
    root->isDirectory = true;
    currentDir = root;
    currentUsedDiskMemory = 0;

    // Add Hash Table intialization
    tableSize = 5;
    table = new HashNode*[tableSize];
    for (int i = 0; i < tableSize; ++i) {
        table[i] = nullptr;
    }
    hashTableType = LINEAR_PROBING;
}

FileSim::~FileSim() {
    delete[] table;
}



// ===========Helper methods===========

int FileSim::makeNode(std::string name, bool isDirectory) {
    if (currentUsedDiskMemory + 1 > diskSize) {
        cout << "No space left on disk!" << endl;
        return -1;
    }

    // if same name exists
    for (Inode* child : currentDir->childNodes) {
        if (child->name == name) {
            cout << "Duplicate name: " << name << endl;
            return -1;
        }
    }

    // create new file or directory
    Inode* newNode = new Inode;
    newNode->name = name;
    newNode->isDirectory = isDirectory;
    newNode->parentNode = currentDir;
    newNode->creationTime = time(NULL);
    newNode->modTime = newNode->creationTime;
    newNode->fileSize = 1;

    currentUsedDiskMemory += 1;

    currentDir->childNodes.push_back(newNode);

    // if (!isDirectory) {
    //     insertItem(name, newNode);
    // }
    insertItem(name, newNode);
    
    return 0;
}

Inode* getChild(Inode* parent, std::string name) {
    for (Inode* child : parent->childNodes) {
        if (child->name == name) {
            return child;
        }
    }
    return nullptr;
}

Inode* FileSim::pathToNode(std::string path) {
    // Returns the node pointed to by `path`.
    //
    // `path` may be...
    //    1. The special path '..', which returns the parent of the current
    //       directory, or the current directory if there is no parent.
    //    2. An absolute path in the format /dir1/dir2/...
    //       where / is the root and /dir1 is a child directory of the root.
    //    3. A relative path (a file in the current working directory)

    // 1
    if (path == "..") {
        return currentDir->parentNode;
    }

    // 2 & 3
    Inode *node;
    if (path.front() == '/') {
        node = root; // 2
        path = path.substr(1);
        
    } else {
        node = currentDir; // 3
    }

    stringstream pathStream(path);
    string pathSegment;
    // std::vector<std::string> seglist;

    while (getline(pathStream, pathSegment, '/')) {
        if (Inode* child = getChild(node, pathSegment)) {
            node = child;
        } else {
            return nullptr;
        }
    }
    return node;
}

std::string FileSim::nodeToPath(Inode *node) {
    // Returns an absolute path given a node
    if (!node->parentNode) {
        // Root
        return "/";
    }

    string path = "";
    while (node->parentNode) {
        path.insert(0, "/" + node->name);
        node = node->parentNode;
    }
    return path;
}

int recursiveDelete(Inode *node) {
    if (node->isDirectory) {
        for (Inode* child : node->childNodes) {
            recursiveDelete(child);
        }
    }
    delete node;
    return 0;
}

void treeHelper(Inode* node, int level) {
    if (level == 0) {
        cout << " " << node->name << endl;
    } else {
        cout << "- " << node->name << endl;
    }

    if (node->isDirectory) {
        for (Inode* child : node->childNodes) {
            treeHelper(child, level + 1);
        }
    }
}

std::string timeToString(time_t t) {
    char buffer[26];
    struct tm* tm_info = localtime(&t);
    strftime(buffer, 26, "%Y-%m-%d %H:%M:%S", tm_info);
    return std::string(buffer);
}

// ===========Public API===========

int FileSim::touch(std::string fileName) {
    return makeNode(fileName, false);
}

int FileSim::mkdir(std::string dirName) {
    return makeNode(dirName, true);
}

void FileSim::ls() {
    for (Inode* child : currentDir->childNodes) {
        cout << child->name << endl;
    }
}

void FileSim::pwd() {
    cout << nodeToPath(currentDir) << endl;
}

void FileSim::tree() {
    if (currentDir) {
        treeHelper(currentDir, 0);
    }
}

int FileSim::cat(std::string path) {
    Inode* file = pathToNode(path);
    if (!file) {
        cout << "cat: " << path << ": No such file or directory" << endl;
        return -1;
    }
    if (file->isDirectory) {
        cout << "cat: " << path << ": Is a directory" << endl;
        return -1;
    }
    cout << file->fileData << endl;
    return 0;
}

int FileSim::stat(std::string path) {
    Inode* file = pathToNode(path);
    if (!file) {
        cout << "Error: File not found." << endl;
        return -1;
    }

    cout << "Name: " << file->name << endl;
    cout << "Size: " << file->fileSize << " bytes" << endl;
    cout << "Type: " << (file->isDirectory ? "Directory" : "File") << endl;
    cout << "Created: " << timeToString(file->creationTime) << endl;
    cout << "Last Modified: " << timeToString(file->modTime) << endl;
    return 0;
}

int FileSim::edit(std::string path, std::string newValue) {
    Inode* file = pathToNode(path);
    if (!file) {
        cout << "edit: " << path << ": No such file or directory" << endl;
        return -1;
    }
    if (file->isDirectory) {
        cout << "edit: " << path << ": Is a directory" << endl;
        return -1;
    }

    // check if editing will exceed disk space
    int newSize = newValue.length() + 1;
    if (currentUsedDiskMemory + newSize - file->fileSize > diskSize) {
        cout << "edit: No more space left on disk" << endl;
        return -1;
    }

    currentUsedDiskMemory += (newSize - file->fileSize);
    file->fileData = newValue;
    file->fileSize = newSize;
    file->modTime = time(NULL);
    return 0;
 
}

int FileSim::cd(std::string dirPath) {
    Inode* dir = pathToNode(dirPath);
    if (!dir) {
        cout << "cd: no such file or directory: " << dirPath << endl;
        return -1;
    }
    if (!dir->isDirectory) {
        cout << "cd: not a directory: " << dirPath << endl;
        return -1;
    }
    currentDir = dir;
    return 0;
}

int FileSim::rm(std::string path, bool recursive) {
    if (path == "." || path == "..") {
        cout << "rm: '.' and '..' may not be removed" << endl;
        return -1;
    }
    Inode* node = pathToNode(path);
    if (!node) {
        cout << "rm: " << path << ": No such file or directory" << endl;
        return -1;
    }

    if (node->isDirectory && !recursive) {
        cout << "rm: " << path << ": is a directory" << endl;
        return -1;
    }

    if (node->isDirectory) {
        for (Inode* child : node->childNodes) {
            recursiveDelete(child);
        }
    }

    // remove from hash table
    int index = hashFunction(node->name);
    HashNode* prev = nullptr;
    HashNode* curr = table[index];
    while (curr) {
        if (curr->key == node->name){
            if (prev) {
                prev->next = curr->next;
            } else {
                table[index] = curr->next;
            }
            delete curr;
            break;
        }
        prev = curr;
        curr = curr->next;
    }

    // remove the node from the parent children list
    auto& children = node->parentNode->childNodes;
    auto it = find(children.begin(), children.end(), node);
    if(it != children.end()) {
        children.erase(it);
    } 
    delete node;
    return 0;
    
}

int FileSim::mv(std::string originPath, std::string destPath) {
    Inode* originNode = pathToNode(originPath);
    if (!originNode) {
        cout << "Moving from: " << originPath << " to " << destPath << endl;
        cout << "mv: " << originPath << ": No such file or directory" << endl;
        return -1;
    }

    Inode* destNode = pathToNode(destPath);
    
    // Case 1: Renaming a file 
    if (!destNode) {
        // Check if destination is a valid new name (same directory)
        string newName = destPath;
        size_t lastSlash = destPath.find_last_of('/');
        Inode* targetParent = originNode->parentNode;

        if (lastSlash != string::npos) {
            string parentPath = destPath.substr(0, lastSlash);
            newName = destPath.substr(lastSlash + 1);
            targetParent = pathToNode(parentPath);
            if (!targetParent || !targetParent->isDirectory) {
                cout << "Moving from: " << originPath << " to " << destPath << endl;
                cout << "mv: " << parentPath << ": No such directory" << endl;
                return -1;
            }
        }

        // Check for duplicate names in the target directory
        for (Inode* child : targetParent->childNodes) {
            if (child->name == newName) {
                cout << "Moving from: " << originPath << " to " << destPath << endl;
                cout << "mv: " << newName << " Already exists!" << endl;
                return -1;
            }
        }

        cout << "Moving from: " << originPath << " to " << destPath << endl;

        string oldName = originNode->name;
        originNode->name = newName;

        if (originNode->parentNode != targetParent) {
            auto& oldChildren = originNode->parentNode->childNodes;
            oldChildren.erase(remove(oldChildren.begin(), oldChildren.end(), originNode), oldChildren.end());
            targetParent->childNodes.push_back(originNode);
            originNode->parentNode = targetParent;
        }

        // Update the hash table
        unsigned int index = hashFunction(oldName);
        HashNode* prev = nullptr;
        HashNode* curr = table[index];
        while (curr) {
            if (curr->key == oldName) {
                if (prev) {
                    prev->next = curr->next;
                } else {
                    table[index] = curr->next;
                }
                delete curr;
                break;
            }
            prev = curr;
            curr = curr->next;
        }
        insertItem(destPath, originNode);
        
        // cout << "Moving from " << oldName << " to " << destPath << endl;
        return 0;
    }

    // Case 2: Moving the file to a new directory
    if (destNode->isDirectory) {
        // Check if the file is not already in the destination directory
        for (Inode* child : destNode->childNodes) {
            if (child->name == originNode->name) {
                cout << "Moving from: " << originPath << " to " << destPath << endl;
                cout << "mv: " << originNode->name << " Already exists!" << endl;
                return -1;
            }
        }
        cout << "Moving from: " << originPath << " to " << destPath << endl;

        auto& oldChildren = originNode->parentNode->childNodes;
        oldChildren.erase(remove(oldChildren.begin(), oldChildren.end(), originNode), oldChildren.end());

        originNode->parentNode = destNode;
        destNode->childNodes.push_back(originNode);       

        // cout << "mv: " << originPath << " to " << destPath << endl;
        return 0;
    }

    cout << "Moving from: " << originPath << " to " << destPath << endl;
    cout << "mv: invalid move operation" << endl;
    return 0;
}

void FileSim::search(std::string key) {
    bool found = false;
    for (int i = 0; i < tableSize; i++) {
        HashNode* node = table[i];
        while (node != nullptr) {
            if (node->key == key) {
                cout << "Found: " << nodeToPath(node->iptr) << endl;
                found = true;
            }
            node = node->next;
        }
    }
    if (!found) {
        cout << "No files found with name: " << key << endl;
    }
}

// ===========Hash Table===========

HashNode *FileSim::createNode(std::string _key, Inode *_iptr, HashNode *_next) {
    HashNode* newNode = new HashNode;
    newNode->key = _key;
    newNode->iptr = _iptr;
    newNode->next = _next;
    return newNode;
}

// Hash Function
unsigned int FileSim::hashFunction(string s) {
    unsigned int hash = 0;
    for (char c : s) {
        hash += c;
    }
    return hash % tableSize;
}

// Search Method
HashNode *FileSim::searchItem(string key) {
    unsigned int index = hashFunction(key);
    HashNode* node = table[index];
    while (node) {
        if (node->key == key) {
            return node;
        }
        node = node->next;
    }
    return nullptr; // Key not found
}

// bool FileSim::insertItem(std::string _key, Inode* _iptr) {
//     unsigned int index = hashFunction(_key);
//     HashNode* existingNode = searchItem(_key);
//     if (existingNode) {
//         cout << "Warning: File already exists in hash table." << endl;
//         return false;
//     }
//     table[index] = createNode(_key, _iptr, table[index]);
//     return true;
// }

// Insert new record
bool FileSim::insertItemLinear(std::string _key, Inode *_iptr) {
    unsigned int index = hashFunction(_key);
    unsigned int startIdx = index;

    // HashNode* node = table[index];
    // while (node) {
    //     if (node->key == _key) {
    //         return false;
    //     }
    //     index = (index + 1) % tableSize;
    //     node = table[index];
    // }

    while(table[index] != nullptr) {
        if (table[index]->key == _key) {
            table[index] = createNode(_key, _iptr, table[index]);
            numCollisions++;
            return true;
        }
        index = (index + 1) % tableSize;
        if (index == startIdx) {
            return false;
        }
    }

    table[index] = createNode(_key, _iptr, table[index]);
    // numElements++;
    return true;
}

bool FileSim::insertItemQuadratic(std::string _key, Inode *_iptr) {
    unsigned int index = hashFunction(_key);
    unsigned int startIndex = index;
    int probe = 1;

    // HashNode* node = table[index];
    // while (node) {
    //     if (node->key == _key) {
    //         return false;
    //     }
    //     index = (index + i * i) % tableSize;
    //     i++;
    //     node = table[index];
    // }

    while(table[index] != nullptr) {
        if (table[index]->key == _key) {
            return false;
        }
        numCollisions++;
        index = (startIndex + probe * probe) % tableSize;
        probe++;
        if (index == startIndex) {
            return false;
        }
    }

    table[index] = createNode(_key, _iptr, table[index]);
    numElements++;
    return true;
}

bool FileSim::insertItemChaining(std::string _key, Inode *_iptr) {
    unsigned int index = hashFunction(_key);
    // HashNode* node = createNode(_key, _iptr, table[index]);
    // table[index] = node;
    // return true;

    HashNode *currentNode = table[index];

    while (currentNode != nullptr) {
        if (currentNode->key == _key) {
            return false;
        }
        currentNode = currentNode->next;
    }
    if (table[index] != nullptr) {
        numCollisions++;
    }
    table[index] = createNode(_key, _iptr, table[index]);
    numElements++;
    return true;
}


//Resize the table
void FileSim::resizeHashTable() {
    int newTableSize = tableSize * 2;
    HashNode** newTable = new HashNode*[newTableSize];
    for (int i = 0; i < newTableSize; i++) {
        newTable[i] = nullptr;
    }

    // rehash all the old items
    for (int i = 0; i < tableSize; i++) {
        HashNode* current = table[i];
        while (current != nullptr) {
            unsigned int newHash = hashFunction(current->key) % newTableSize;
            HashNode* newNode = new HashNode{current->key, current->iptr = nullptr};

            if (newTable[newHash] == nullptr) {
                newTable[newHash] = newNode;
            } else {
                HashNode* temp = newTable[newHash];
                while (temp->next != nullptr) {
                    temp = temp->next;
                }
                temp->next = newNode;
            }
            current = current->next;
        }
    }
    delete[] table;
    table = newTable;
    tableSize = newTableSize;
}

void FileSim::resetCollisions() {
    numCollisions = 0;
}

int FileSim::getCollisions() const {
    return numCollisions;
}