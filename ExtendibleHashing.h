#ifndef EXTENDIBLEHASHING_EXTENDIBLEHASHING_H
#define EXTENDIBLEHASHING_EXTENDIBLEHASHING_H

#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <bitset>

using namespace std;


template<class T>
class ExtendibleHashing {
    string fileName;
    static int globalDepth;
    static int maxBucketSize;

    bitset<globalDepth> (*hash)(T record); // Function pointer to hash function
    // Has to be a pointer to allow for different hash functions (we don't know how T looks like)

    struct Bucket {
        int localDepth;
        int size;
        T *records;

        Bucket() {
            this->localDepth = 0;
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
                    size--;
                    if (size == 0) {
                        delete this;
                    }
                    break;
                }
            }
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
            return size == maxBucketSize;
        }

        bool isEmpty() {
            return size == 0;
        }
    };

public:

    void saveBucket(Bucket *bucket, int index);
    Bucket *loadBucket(int index);

    void splitBucket(int index);

};


#endif //EXTENDIBLEHASHING_EXTENDIBLEHASHING_H
