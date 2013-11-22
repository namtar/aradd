/* 
 * File:   main.c
 * Author: Matthias Drummer <s0542834>
 *
 * Created on 22. November 2013, 09:56
 */

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>
#include <sys/stat.h>

#include "archiver.h"

off_t oldLenghtOfFile;
off_t lenghtOfFile; // in bytes

/**
 * Adds a file to the end of the archive
 * 
 * @param fileName the name of the file to add
 * @param archiveFd the file descriptor of the archive to write to
 * 
 * @return a return code. Success or Failure
 */
int addFile(char* fileName, int archiveFd);

/*
 * Main function of the aradd project.
 */
int main(int argc, char** argv) {

    if (argc == 1) {
        printf("No archive name given. Exit program\n");
        return EXIT_FAILURE;
    } else if (argc != 3) {
        // number of arguments must be 3.
        // programName, ArchiveName, File to add
        printf("2 Arguments needed. 1st Archive to use, 2nd File to add\n");
        return EXIT_FAILURE;
    }

    // open archive by given name
    int fileDescriptor = open(argv[1], O_RDWR, S_IRWXU | S_IRWXG);
    if (fileDescriptor == -1) {
        return EXIT_FAILURE;
    }

    // check magic number
    short magicNumber;
    int numberOfBytesRead = read(fileDescriptor, &magicNumber, 2);
    if (numberOfBytesRead == -1) {
        printf("Error when reading the magic number from the archive\n");
        return EXIT_FAILURE;
    }

    // check for correct archive
    if (magicNumber != 0x4242) {
        printf("Falsches Archiv\n");
        return EXIT_FAILURE;
    }

    // TODO: find first index entry which is free. If we reach EOA then create a new index at the end of the file

    // read first index
    Archive_Index indexes[16];
    numberOfBytesRead = read(fileDescriptor, indexes, sizeof (indexes));

    if (numberOfBytesRead == -1) {
        printf("Error when reading the archive index\n");
        return EXIT_FAILURE;
    }

    // determine original lenght of file;
    struct stat fileStat;
    if (fstat(fileDescriptor, &fileStat) == -1) {
        printf("Failure when determining the lenght of the file\n");
        return EXIT_FAILURE;
    }
    lenghtOfFile = fileStat.st_size;
    oldLenghtOfFile = fileStat.st_size;

    // before iterating check for another index
    //    while (indexes[15].state == Index_State.CONTINUE) {
    //        // fetch the next archive
    //
    ////        if (lseek(fileDescriptor, 0, SEEK_END) == -1) {
    //        if (lseek(fileDescriptor, indexes[15].bytePositionInArchive, SEEK_SET) == -1) {
    //            printf("Fehler beim Positionieren des fileDescriptors");
    //            return EXIT_FAILURE;
    //        }
    //
    //        if (read(fileDescriptor, indexes, sizeof (indexes)) == -1) {
    //            printf("Error when reading the archive index");
    //            return EXIT_FAILURE;
    //        }
    //    }

    int i;
    for (i = 0; i < 16; i++) {
        Archive_Index index = indexes[i];
        if (index.state == FREE) {
            // when free add file and stop loop
            break;
        } else if (index.state == EOA) {
            // add new index at the end of the archive
        }
    }

    close(fileDescriptor);

    return (EXIT_SUCCESS);
}

int addFile(char *fileName, int archiveFd) {

    char *buffer[100]; // buffer to use

    int fileDescriptor = open(fileName, O_RDONLY, S_IRUSR | S_IRGRP);
    if (fileDescriptor == -1) {
        return EXIT_FAILURE;
    }

    struct stat fStat;   
    if (fstat(fileDescriptor, &fStat) == -1) {
        printf("Failure when determining the lenght of the file\n");
        return EXIT_FAILURE;
    }

    int numberOfBytesRead = read(fileDescriptor, buffer, sizeof (buffer));

    lseek(archiveFd, 0, SEEK_END); // set the archive file descriptor to the end of the file
//    write(fileDescriptor,)
}

