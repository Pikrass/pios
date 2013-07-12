#include "sd.h"

#include "term.h"

void idle(int ms) {
	// These are not real milliseconds :)
	ms *= 1000;
	while(ms > 0)
		--ms;
}

int sd_send_command(int cmd, int flags, int arg) {
	*INTERRUPT = IR_ALL;
	*ARG1 = arg;
	*CMDTM = CMD_INDEX(cmd) | flags;
	while(*INTERRUPT == 0);
	return *INTERRUPT & IR_ERR;
}

int sd_init(struct sd_card *card) {
	int resp, powerup;
	int raw_csd[4];

	// Reset
	*CONTROL0 = 0;
	*CONTROL1 = SRST_HC;
	*CONTROL2 = 0;
	while(*CONTROL1 & SRST_HC);

	// Clock config
	*CONTROL1 = CLK_INTLEN | CLK_GENSEL | CLK_FREQ8(7) | DATA_TOUNIT_DISABLE;
	// Wait for internal clock to stabilize
	while((*CONTROL1 & CLK_STABLE) == 0);

	// Activate SD clock
	*CONTROL1 |= CLK_EN,

	// Enable interrupts
	*IRPT_MASK = IR_ALL;

	// Send CMD0
	if(sd_send_command(CMD_GO_IDLE_STATE, 0, 0))
		return 1;

	// Send CMD8
	if(sd_send_command(CMD_SEND_IF_COND, CMD_RSPNS_48BUSY, 0x1aa))
		return 2;
	if(*RESP0 != 0x1aa) {
		return 2;
	}

	// Send ACMD41 until powerup bit is received
	powerup = 0;
	while(!powerup) {
		*INTERRUPT = IR_ALL;
		*ARG1 = 0;
		*CMDTM = CMD_INDEX(CMD_APP) | TM_DAT_CARD_TO_HOST;
		while((*INTERRUPT & IR_CMD_DONE) == 0);

		idle(500);
		if(sd_send_command(CMD_SEND_OP_COND, CMD_RSPNS_48, 0x40ff0000))
			return 3;

		powerup = (*RESP0 & 0x80000000);
	}
	card->type = *RESP0 & 40000000;

	// Send CMD2 to get the CID
	if(sd_send_command(CMD_ALL_SEND_CID, CMD_RSPNS_136, 0))
		return 4;

	// Send CMD3 to get the RCA
	if(sd_send_command(CMD_SEND_RELATIVE_ADDR, CMD_RSPNS_48, 0))
		return 5;

	resp = *RESP0;
	card->rca = resp & 0xffff0000;

	// Send CMD9 to get the CSD
	if(sd_send_command(CMD_SEND_CSD, CMD_RSPNS_136, card->rca))
		return 6;

	raw_csd[0] = *RESP0;
	raw_csd[1] = *RESP1;
	raw_csd[2] = *RESP2;
	raw_csd[3] = *RESP3;
	sd_parse_csd(raw_csd, &card->csd);

	// Send CMD7 to select the card
	*INTERRUPT = IR_ALL;
	*ARG1 = card->rca;
	*CMDTM = CMD_INDEX(CMD_SELECT_CARD) | CMD_RSPNS_48;
	while(*INTERRUPT == 0);
	if(*INTERRUPT & IR_ERR) {
		return 7;
	}

	return 0;
}

void sd_parse_csd(int raw[4], struct csd *csd) {
	csd->file_format        = UNSTUFF_BITS(raw, 2, 2);
	csd->tmp_write_protect  = UNSTUFF_BITS(raw, 4, 1);
	csd->perm_write_protect = UNSTUFF_BITS(raw, 5, 1);
	csd->copy               = UNSTUFF_BITS(raw, 6, 1);
	csd->file_format_grp    = UNSTUFF_BITS(raw, 7, 1);
	csd->write_bl_partial   = UNSTUFF_BITS(raw, 13, 1);
	csd->write_bl_len       = UNSTUFF_BITS(raw, 14, 4);
	csd->r2w_factor         = UNSTUFF_BITS(raw, 18, 3);
	csd->wp_grp_enable      = UNSTUFF_BITS(raw, 23, 1);
	csd->wp_grp_size        = UNSTUFF_BITS(raw, 24, 7);
	csd->sector_size        = UNSTUFF_BITS(raw, 31, 7);
	csd->erase_blk_en       = UNSTUFF_BITS(raw, 38, 1);
	csd->c_size_mult        = UNSTUFF_BITS(raw, 39, 3);
	csd->vdd_w_curr_max     = UNSTUFF_BITS(raw, 42, 3);
	csd->vdd_w_curr_min     = UNSTUFF_BITS(raw, 45, 3);
	csd->vdd_r_curr_max     = UNSTUFF_BITS(raw, 48, 3);
	csd->vdd_r_curr_min     = UNSTUFF_BITS(raw, 51, 3);
	csd->c_size             = UNSTUFF_BITS(raw, 54, 12);
	csd->dsr_imp            = UNSTUFF_BITS(raw, 68, 1);
	csd->read_blk_misalign  = UNSTUFF_BITS(raw, 69, 1);
	csd->write_blk_misalign = UNSTUFF_BITS(raw, 70, 1);
	csd->read_bl_partial    = UNSTUFF_BITS(raw, 71, 1);
	csd->read_bl_len        = UNSTUFF_BITS(raw, 72, 4);
	csd->ccc                = UNSTUFF_BITS(raw, 76, 12);
	csd->tran_speed         = UNSTUFF_BITS(raw, 88, 8);
	csd->nsac               = UNSTUFF_BITS(raw, 96, 8);
	csd->taac               = UNSTUFF_BITS(raw, 104, 8);
	csd->csd_structure      = UNSTUFF_BITS(raw, 118, 2);
}
