#include "fat32.h"

#include "mem.h"

void fat_init(struct fat_boot_sector *bs, struct fat *fat) {
	fat->sector_len = bs->sector_len;
	fat->cluster_len = bs->cluster_len;
	fat->nb_fats = bs->nb_fats;
}

void fat32_init(struct sd_card *sd, unsigned int start, struct fat *fat) {
	struct fat_boot_sector *bs = kmalloc(512, KMALLOC_CONT);
	sd_read(sd, start, 1, bs);

	fat_init(bs, fat);

	fat->fat_bits = 32;
	fat->fat_len = bs->e32.fat_len;
	fat->first_fat = start + bs->reserved_sectors;
	fat->root = fat->first_fat + fat->nb_fats * fat->fat_len;

	//kfree(bs);
}
