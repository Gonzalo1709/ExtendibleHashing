#ifndef EXTENDIBLEHASHING_EXTENDIBLEHASHING_H
#define EXTENDIBLEHASHING_EXTENDIBLEHASHING_H

#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <bitset>
#include <functional>
#include <unordered_map>

using namespace std;


template<class T>
class ExtendibleHashing {
    string fileName;
    static int globalDepth;
    static int maxBucketSize;
    int buckets = 0;
     // Function pointer to hash function
    // Has to be a pointer to allow for different hash functions (we don't know how T looks like)
    std::function<std::bitset<sizeof(int) * 8>(T)> hash;

    unordered_map<string, int> directory;

    //Constructor
public:
     ExtendibleHashing(string fileName, int globalDepthIn, int maxBucketSize, std::function<std::bitset<sizeof(int) * 8>(T)> hash){
        this->fileName = fileName;
        this->globalDepth = globalDepthIn;
        this->maxBucketSize = maxBucketSize;
        this->hash = hash;

        // Initialize first two buckets (0 and 1)
        auto *bucket0 = new Bucket();
        auto *bucket1 = new Bucket();
        saveBucket(bucket0, 0);
        saveBucket(bucket1, 1);
        buckets = 2;

        // Initialize directory
        directory["0"] = 0;
        directory["1"] = 1;
     }

    struct Bucket {
        int localDepth;
        int size;
        T *records;
        int next = -1;

        Bucket() {
            this->localDepth = 0;
            this->size = 0;
            this->records = new T[maxBucketSize];
        }

        Bucket(int localDepth) {
            this->localDepth = localDepth;
            this->size = 0;
            this->records = new T[maxBucketSize];
        }

        ~Bucket() {
            delete[] records;
        }

        void insert(T record) {
            records[size++] = record;
            // We don't have to check size here.
            // We'll do it in the insert function of ExtendibleHashing.
            // If it's full, we'll split the bucket.
        }

        void remove(T record) {
            for (int i = 0; i < size; i++) {
                if (records[i] == record) {
                    for (int j = i; j < size - 1; j++) {
                        records[j] = records[j + 1];
                    }
                    records[size - 1] = NULL;
                    size--;
                    if (size == 0) {
                        delete this;
                    }
                    break;
                }
            }
        }

        void removePos(int pos) {
            for (int i = pos; i < size - 1; i++) {
                records[i] = records[i + 1];
            }
            records[size - 1] = NULL;
            size--;
            if (size == 0) {
                delete this;
            }
        }

        T removeLast() {
            auto record = records[size - 1];
            records[size - 1] = NULL;
            size--;
            if (size == 0) {
                delete this;
            }
            return record;
        }

        bool contains(T record) {
            for (int i = 0; i < size; i++) {
                if (records[i] == record) {
                    return true;
                }
            }
            return false;
        }

        bool isFull() {
            return size >= maxBucketSize;
        }

        bool isEmpty() {
            return size == 0;
        }
    };

public:

    void saveBucket(Bucket *bucket, int index);
    Bucket *loadBucket(int index);

    void insert(T record);

    void printAllBucketsFromDir() {
        // Explore directory, load buckets and print them
        for (auto const &pair : directory) {
            auto *b = loadBucket(pair.second);
            cout << "Bucket " << pair.first << ": ";
            for (int j = 0; j < b->size; j++) {
                cout << b->records[j] << " ";
                while (b->next != -1) {
                    cout << "-> ";
                    b = loadBucket(b->next);
                    cout << b->records[j] << " ";
                }
            }
            cout << endl;
        }
    }

    void printAllBucketsFromMemory() {
        for (int i = 0; i < buckets; i++) {
            auto *b = loadBucket(i);
            cout << "Bucket " << i << ": ";
            for (int j = 0; j < b->size; j++) {
                cout << b->records[j] << " ";
            }
            cout << endl;
        }
    }

    void splitBucket(int depth, string key);
    void addOverflowBucket(int index);
};




#endif //EXTENDIBLEHASHING_EXTENDIBLEHASHING_H
