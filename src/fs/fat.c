#include "fat.h"
#include <stdint.h>
#include <types.h>
#include <errno.h>
#include <libc.h>
#include <drivers/ahci.h>

/* Directory entry */
#define DIR_ENTRY_SIZE		(32)
#define DIR_ATTR_VOLUME_ID	(0x08)
#define DIR_ATTR_DIRECTORY	(0x10)
#define DIR_ATTR_LFN		(0x0F)
#define DIR_ATTR_LFN_MASK	(0x3F)
#define DIR_FREE		(0xE5)
#define DIR_END			(0x00)

/* Long name entries: 13 UCS-2 characters each, at most 20 of them in a row */
#define LFN_CHARS		(13)
#define LFN_LAST		(0x40)
#define LFN_MAX_ENTRIES		(20)

/* FAT values from here up end a chain */
#define FAT12_EOC		(0xFF8)
#define FAT16_EOC		(0xFFF8)
#define FAT32_EOC		(0x0FFFFFF8)

#define FAT_BAD_CLUSTER		(0xFFFFFFFF)

#define MBR_SIGNATURE		(0xAA55)
#define MBR_PARTITION_TABLE	(446)

typedef enum { FAT12, FAT16, FAT32 } fat_type_t;

static struct {
	uint8_t mounted;
	fat_type_t type;
	uint32_t disk;
	uint64_t start;			/* First sector of the volume */
	uint32_t sectors_per_cluster;
	uint32_t cluster_bytes;
	uint32_t fat_start;		/* Volume-relative sectors */
	uint32_t root_start;		/* FAT12/16 only */
	uint32_t root_entries;		/* FAT12/16 only */
	uint32_t data_start;
	uint32_t root_cluster;		/* FAT32 only */
	uint32_t clusters;		/* Count of data clusters */
} vol;

/* One sector of the FAT is kept around: following a chain walks it in order,
 * so this turns most lookups into no reads at all */
static uint8_t fat_cache[AHCI_SECTOR_SIZE];
static uint64_t fat_cache_sector = (uint64_t)-1;

/* Scratch for directory sectors and partial file sectors */
static uint8_t sector[AHCI_SECTOR_SIZE];

static uint16_t le16(const uint8_t *p)
{
	return (uint16_t)(p[0] | (p[1] << 8));
}

static uint32_t le32(const uint8_t *p)
{
	return (uint32_t)p[0] | ((uint32_t)p[1] << 8)
	     | ((uint32_t)p[2] << 16) | ((uint32_t)p[3] << 24);
}


/* ------------------------------------------------------------------ disk */

/* Volume-relative sector. Anything that goes wrong is an I/O error. */
static uint8_t read_sectors(uint64_t lba, uint32_t count, void *buffer)
{
	if(ahci_read(vol.disk, vol.start + lba, count, buffer) != EOK) {
		return EIO;
	}

	return EOK;
}

static uint32_t cluster_sector(uint32_t cluster)
{
	return vol.data_start + (cluster - 2) * vol.sectors_per_cluster;
}

static uint8_t cluster_valid(uint32_t cluster)
{
	return cluster >= 2 && cluster - 2 < vol.clusters;
}

/* The cluster after this one. 0 for the end of the chain, and for a chain
 * that leads somewhere it should not. */
static uint32_t next_cluster(uint32_t cluster)
{
	uint32_t offset;
	uint64_t sec;
	uint32_t value;

	if(vol.type == FAT12) {
		offset = cluster + cluster / 2;
	} else if(vol.type == FAT16) {
		offset = cluster * 2;
	} else {
		offset = cluster * 4;
	}

	sec = vol.fat_start + offset / AHCI_SECTOR_SIZE;

	if(sec != fat_cache_sector) {
		if(read_sectors(sec, 1, fat_cache) != EOK) {
			fat_cache_sector = (uint64_t)-1;
			return 0;
		}

		fat_cache_sector = sec;
	}

	offset %= AHCI_SECTOR_SIZE;

	if(vol.type == FAT12) {
		/* The two bytes of a 12-bit value can straddle a sector */
		uint8_t pair[2];

		pair[0] = fat_cache[offset];

		if(offset + 1 < AHCI_SECTOR_SIZE) {
			pair[1] = fat_cache[offset + 1];
		} else {
			uint8_t next[AHCI_SECTOR_SIZE];

			if(read_sectors(sec + 1, 1, next) != EOK) {
				return 0;
			}

			pair[1] = next[0];
		}

		value = le16(pair);
		value = (cluster & 1) ? value >> 4 : value & 0xFFF;

		if(value >= FAT12_EOC) {
			return 0;
		}
	} else if(vol.type == FAT16) {
		value = le16(&fat_cache[offset]);

		if(value >= FAT16_EOC) {
			return 0;
		}
	} else {
		value = le32(&fat_cache[offset]) & 0x0FFFFFFF;

		if(value >= FAT32_EOC) {
			return 0;
		}
	}

	return cluster_valid(value) ? value : 0;
}


