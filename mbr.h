#ifndef MBR_H
#define MBR_H

#define PARTTYPE_FAT32 0x0b

struct mbr_part_entry {
	unsigned char status;
	unsigned char chs_start[3];
	unsigned char type;
	unsigned char chs_end[3];
	unsigned int lba_first;
	unsigned int nb_sectors;
} __attribute__((packed));

struct mbr {
	unsigned char code[446];
	struct mbr_part_entry partitions[4];
	unsigned char signature[2];
} __attribute__((packed));

struct partition {
	unsigned int start, size;
	unsigned char type;
};

void parse_partition_table(struct mbr *mbr, struct partition parts[4]);

#endif
