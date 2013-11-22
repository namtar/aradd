/* 
 * File:   archiver.c
 * Author: drummer
 *
 * Created on 27. Oktober 2013, 15:20
 */

#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include "archiver.h"

char* mapIndexStateToString(enum Index_State indexState) {

    //  FREE, ACTUAL, DELETED, REPLACED, EOA, CONTINUE

    switch (indexState) {
        case 0:
            return "FREE";
        case 1:
            return "ACTUAL";
        case 2:
            return "DELETED";
        case 3:
            return "REPLACED";
        case 4:
            return "EOA";
        case 5:
            return "CONTINUE";
        default:
            printf("Enum value is not known / mapped.");
            exit(EXIT_FAILURE);
    }
}

char* mapFileTypeToString(enum File_Type fileType) {

    // NONE, DIRECTORY, PLAINFILE

    switch (fileType) {
        case 0:
            return "NONE";
        case 1:
            return "DIRECTORY";
        case 2:
            return "PLAINFILE";
        default:
            printf("Enum value is not known / mapped.");
            exit(EXIT_FAILURE);
    }
}