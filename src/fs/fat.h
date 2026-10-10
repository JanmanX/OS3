#pragma once

#include <stdint.h>

/*
 * Read-only FAT12, FAT16 and FAT32.
 *
 * One volume is mounted at a time. Paths are absolute and '/'-separated, and
 * each part of one is matched, ignoring case, against an entry's long name -
 * or its 8.3 name when it has no long one.
 */

/* Longest name, terminator included. Anything outside ASCII reads as '?'. */
#define FAT_NAME_MAX	(256)

typedef struct fat_entry {
	char name[FAT_NAME_MAX];
	uint8_t is_dir;
	uint32_t size;
	uint32_t cluster;	/* First cluster, 0 for an empty file */
} fat_entry_t;

/* A position in a directory, for fat_readdir() */
typedef struct fat_dir {
	uint32_t cluster;	/* Being walked, 0 for a FAT12/16 root */
	uint32_t index;		/* Next entry within it */
	uint32_t steps;		/* Clusters walked, to stop on a looped chain */
	uint8_t done;
} fat_dir_t;

typedef struct fat_file {
	uint32_t size;
	uint32_t pos;
	uint32_t cluster;	/* The cluster holding "pos" */
} fat_file_t;

/* Mount the first FAT volume found on any disk, whether it fills the disk or
 * sits in an MBR partition. EOK, or ENODEV when there is none. */
uint8_t fat_init(void);

/* ENODEV with nothing mounted, ENOENT when the path leads nowhere, ENOTDIR
 * when it runs through - or, for fat_opendir(), ends at - a file */
uint8_t fat_opendir(const char *path, fat_dir_t *dir);

/* The next entry in on-disk order, "." and ".." included. EOK, or ENOENT
 * once there are no more. */
uint8_t fat_readdir(fat_dir_t *dir, fat_entry_t *out);

/* As fat_opendir(), but EISDIR when the path is a directory */
uint8_t fat_open(const char *path, fat_file_t *file);

/* Up to "len" bytes from the current position. Returns how many were read,
 * 0 at the end of the file, or a negative errno. */
int64_t fat_read(fat_file_t *file, void *buffer, uint64_t len);