/* ----------------------------------------------------------------- mount */

/* Is the sector at "lba" the start of a FAT volume? Fills in "vol" if so. */
static uint8_t try_volume(uint32_t disk, uint64_t lba)
{
	uint8_t bpb[AHCI_SECTOR_SIZE];
	uint32_t bytes_per_sector;
	uint32_t reserved;
	uint32_t fats;
	uint32_t root_entries;
	uint32_t total;
	uint32_t fat_size;
	uint32_t root_sectors;
	uint32_t data_sectors;
	uint32_t spc;

	if(ahci_read(disk, lba, 1, bpb) != EOK) {
		return 0;
	}

	if(le16(&bpb[510]) != MBR_SIGNATURE) {
		return 0;
	}

	/* A boot sector starts with a jump over the BPB. A partition table
	 * need not, and its fields would otherwise pass for a BPB by chance. */
	if(bpb[0] != 0xEB && bpb[0] != 0xE9) {
		return 0;
	}

	bytes_per_sector = le16(&bpb[11]);
	spc = bpb[13];
	reserved = le16(&bpb[14]);
	fats = bpb[16];
	root_entries = le16(&bpb[17]);
	total = le16(&bpb[19]);

	if(total == 0) {
		total = le32(&bpb[32]);
	}

	fat_size = le16(&bpb[22]);

	if(fat_size == 0) {
		fat_size = le32(&bpb[36]);
	}

	/* Only the sector size the disk driver deals in */
	if(bytes_per_sector != AHCI_SECTOR_SIZE) {
		return 0;
	}

	if(spc == 0 || (spc & (spc - 1)) != 0 || reserved == 0 || fats == 0
	   || fat_size == 0 || total == 0) {
		return 0;
	}

	root_sectors = (root_entries * DIR_ENTRY_SIZE + bytes_per_sector - 1)
		       / bytes_per_sector;

	if(reserved + fats * fat_size + root_sectors >= total) {
		return 0;
	}

	data_sectors = total - (reserved + fats * fat_size + root_sectors);

	vol.disk = disk;
	vol.start = lba;
	vol.sectors_per_cluster = spc;
	vol.cluster_bytes = spc * bytes_per_sector;
	vol.fat_start = reserved;
	vol.root_entries = root_entries;
	vol.root_start = reserved + fats * fat_size;
	vol.data_start = vol.root_start + root_sectors;
	vol.clusters = data_sectors / spc;
	vol.root_cluster = 0;

	/* The cluster count alone settles the type; the label in the boot
	 * sector is only a hint and often wrong */
	if(vol.clusters < 4085) {
		vol.type = FAT12;
	} else if(vol.clusters < 65525) {
		vol.type = FAT16;
	} else {
		vol.type = FAT32;
		vol.root_cluster = le32(&bpb[44]);
		vol.root_entries = 0;

		if(!cluster_valid(vol.root_cluster)) {
			return 0;
		}
	}

	fat_cache_sector = (uint64_t)-1;
	vol.mounted = 1;

	return 1;
}

static uint8_t is_fat_partition(uint8_t type)
{
	return type == 0x01 || type == 0x04 || type == 0x06 || type == 0x0B
	    || type == 0x0C || type == 0x0E;
}

