#include "fileHelper.h"

bool fileExists(const char* filename){
    FILE* file = fopen(filename, "r");
    if(file){
        fclose(file);
        return true;
    }
    return false;
}
bool touchFile(const char* filename){
    FILE* file = fopen(filename, "a");
    if(file){
        fclose(file);
        return true;
    }
    return false;
}
bool createFile(const char* filename){
    FILE* file = fopen(filename, "w");
    if(file){
        fclose(file);
        return true;
    }
    return false;
}