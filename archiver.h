/*
 * Common header for all archiver programs (arnew, ardir, aradd, ardel, arrep, arclean
 *  
 * File:   archiver.h
 * Author: drummer
 *
 * Created on 27. Oktober 2013, 15:19
 */

#ifndef ARCHIVER_H
#define	ARCHIVER_H

#ifdef	__cplusplus
extern "C" {
#endif

#ifdef	__cplusplus
}
#endif

// typedefs
typedef struct Archive_Index Archive_Index;


// enumerations

enum Index_State {
    FREE, ACTUAL, DELETED, REPLACED, EOA, CONTINUE
};

enum File_Type {
    NONE, DIRECTORY, PLAINFILE
};


// constants
#define MAGIC_NUMBER_ARCHIVE "0x4242" // TODO: soll diese nummer nicht vielleicht als char gespeichert werden?

// define structs

/**
 * A struct that represents an index entry.
 **/
struct Archive_Index {
    enum Index_State state;
    time_t lastAccessTime;
    uid_t uid;
    gid_t gid;
    enum File_Type fileType;
    char fileName[255];
    off_t sizeInBytes;
    off_t bytePositionInArchive; // if Index_State is CONTINUE, then this position is the position of the next index in the archive.
};

/******************* function definitions **********************/


/**
 * Maps a given Index_State enum to a string.
 * 
 * @param indexState the {@link Index_State}
 * @return the string representation of the enum
 */
char* mapIndexStateToString(enum Index_State indexState);

/**
 * Maps a given FileType enum to a string.
 * 
 * @param fileType the given {@link File_Type}
 * @return the string representation of the enum
 */
char* mapFileTypeToString(enum File_Type fileType);

#endif	/* ARCHIVER_H */