uint8_t fat_init(void)
{
	uint32_t disk;
	uint32_t i;

	vol.mounted = 0;

	for(disk = 0; disk < ahci_disk_count(); disk++) {
		uint8_t mbr[AHCI_SECTOR_SIZE];

		/* A volume that fills the whole disk */
		if(try_volume(disk, 0)) {
			goto mounted;
		}

		if(ahci_read(disk, 0, 1, mbr) != EOK
		   || le16(&mbr[510]) != MBR_SIGNATURE) {
			continue;
		}

		for(i = 0; i < 4; i++) {
			const uint8_t *e = &mbr[MBR_PARTITION_TABLE + i * 16];

			if(!is_fat_partition(e[4]) || le32(&e[12]) == 0) {
				continue;
			}

			if(try_volume(disk, le32(&e[8]))) {
				goto mounted;
			}
		}
	}

	LOG("FAT: no volume found");
	return ENODEV;

mounted:
	LOGF("FAT: FAT%d volume on disk %d at sector %d, %d clusters of %d bytes\n",
	     (uint64_t)(vol.type == FAT12 ? 12 : vol.type == FAT16 ? 16 : 32),
	     (uint64_t)vol.disk, (uint64_t)vol.start,
	     (uint64_t)vol.clusters, (uint64_t)vol.cluster_bytes);

	return EOK;
}


/* ------------------------------------------------------------- directory */

static void dir_root(fat_dir_t *dir)
{
	dir->cluster = vol.type == FAT32 ? vol.root_cluster : 0;
	dir->index = 0;
	dir->steps = 0;
	dir->done = 0;
}

/* The 8.3 name as a string: "NAME.EXT", trailing blanks dropped */
static void short_name(const uint8_t *e, char *out)
{
	uint32_t n = 0;
	uint32_t i;
	uint32_t base_len = 8;
	uint32_t ext_len = 3;
	uint8_t lower_base = (e[12] & 0x08) != 0;
	uint8_t lower_ext = (e[12] & 0x10) != 0;

	while(base_len > 0 && e[base_len - 1] == ' ') {
		base_len--;
	}

	while(ext_len > 0 && e[8 + ext_len - 1] == ' ') {
		ext_len--;
	}

	for(i = 0; i < base_len; i++) {
		uint8_t c = e[i];

		/* A leading 0xE5 is stored as 0x05 so it is not read as free */
		if(i == 0 && c == 0x05) {
			c = 0xE5;
		}

		if(c >= 0x80) {
			c = '?';
		} else if(lower_base) {
			c = (uint8_t)tolower(c);
		}

		out[n++] = (char)c;
	}

	if(ext_len > 0) {
		out[n++] = '.';

		for(i = 0; i < ext_len; i++) {
			uint8_t c = e[8 + i];

			if(c >= 0x80) {
				c = '?';
			} else if(lower_ext) {
				c = (uint8_t)tolower(c);
			}

			out[n++] = (char)c;
		}
	}

	out[n] = '\0';
}

static uint8_t short_checksum(const uint8_t *e)
{
	uint8_t sum = 0;
	uint32_t i;

	for(i = 0; i < 11; i++) {
		sum = (uint8_t)(((sum & 1) ? 0x80 : 0x00) + (sum >> 1) + e[i]);
	}

	return sum;
}

/* Copy the characters of one long name entry into place, 13 per entry */
static void lfn_collect(const uint8_t *e, char *name, uint32_t slot)
{
	static const uint8_t offsets[LFN_CHARS] =
		{ 1, 3, 5, 7, 9, 14, 16, 18, 20, 22, 24, 28, 30 };
	uint32_t i;

	for(i = 0; i < LFN_CHARS; i++) {
		uint16_t c = le16(&e[offsets[i]]);
		uint32_t at = slot * LFN_CHARS + i;

		if(at >= FAT_NAME_MAX - 1) {
			continue;
		}

		name[at] = (c == 0 || c == 0xFFFF) ? '\0'
			 : (c < 0x80 ? (char)c : '?');
	}
}

