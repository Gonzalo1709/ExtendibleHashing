#include "ExtendibleHashing.h"
#include <iostream>
#include <fstream>
#include <bitset>
#include <string>

using namespace std;

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

    int currentDepth = 0;


    while( (directory.find(currentSearch.to_string().substr((sizeof(int) * 8) - (currentDepth + 1), currentDepth + 1 )) == directory.end()) && (currentDepth < globalDepth - 1) ) {
        currentDepth++;
        currentSearch = (hashValue & bitset<32>((1 << (currentDepth + 1)) - 1));
    }

    int index = directory[currentSearch.to_string().substr((sizeof(int) * 8) - (currentDepth + 1), currentDepth + 1)];

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
        splitBucket(currentDepth, currentSearch.to_string().substr((sizeof(int) * 8) - (currentDepth + 1), currentDepth + 1));
    }
}

template<typename T>
void ExtendibleHashing<T>::splitBucket(int depth, string key) {
    int bucketIndex = directory[key];
    auto *bucket = loadBucket(bucketIndex);

    if (bucket->localDepth >= globalDepth) {
        addOverflowBucket(bucketIndex);
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

    directory.erase(key);

    string newKey1 = "0" + key;
    string newKey2 = "1" + key;

    bucket->localDepth++;
    newBucket->localDepth = bucket->localDepth;

    saveBucket(bucket, bucketIndex);
    saveBucket(newBucket, buckets);

    directory[newKey1] = bucketIndex;
    directory[newKey2] = buckets++;

    if (bucket->isFull()) {
        splitBucket(depth + 1, newKey1);
    }
    if (newBucket->isFull()) {
        splitBucket(depth + 1, newKey1);
    }
}

template<typename T>
void ExtendibleHashing<T>::addOverflowBucket(int index) {
    auto bucket = loadBucket(index);
    auto newBucket = new Bucket(bucket->localDepth);

    newBucket->insert(bucket->removeLast());

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
    ExtendibleHashing<int> eh("data.dat", 3, 3, CustomHash);
    for (int i = 0; i < 10; i++) {
        cout << "Inserting " << i << endl;
        eh.insert(i);
        cout << "Buckets in directory:" << endl;
        eh.printAllBucketsFromDir();

        cout << "Buckets in memory:" << endl;
        eh.printAllBucketsFromMemory();

        cout << endl;
    }
}