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

	fat->sd = sd;
	fat->fat_bits = 32;
	fat->fat_len = bs->e32.fat_len;
	fat->first_fat = start + bs->reserved_sectors;
	fat->root = fat->first_fat + fat->nb_fats * fat->fat_len;

	//kfree(bs);
}

void fat32_printdir(struct fat *fat, unsigned int addr, struct terminfo *term) {
	struct fat_direntry *dir = kmalloc(fat->cluster_len * fat->sector_len, KMALLOC_CONT);
	term_printf(term, "term_printf needed because of a bug...\n");
	sd_read(fat->sd, addr, fat->cluster_len * fat->sector_len / 512, dir);

	unsigned int max_entries = fat->cluster_len * fat->sector_len / sizeof(struct fat_direntry);

	for(unsigned int i=0 ; i < max_entries ; ++i) {
		// Label or long filename: don't print (yet)
		if(dir[i].attrs & FAT_DIRFLAG_VOL)
			continue;
		if(dir[i].filename[0] == 0xe5)
			continue;
		if(dir[i].filename[0] == 0x00)
			break;


		if(dir[i].attrs & FAT_DIRFLAG_DIR)
			term_printf(term, "d ");
		else
			term_printf(term, "- ");

		for(int c=0 ; c < 8 ; ++c)
			if(dir[i].filename[c] != ' ')
				term_print_char(term, dir[i].filename[c]);

		term_printf(term, ".");

		for(int c=0 ; c < 3 ; ++c)
			if(dir[i].ext[c] != ' ')
				term_print_char(term, dir[i].ext[c]);

		term_printf(term, "\n");
	}
}
