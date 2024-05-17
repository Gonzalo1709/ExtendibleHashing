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

    if (bucket->localDepth >= globalDepth - 1) {
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
    if (bucket->size < 1) {
        return;
    }
    auto newBucket = new Bucket(bucket->localDepth);

    newBucket->insert(bucket->removeLast());

    bucket->next = buckets;

    saveBucket(bucket, index);
    saveBucket(newBucket, buckets++);
}

template<class T>
void ExtendibleHashing<T>::deleteItem(T record) {
    bitset<sizeof(int) * 8> hashValue = hash(record);
    bitset<sizeof(int) * 8> currentSearch = hashValue & bitset<32>(1);

    int currentDepth = 0;

    while( (directory.find(currentSearch.to_string().substr((sizeof(int) * 8) - (currentDepth + 1), currentDepth + 1 )) == directory.end()) && (currentDepth < globalDepth - 1) ) {
        currentDepth++;
        currentSearch = (hashValue & bitset<32>((1 << (currentDepth + 1)) - 1));
    }

    int index = directory[currentSearch.to_string().substr((sizeof(int) * 8) - (currentDepth + 1), currentDepth + 1)];

    auto bucket = loadBucket(index);

    bool deleted = false;
    if (bucket->contains(record)) {
        bucket->remove(record);
        saveBucket(bucket, index);
        deleted = true;
    }
    int previousIndex = index;
    if (!deleted) {
        if (bucket->next != -1) {
            while (bucket->next != -1 && !bucket->contains(record)) {
                previousIndex = index;
                index = bucket->next;
                bucket = loadBucket(bucket->next);
            }
        }
        if (!bucket->contains(record)) {
            return;
        }
        bucket->remove(record);
        saveBucket(bucket, index);
    }
    if (bucket->isEmpty()) {
        cout << "Bucket is empty" << endl;
        if (previousIndex != index) {
            auto previousBucket = loadBucket(previousIndex);
            previousBucket->next = bucket->next;
            saveBucket(previousBucket, previousIndex);
        }
        else {
            bool done = false;
            // Check if sibling bucket (same key up to its most significant bit (ie 101 and 001 are siblings)) has records
            while (!done && currentDepth > 0) {
                string key = currentSearch.to_string().substr((sizeof(int) * 8) - (currentDepth + 1), currentDepth + 1);
                string siblingKey = key;
                siblingKey[0] = (siblingKey[0] == '0') ? '1' : '0';
                int siblingIndex = directory[siblingKey];
                auto siblingBucket = loadBucket(siblingIndex);

                // if both buckets are empty, we can delete them and make a parent key (one with less depth (ie. 01 is a parent for 001 and 101)) an entry in the directory
                // we can reuse the lowest index of one of the siblings (to avoid creating a new bucket in memory)
                int lowestIndex = (index < siblingIndex) ? index : siblingIndex;
                if (siblingBucket->isEmpty()) {
                    directory.erase(key);
                    directory.erase(siblingKey);
                    directory[key.substr(1, key.size() - 1)] = lowestIndex;
                    delete siblingBucket;
                    delete bucket;
                    currentDepth--;
                } else {
                    done = true;
                }
            }
        }
    }
    if (bucket->next != -1) {
        cout << "Bucket has next" << endl;
        // we can bring an item from its next bucket to the current bucket
        auto nextBucket = loadBucket(bucket->next);
        bucket->insert(nextBucket->removeLast());
        if (nextBucket->isEmpty()) {
            bucket->next = nextBucket->next;
            saveBucket(bucket, index);
            delete nextBucket;
        } else {
            saveBucket(bucket, index);
            saveBucket(nextBucket, bucket->next);
        }
    }
    if (currentDepth != 0 && previousIndex == index && bucket->next == -1) {
        cout << "Possible merge" << endl;
        // We might want to merge the bucket with a sibling into its parent.
        bool done = false;
        while (!done && currentDepth > 0) {
            string key = currentSearch.to_string().substr((sizeof(int) * 8) - (currentDepth + 1), currentDepth + 1);
            string siblingKey = key;
            siblingKey[0] = (siblingKey[0] == '0') ? '1' : '0';
            int siblingIndex = directory[siblingKey];
            auto siblingBucket = loadBucket(siblingIndex);

            if (siblingBucket->size + bucket->size <= maxBucketSize) {
                // Parent bucket is not a bucket as it's been split before
                // We can reuse the lowest index of the sibling and merge the two buckets
                int lowestIndex = (index < siblingIndex) ? index : siblingIndex;
                auto parentBucket = new Bucket();
                parentBucket->localDepth = currentDepth - 1;
                parentBucket->size = siblingBucket->size + bucket->size;
                for (int i = 0; i < siblingBucket->size; i++) {
                    parentBucket->insert(siblingBucket->records[i]);
                }
                for (int i = 0; i < bucket->size; i++) {
                    parentBucket->insert(bucket->records[i]);
                }
                saveBucket(parentBucket, lowestIndex);
                directory.erase(key);
                directory.erase(siblingKey);
                directory[key.substr(1, key.size() - 1)] = lowestIndex;
                delete siblingBucket;
                delete bucket;
                currentDepth--;
            } else {
                done = true;
            }
        }
    }
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
    cout << "Deleting 6" << endl;
    eh.deleteItem(6);
    cout << "Buckets in directory:" << endl;
    eh.printAllBucketsFromDir();
    cout << "Buckets in memory:" << endl;
    eh.printAllBucketsFromMemory();
}