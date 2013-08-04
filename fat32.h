#ifndef H_FAT32
#define H_FAT32

struct ebpb_16 {
};

struct ebpb_32 {
	unsigned int fat_len;
	unsigned short flags;
	unsigned char version_min, version_maj;
	unsigned int root_cluster;
	unsigned short fsinfo_cluster;
	unsigned short backup_boot_sector;
	unsigned char reserved1[12];
	unsigned char drive_num;
	unsigned char reserved2;
	unsigned char sig;
	unsigned int volumeid;
	unsigned char label[11];
	unsigned char fat32_id[8];
	unsigned char code[420];
	unsigned char boot_sig[2];
} __attribute__((packed));

struct fat_boot_sector {
	unsigned char code[3];
	unsigned char oem[8];
	unsigned short sector_len; // in bytes
	unsigned char cluster_len; // in sectors
	unsigned short reserved_sectors;
	unsigned char nb_fats;
	unsigned short nb_root_entries;
	unsigned short nb_sectors;
	unsigned char media_type;
	unsigned short fat_len; // in sectors, FAT12 & 16 only
	unsigned short track_len; // in sectors
	unsigned short nb_sides;
	unsigned int hid_sectors;
	unsigned int nb_sectors_large;
	union {
		struct ebpb_16 e16;
		struct ebpb_32 e32;
	};
} __attribute__((packed));

struct fat {
	unsigned short sector_len; // in bytes
	unsigned char cluster_len; // in sectors

	unsigned char nb_fats;
	unsigned char fat_bits;
	unsigned int fat_len;
	unsigned int first_fat;
	unsigned int root;
};

#include "sd.h"
void fat32_init(struct sd_card *sd, unsigned int start, struct fat *fat);

#endif
