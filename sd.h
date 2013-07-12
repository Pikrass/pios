#ifndef SD_H
#define SD_H

#define EMMC_BASE      (void*)(0x20300000)
#define ARG2           (int*)(EMMC_BASE + 0x00)
#define BLKSIZECNT     (int*)(EMMC_BASE + 0x04)
#define ARG1           (int*)(EMMC_BASE + 0x08)
#define CMDTM          (int*)(EMMC_BASE + 0x0c)
#define RESP0          (int*)(EMMC_BASE + 0x10)
#define RESP1          (int*)(EMMC_BASE + 0x14)
#define RESP2          (int*)(EMMC_BASE + 0x18)
#define RESP3          (int*)(EMMC_BASE + 0x1c)
#define DATA           (int*)(EMMC_BASE + 0x20)
#define STATUS         (int*)(EMMC_BASE + 0x24)
#define CONTROL0       (int*)(EMMC_BASE + 0x28)
#define CONTROL1       (int*)(EMMC_BASE + 0x2c)
#define INTERRUPT      (int*)(EMMC_BASE + 0x30)
#define IRPT_MASK      (int*)(EMMC_BASE + 0x34)
#define IRPT_EN        (int*)(EMMC_BASE + 0x38)
#define CONTROL2       (int*)(EMMC_BASE + 0x3c)
#define FORCE_IRPT     (int*)(EMMC_BASE + 0x50)
#define BOOT_TIMEOUT   (int*)(EMMC_BASE + 0x70)
#define DBG_SEL        (int*)(EMMC_BASE + 0x74)
#define EXRDFIFO_CFG   (int*)(EMMC_BASE + 0x80)
#define EXRDFIFO_EN    (int*)(EMMC_BASE + 0x84)
#define TUNE_STEP      (int*)(EMMC_BASE + 0x88)
#define TUNE_STEP_STD  (int*)(EMMC_BASE + 0x8c)
#define TUNE_STEP_DDR  (int*)(EMMC_BASE + 0x90)
#define SPI_INT_SPT    (int*)(EMMC_BASE + 0xf0)
#define SLOTISR_VER    (int*)(EMMC_BASE + 0xfc)

// CMDTM
#define TM_BLKCNT_EN         1<<1
#define TM_AUTO_CMD_12       1<<2
#define TM_AUTO_CMD_23       1<<3
#define TM_DAT_CARD_TO_HOST  1<<4
#define TM_MULTI_BLOCK       1<<5
#define CMD_RSPNS_136        0b01<<16
#define CMD_RSPNS_48         0b10<<16
#define CMD_RSPNS_48BUSY     0b11<<16
#define CMD_CRCCHK_EN        1<<19
#define CMD_IXCHK_EN         1<<20
#define CMD_ISDATA           1<<21
#define CMD_SUSPEND          0b01<<22
#define CMD_RESUME           0b10<<22
#define CMD_ABORT            0b11<<22
#define CMD_INDEX(x)         x<<24

// CONTROL1
#define CLK_INTLEN           1<<0
#define CLK_STABLE           1<<1
#define CLK_EN               1<<2
#define CLK_GENSEL           1<<5
#define CLK_FREQ_MS2(x)      x<<6
#define CLK_FREQ8(x)         1<<(8+x)
#define DATA_TOUNIT(x)       x<<16
#define DATA_TOUNIT_DISABLE  0xf<<16
#define SRST_HC              1<<24
#define SRST_CMD             1<<25
#define SRST_DATA            1<<26

// Interrupts
#define IR_CMD_DONE   1<<0
#define IR_DATA_DONE  1<<1
#define IR_BLOCK_GAP  1<<2
#define IR_WRITE_RDY  1<<4
#define IR_READ_RDY   1<<5
#define IR_CARD       1<<8
#define IR_RETUNE     1<<12
#define IR_BOOTACK    1<<13
#define IR_ENDBOOT    1<<14
#define IR_ERR        1<<15
#define IR_CTO_ERR    1<<16
#define IR_CCRC_ERR   1<<17
#define IR_CEND_ERR   1<<18
#define IR_CBAD_ERR   1<<19
#define IR_DTO_ERR    1<<20
#define IR_DCRC_ERR   1<<21
#define IR_DEND_ERR   1<<22
#define IR_ACMD_ERR   1<<24
#define IR_ALL        0x17ff137


// Commands
#define CMD_GO_IDLE_STATE      0
#define CMD_ALL_SEND_CID       2
#define CMD_SEND_RELATIVE_ADDR 3
#define CMD_SEND_IF_COND       8
#define CMD_SEND_CSD           9
#define CMD_SEND_OP_COND       41
#define CMD_APP                55

#define UNSTUFF_BITS(base,start,size) \
	({ \
		const int __mask = (1 << (size)) - 1;            \
		const int __shft = ((start) & 31);               \
		int __res = (base[(start)/32] >> __shft);         \
		if(__shft + (size) > 32)                         \
			__res |= base[(start)/32+1] << (32 - __shft); \
		__res & __mask;                                  \
	})

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


#include "term.h"
int sd_init(struct terminfo *dbg_out);
void sd_parse_csd(int raw[4], struct csd *csd);

#endif
