//
// Created by USUARIO on 5/11/2024.
//

#include "ExtendibleHashing.h"
#include <iostream>
#include <fstream>
#include <cstdio>
#include <string>
#include <bitset>
#include <functional>

template<class T>
void ExtendibleHashing<T>::saveBucket(typename ExtendibleHashing<T>::Bucket *bucket, int index) {
    //if the file does not exist, create it
    fstream file(this->fileName, ios::in | ios::out | ios::binary);
    if (!file) {
        file.open(this->fileName, ios::out | ios::binary);
        file.close();
        file.open(this->fileName, ios::in | ios::out | ios::binary);
    }

    file.seekp(index * sizeof(Bucket));
    file.write((char *) bucket, sizeof(Bucket));
    file.close();
}


template<typename T>
typename ExtendibleHashing<T>::Bucket *ExtendibleHashing<T>::loadBucket(int index) {
    fstream file(this->fileName, ios::in | ios::binary);
    auto *bucket = new Bucket();
    file.seekg(index * sizeof(Bucket));
    file.read((char *) bucket, sizeof(Bucket));
    file.close();
    return bucket;
}

template<class T>
void ExtendibleHashing<T>::insert(T record) {
    bitset<sizeof(int) * 8> hashValue = hash(record);
    bitset<sizeof(int) * 8> currentSearch = hashValue & bitset<32>(1);

    int currentDepth = currentSearch.test(sizeof(int) * 8 - 1) ? depth1 : depth0;

    currentSearch = hashValue & bitset<32>((1 << currentDepth) - 1);

    int index = directory[currentDepth][currentSearch];
    auto bucket = loadBucket(index);

    if (bucket->next != -1) {
        while (bucket->next != -1) {
            index = bucket->next;
            bucket = loadBucket(bucket->next);
        }
    }

    bucket->insert(record);
    saveBucket(bucket, index);
    if (bucket->isFull()) {
        splitBucket(currentDepth, currentSearch, currentSearch.test(sizeof(int) * 8 - 1));
    }
}

template<typename T>
void ExtendibleHashing<T>::splitBucket(int depth, bitset<sizeof(int) * 8> key, bool change1) {
    int bucketIndex = directory[depth][key];
    auto *bucket = loadBucket(bucketIndex);

    if (bucket->localDepth == globalDepth) {
        addOverflowBucket(bucket, bucketIndex);
        return;
    }

    auto *newBucket = new Bucket();

    T* temp = new T[maxBucketSize];
    for (int i = 0; i < bucket->size; i++) {
        temp[i] = bucket->records[i];
    }

    int totalRecords = bucket->size;
    bucket->size = 0;

    for (int i = 0; i < totalRecords; i++) {
        if (hash(temp[i])[(sizeof(int) * 8) - (depth-1)] == 0) {
            bucket->insert(temp[i]);
        } else {
            newBucket->insert(temp[i]);
        }
    }

    directory[depth].erase(key);

    bitset<sizeof(int) * 8> newKey1 = key;
    bitset<sizeof(int) * 8> newKey2(key.to_ulong() + (bitset<32>(1 << depth).to_ulong()));

    bucket->localDepth++;
    newBucket->localDepth = bucket->localDepth;

    saveBucket(bucket, bucketIndex);
    saveBucket(newBucket, buckets);

    directory[depth].erase(key);

    directory[depth + 1][newKey1] = bucketIndex;
    directory[depth + 1][newKey2] = buckets++;
    if (change1) {
        depth1 += 1;
    } else {
        depth0 += 1;
    }

    if (bucket->isFull()) {
        splitBucket(depth + 1, newKey1, change1);
    }
    if (newBucket->isFull()) {
        splitBucket(depth + 1, newKey1, change1);
    }
}

template<typename T>
void ExtendibleHashing<T>::addOverflowBucket(ExtendibleHashing::Bucket *bucket, int index) {
    auto newBucket = new Bucket(bucket->localDepth);

    newBucket->insert(bucket->records[bucket->size - 1]);
    bucket->removeLast();

    bucket->next = buckets;
    saveBucket(bucket, index);
    saveBucket(newBucket, buckets++);
}

bitset<sizeof(int) * 8> CustomHash(int record) {
        return bitset<sizeof(int) * 8>(record % 3);
    }

template<typename T>
int ExtendibleHashing<T>::maxBucketSize = 3; // Initialize with a default value

template<typename T>
int ExtendibleHashing<T>::globalDepth = 3;


int main(){
     //hash function (mod 3
    ExtendibleHashing<int> eh("data.dat", 3, 3, CustomHash);
    eh.insert(1);
    eh.insert(2);
    eh.insert(3);
    eh.insert(4);
    eh.insert(5);
    eh.insert(1);
    eh.insert(2);
    eh.insert(3);
    eh.insert(4);
    eh.insert(5);
    cout << "Printing all buckets from directory" << endl;
    eh.printAllBucketsFromDir();
    cout << "Printing all buckets from memory" << endl;
    eh.printAllBucketsFromMemory();
}