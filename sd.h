#ifndef SD_H
#define SD_H

struct csd {
	unsigned int                    : 2;
	unsigned int file_format        : 2;
	unsigned int tmp_write_protect  : 1;
	unsigned int perm_write_protect : 1;
	unsigned int copy               : 1;
	unsigned int file_format_grp    : 1;
	unsigned int                    : 5;
	unsigned int write_bl_partial   : 1;
	unsigned int write_bl_len       : 4;
	unsigned int r2w_factor         : 3;
	unsigned int                    : 2;
	unsigned int wp_grp_enable      : 1;
	unsigned int wp_grp_size        : 7;
	unsigned int sector_size        : 7;
	unsigned int erase_blk_en       : 1;
	unsigned int c_size_mult        : 3;
	unsigned int vdd_w_curr_max     : 3;
	unsigned int vdd_w_curr_min     : 3;
	unsigned int vdd_r_curr_max     : 3;
	unsigned int vdd_r_curr_min     : 3;
	unsigned int c_size             : 12;
	unsigned int                    : 2;
	unsigned int dsr_imp            : 1;
	unsigned int read_blk_misalign  : 1;
	unsigned int write_blk_misalign : 1;
	unsigned int read_bl_partial    : 1;
	unsigned int read_bl_len        : 4;
	unsigned int ccc                : 12;
	unsigned int tran_speed         : 8;
	unsigned int nsac               : 8;
	unsigned int taac               : 8;
	unsigned int                    : 6;
	unsigned int csd_structure      : 2;
};

struct sd_card {
	unsigned int type;
	unsigned int rca;
	struct csd csd;
};


int sd_init(struct sd_card *card);
void sd_parse_csd(int raw[4], struct csd *csd);

#endif
