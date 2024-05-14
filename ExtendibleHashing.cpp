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

template<typename T>
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

template<typename T>
void ExtendibleHashing<T>::splitBucket(int index) {
    auto *bucket = loadBucket(index);
    auto *newBucket = new Bucket();

}

bitset<32> CustomHash(int record) { 
        return bitset<32>(record % 3); 
    }

template<typename T>
int ExtendibleHashing<T>::maxBucketSize = 0; // Initialize with a default value


int main(){
     //hash function (mod 3
    ExtendibleHashing<int> eh("data.dat", 3, CustomHash);
    ExtendibleHashing<int>::Bucket b;
    b.insert(5); //inserting 5 into the bucket
    b.insert(6);
    b.insert(7);

    eh.saveBucket(&b, 0); //saving the bucket to the file
    auto *b2 = eh.loadBucket(0); //loading the bucket from the file
    cout << b2->records[0] << endl; //printing the first record of the bucket
    cout << b2->records[1] << endl;
}