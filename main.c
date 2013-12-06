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
#include <string.h>

#include "archiver.h"

off_t oldLenghtOfFile;
off_t lenghtOfFile; // in bytes
int indexElementPosition = 0;
int startPositionOfFile = 0;

/**
 * Adds a file to the end of the archive
 * 
 * @param fileName the name of the file to add
 * @param archiveFd the file descriptor of the archive to write to
 * 
 * @return a return code. Success or Failure
 */
int addFile(char* fileName, int archiveFd);

/**
 * Function checks if there are any free indexes left and appends a new list of index elements to the end of the archive if not.
 * The position of the first free index will be stored in a global variable for later use.
 * 
 * @param archiveFd the file descriptor of the archive.
 * 
 * @return a return code. Success or Failure
 */
int doArchiveIndexMagic(int archiveFd);

/**
 * Writes the directory of contents to the archive.
 * 
 * @param fileDescriptor the file descriptor of the archive
 * @param indexes an array of 16 Archive Index structs
 * 
 * @return the file descriptor. If anythin wents wrong -1 is returned
 */
int writeDirectoryOfContents(int fileDescriptor, Archive_Index indexes[]);

/**
 * Helper function to gernerate the global array of archive index structs.
 * 
 * @param indexes an array of 16 Archive Index structs
 */
void generateIndexes(Archive_Index indexes[]);

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
    int archiveFd = open(argv[1], O_RDWR, S_IRWXU | S_IRWXG);
    if (archiveFd == -1) {
        return EXIT_FAILURE;
    }

    // check magic number
    short magicNumber;
    int numberOfBytesRead = read(archiveFd, &magicNumber, 2);
    if (numberOfBytesRead == -1) {
        printf("Error when reading the magic number from the archive\n");
        return EXIT_FAILURE;
    }

    // check for correct archive
    if (magicNumber != 0x4242) {
        printf("Falsches Archiv\n");
        return EXIT_FAILURE;
    }

    // determine original lenght of file;
    struct stat fileStat;
    if (fstat(archiveFd, &fileStat) == -1) {
        printf("Failure when determining the lenght of the file\n");
        return EXIT_FAILURE;
    }
    lenghtOfFile = fileStat.st_size;
    oldLenghtOfFile = fileStat.st_size;


    // TODO: important, before appending the file check if we have index entries left.    
    if (doArchiveIndexMagic(archiveFd) == EXIT_FAILURE) {
        return EXIT_FAILURE;
    }

    if (addFile(argv[2], archiveFd) == -1) {
        printf("Failure when writing to the archive\n");
        close(archiveFd);
    }

    // determine original lenght of file;
    struct stat newArchiveFileStat;
    if (fstat(archiveFd, &newArchiveFileStat) == -1) {
        printf("Failure when determining the lenght of the file\n");
        return EXIT_FAILURE;
    }
    struct stat addFileStat;
    if (fstat(archiveFd, &addFileStat) == -1) {
        printf("Failure when determining the lenght of the file\n");
        return EXIT_FAILURE;
    }

    // Write ArchiveIndex entry for added file
    Archive_Index newIndex;
    newIndex.bytePositionInArchive = startPositionOfFile;
    strcpy(newIndex.fileName, argv[2]);
    newIndex.fileType = PLAINFILE;
    newIndex.gid = addFileStat.st_gid;
    newIndex.state = ACTUAL;
    newIndex.uid = addFileStat.st_uid;
    time_t now;
    localtime(&now);
    newIndex.lastAccessTime = now;
    newIndex.sizeInBytes = newArchiveFileStat.st_size - oldLenghtOfFile;

    lseek(archiveFd, indexElementPosition, SEEK_SET);

    int bytesWritten = write(archiveFd, &newIndex, sizeof (newIndex));
    if (bytesWritten == -1) {
        printf("There was an error writing the new Index Element for the added file\n");
        return EXIT_FAILURE;
    }

    close(archiveFd);

    return (EXIT_SUCCESS);
}