uint8_t fat_readdir(fat_dir_t *dir, fat_entry_t *out)
{
	uint32_t per_cluster = vol.cluster_bytes / DIR_ENTRY_SIZE;
	char lfn[FAT_NAME_MAX];
	uint32_t lfn_slots = 0;		/* Entries of the long name seen */
	uint8_t lfn_sum = 0;
	uint32_t lfn_expect = 0;	/* Order number the next one must have */

	if(!vol.mounted) {
		return ENODEV;
	}

	while(!dir->done) {
		const uint8_t *e;
		uint32_t limit;
		uint32_t in_sector;
		uint64_t lba;

		/* Past the end of this cluster (or of the fixed root) */
		limit = dir->cluster == 0 ? vol.root_entries : per_cluster;

		if(dir->index >= limit) {
			if(dir->cluster == 0) {
				dir->done = 1;
				break;
			}

			dir->cluster = next_cluster(dir->cluster);
			dir->index = 0;

			if(dir->cluster == 0 || ++dir->steps > vol.clusters) {
				dir->done = 1;
				break;
			}

			continue;
		}

		if(dir->cluster == 0) {
			lba = vol.root_start
			    + dir->index * DIR_ENTRY_SIZE / AHCI_SECTOR_SIZE;
		} else {
			lba = cluster_sector(dir->cluster)
			    + dir->index * DIR_ENTRY_SIZE / AHCI_SECTOR_SIZE;
		}

		in_sector = (dir->index * DIR_ENTRY_SIZE) % AHCI_SECTOR_SIZE;

		if(read_sectors(lba, 1, sector) != EOK) {
			dir->done = 1;
			break;
		}

		e = &sector[in_sector];
		dir->index++;

		if(e[0] == DIR_END) {
			dir->done = 1;
			break;
		}

		if(e[0] == DIR_FREE) {
			lfn_slots = 0;
			continue;
		}

		if((e[11] & DIR_ATTR_LFN_MASK) == DIR_ATTR_LFN) {
			uint32_t order = e[0] & 0x1F;

			if(e[0] & LFN_LAST) {
				/* Starts a name; the highest order comes first */
				if(order == 0 || order > LFN_MAX_ENTRIES) {
					lfn_slots = 0;
					continue;
				}

				memset(lfn, 0, sizeof(lfn));
				lfn_slots = order;
				lfn_sum = e[13];
				lfn_expect = order;
			} else if(lfn_slots == 0 || order != lfn_expect
				  || e[13] != lfn_sum) {
				lfn_slots = 0;
				continue;
			}

			lfn_collect(e, lfn, order - 1);
			lfn_expect = order - 1;

			continue;
		}

		if(e[11] & DIR_ATTR_VOLUME_ID) {
			lfn_slots = 0;
			continue;
		}

		/* A real entry. Its long name only counts if the entries
		 * before it ran in order down to 1 and match its checksum. */
		if(lfn_slots != 0 && lfn_expect == 0
		   && lfn_sum == short_checksum(e)) {
			lfn[FAT_NAME_MAX - 1] = '\0';
			strncpy(out->name, lfn, FAT_NAME_MAX);
			out->name[FAT_NAME_MAX - 1] = '\0';
		} else {
			short_name(e, out->name);
		}

		lfn_slots = 0;

		out->is_dir = (e[11] & DIR_ATTR_DIRECTORY) != 0;
		out->size = out->is_dir ? 0 : le32(&e[28]);
		out->cluster = ((uint32_t)le16(&e[20]) << 16) | le16(&e[26]);

		if(vol.type != FAT32) {
			out->cluster &= 0xFFFF;
		}

		return EOK;
	}

	return ENOENT;
}


/* ------------------------------------------------------------------ path */

static char lower(char c)
{
	return (char)tolower((unsigned char)c);
}

static uint8_t names_match(const char *a, const char *b)
{
	while(*a && *b) {
		if(lower(*a) != lower(*b)) {
			return 0;
		}

		a++;
		b++;
	}

	return *a == *b;
}

/* Find "name" in a directory */
static uint8_t dir_lookup(const fat_dir_t *where, const char *name,
			  fat_entry_t *found)
{
	fat_dir_t dir = *where;
	uint8_t status;

	dir.index = 0;
	dir.steps = 0;
	dir.done = 0;

	while((status = fat_readdir(&dir, found)) == EOK) {
		if(names_match(found->name, name)) {
			return EOK;
		}
	}

	return status;
}

