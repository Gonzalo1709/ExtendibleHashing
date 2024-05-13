//
// Created by USUARIO on 5/11/2024.
//

#include "ExtendibleHashing.h"
#include <fstream>
#include <cstdio>

template<typename T>
void ExtendibleHashing<T>::saveBucket(typename ExtendibleHashing<T>::Bucket *bucket, int index) {
    fstream file(this->fileName, ios::in | ios::out | ios::binary);
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