int addFile(char *fileName, int archiveFd) {

    char buffer[100]; // buffer to use

    //int fileDescriptor = open(fileName, O_RDONLY, S_IRUSR | S_IRGRP
    int fileDescriptor = open(fileName, O_RDONLY);
    if (fileDescriptor == -1) {
        close(archiveFd);
        return EXIT_FAILURE;
    }

    struct stat fStat;
    if (fstat(fileDescriptor, &fStat) == -1) {
        printf("Failure when determining the lenght of the file\n");
        // close the file which we have written
        close(fileDescriptor);
        close(archiveFd);
        return EXIT_FAILURE;
    }

    lseek(archiveFd, 0, SEEK_END); // set the archive file descriptor to the end of the file
    startPositionOfFile = archiveFd;

    int totalNumberOfBytesWritten = 0;
    int numberOfBytesRead;
    // to do this in the head of the while loop is bad style.
    while ((numberOfBytesRead = read(fileDescriptor, buffer, sizeof (buffer))) > 0) {
        // if numberOfByesRead is 0 then the end of the file has reached.
        totalNumberOfBytesWritten += numberOfBytesRead;
        //printf("Number Of bytes: %i\n", numberOfBytesRead);
        //printf("Inhalt des Puffers: %s\n", buffer);

        // write the number of bytes to the archive
        if (write(archiveFd, &buffer, numberOfBytesRead) == -1) {
            printf("Error when writing a file to the archive.\n");
            // close the file which we have written
            close(archiveFd);
            // close the file we have read
            close(fileDescriptor);
            return EXIT_FAILURE;
        }
        lenghtOfFile += numberOfBytesRead;
    }
    printf("Total number of bytes written: %i\n", totalNumberOfBytesWritten);

    // close the file which we have written
    close(fileDescriptor);
}

int doArchiveIndexMagic(int archiveFd) {

    int startPositionOfIndex = 2;

    lseek(archiveFd, startPositionOfIndex, SEEK_SET);

    // read first index
    Archive_Index indexes[16];
    int numberOfBytesRead = read(archiveFd, indexes, sizeof (indexes));

    if (numberOfBytesRead == -1) {
        printf("Error when reading the archive index\n");
        return EXIT_FAILURE;
    }

    // check last index element if there are more indexes
    short done = 0;
    while (done == 0) {
        if (indexes[15].state == CONTINUE) {
            // load the next index
            lseek(archiveFd, indexes[15].bytePositionInArchive, SEEK_SET);
            startPositionOfIndex = indexes[15].bytePositionInArchive;
            int numberOfBytesRead = read(archiveFd, indexes, sizeof (indexes));
            if (numberOfBytesRead == -1) {
                printf("Error when reading the archive index\n");
                return EXIT_FAILURE;
            }
        } else {
            done = 1;
            // get the first free index element.
            // if there is none, created new index            
            int i;
            for (i = 0; i < 15; i++) {
                if (indexes[i].state == FREE) {
                    indexElementPosition = startPositionOfIndex += (sizeof (Archive_Index) * i);
                    break;
                }
            }
            if (indexElementPosition == 0) {
                // set filedescriptor to the end of the archive and append new index
                int positionOfLastIndex15 = startPositionOfIndex += (sizeof (Archive_Index) * 15);
                int positionOfNewIndex;
                // seek fd and generate new indexes
                lseek(archiveFd, 0, SEEK_END);
                positionOfNewIndex = archiveFd;
                indexElementPosition = positionOfNewIndex;

                Archive_Index newIndexes[16];
                generateIndexes(newIndexes);
                writeDirectoryOfContents(archiveFd, newIndexes);

                // update last element of old index
                Archive_Index lastIndex15 = indexes[15];
                lastIndex15.bytePositionInArchive = positionOfNewIndex;
                lastIndex15.state = CONTINUE;
                time_t now;
                localtime(&now);
                lastIndex15.lastAccessTime = now;

                // write old entry
                lseek(archiveFd, positionOfLastIndex15, SEEK_SET);
                int bytesWritten = write(archiveFd, &lastIndex15, sizeof (Archive_Index));
                if (bytesWritten == -1) {
                    printf("There was an error writing the ArchiveIndex element Nr. 15 to the Archive File.\n");
                    return EXIT_FAILURE;
                }
            }
        }
    }

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
}

int writeDirectoryOfContents(int fileDescriptor, Archive_Index indexes[]) {

    // as default we create a directory of contents for 16 entries. The last entry is always EOA or Continue, depending on further files following or not.
    int result;

    // http://stackoverflow.com/questions/12489525/file-permission-issue-with-open-system-call-linux

    result = write(fileDescriptor, indexes, sizeof (indexes));
    return result;
}

void generateIndexes(Archive_Index indexes[]) {
    int i;

    for (i = 0; i < 16; i++) {


        indexes[i].bytePositionInArchive = 0;
        indexes[i].fileName[0] = '\0';
        indexes[i].fileType = NONE;
        indexes[i].gid = -1;
        indexes[i].lastAccessTime = 0;
        indexes[i].sizeInBytes = 0;
        if (i == 15) {
            indexes[i].state = EOA;
        } else {
            indexes[i].state = FREE;
        }
        indexes[i].uid = -1;
    }

}