/* Resolve a path to its entry. The root has none, so it is reported as a
 * synthetic directory entry with the root's cluster. */
static uint8_t resolve(const char *path, fat_entry_t *entry)
{
	fat_dir_t dir;
	char part[FAT_NAME_MAX];

	if(!vol.mounted) {
		return ENODEV;
	}

	dir_root(&dir);

	part[0] = '\0';
	entry->name[0] = '\0';
	entry->is_dir = 1;
	entry->size = 0;
	entry->cluster = dir.cluster;

	while(*path == '/') {
		path++;
	}

	while(*path) {
		uint32_t n = 0;
		uint8_t status;

		/* A file has nothing inside it to look in */
		if(!entry->is_dir) {
			return ENOTDIR;
		}

		while(*path && *path != '/') {
			if(n >= FAT_NAME_MAX - 1) {
				return ENOENT;
			}

			part[n++] = *path++;
		}

		part[n] = '\0';

		while(*path == '/') {
			path++;
		}

		/* ".." out of a first-level folder is stored as cluster 0,
		 * meaning the root - which FAT32 wants as its real cluster */
		dir.cluster = entry->cluster;

		if(dir.cluster == 0 && vol.type == FAT32) {
			dir.cluster = vol.root_cluster;
		}

		status = dir_lookup(&dir, part, entry);

		if(status != EOK) {
			return status;
		}

		if(entry->is_dir && entry->cluster == 0 && vol.type == FAT32) {
			entry->cluster = vol.root_cluster;
		}
	}

	return EOK;
}

uint8_t fat_opendir(const char *path, fat_dir_t *dir)
{
	fat_entry_t entry;
	uint8_t status = resolve(path, &entry);

	if(status != EOK) {
		return status;
	}

	if(!entry.is_dir) {
		return ENOTDIR;
	}

	dir->cluster = entry.cluster;
	dir->index = 0;
	dir->steps = 0;
	dir->done = 0;

	return EOK;
}


/* ------------------------------------------------------------------ file */

uint8_t fat_open(const char *path, fat_file_t *file)
{
	fat_entry_t entry;
	uint8_t status = resolve(path, &entry);

	if(status != EOK) {
		return status;
	}

	if(entry.is_dir) {
		return EISDIR;
	}

	file->size = entry.size;
	file->pos = 0;
	file->cluster = entry.cluster;

	return EOK;
}

int64_t fat_read(fat_file_t *file, void *buffer, uint64_t len)
{
	uint8_t *dst = buffer;
	uint64_t done = 0;

	if(!vol.mounted) {
		return -ENODEV;
	}

	if(len > file->size - file->pos) {
		len = file->size - file->pos;
	}

	while(done < len) {
		uint32_t in_cluster = file->pos % vol.cluster_bytes;
		uint32_t in_sector = in_cluster % AHCI_SECTOR_SIZE;
		uint32_t chunk;

		/* An empty file, or a chain that ended before the size did */
		if(!cluster_valid(file->cluster)) {
			return done > 0 ? (int64_t)done : -EIO;
		}

		chunk = AHCI_SECTOR_SIZE - in_sector;

		if(chunk > len - done) {
			chunk = (uint32_t)(len - done);
		}

		if(chunk == AHCI_SECTOR_SIZE) {
			/* Whole sector: straight into the caller's buffer */
			if(read_sectors(cluster_sector(file->cluster)
					+ in_cluster / AHCI_SECTOR_SIZE,
					1, dst + done) != EOK) {
				return done > 0 ? (int64_t)done : -EIO;
			}
		} else {
			if(read_sectors(cluster_sector(file->cluster)
					+ in_cluster / AHCI_SECTOR_SIZE,
					1, sector) != EOK) {
				return done > 0 ? (int64_t)done : -EIO;
			}

			memcpy(dst + done, sector + in_sector, chunk);
		}

		done += chunk;
		file->pos += chunk;

		/* Moved on to the next cluster. Only follow the chain while
		 * there is more to read - the last cluster has no next. */
		if(file->pos % vol.cluster_bytes == 0 && file->pos < file->size) {
			file->cluster = next_cluster(file->cluster);
		}
	}

	return (int64_t)done;
}
