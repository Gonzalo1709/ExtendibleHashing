//
// Created by USUARIO on 5/11/2024.
//

#include "ExtendibleHashing.h"

void ExtendibleHashing::saveBucket(ExtendibleHashing::Bucket *bucket, int index) {
    fstream file(this->fileName, ios::in | ios::out | ios::binary);
    file.seekp(index * sizeof(Bucket));
    file.write((char *) bucket, sizeof(Bucket));
    file.close();
}

ExtendibleHashing::Bucket *ExtendibleHashing::loadBucket(int index) {
fstream file(this->fileName, ios::in | ios::binary);
    auto *bucket = new Bucket();
    file.seekg(index * sizeof(Bucket));
    file.read((char *) bucket, sizeof(Bucket));
    file.close();
    return bucket;
}

void ExtendibleHashing::splitBucket(int index) {
    auto *bucket = loadBucket(index);
    auto *newBucket = new Bucket();

}
