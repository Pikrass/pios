#include "sd.h"

#include "term.h"

void idle(int ms) {
	ms *= 1000;
	while(ms > 0)
		--ms;
}

int sd_init(struct terminfo *term) {
	int tmp, resp, powerup;
	int card_type, rca, csd[4];

	term_printf(term, "SD_INIT:");

	// Reset
	*CONTROL0 = 0;
	*CONTROL1 = SRST_HC;
	*CONTROL2 = 0;
	while(*CONTROL1 & SRST_HC);
	term_printf(term, " reset");

	// Clock config
	*CONTROL1 = CLK_INTLEN | CLK_GENSEL | CLK_FREQ8(7) | DATA_TOUNIT_DISABLE;
	// Wait for internal clock to stabilize
	while((*CONTROL1 & CLK_STABLE) == 0);
	term_printf(term, " clock_stable");

	// Activate SD clock
	*CONTROL1 |= CLK_EN,

	// Enable interrupts
	*IRPT_MASK = IR_ALL;

	// Send CMD0
	*ARG1 = 0;
	*CMDTM = CMD_INDEX(CMD_GO_IDLE_STATE);// | TM_DAT_CARD_TO_HOST;
	while(*INTERRUPT == 0);
	term_printf(term, " cmd0"); //TODO: check for errors
	*INTERRUPT = IR_ALL;

	// Send CMD8
	*ARG1 = 0x1aa; // voltage 1, check_pattern 10101010
	*CMDTM = CMD_INDEX(CMD_SEND_IF_COND) | CMD_RSPNS_48BUSY;

	while((*INTERRUPT & IR_CMD_DONE) == 0);

	tmp = *INTERRUPT;
	resp = *RESP0;
	*INTERRUPT = IR_ALL;

	if((tmp & IR_CMD_DONE) > 0 && (tmp & IR_ERR) == 0 && resp == 0x1aa) {
		term_printf(term, " cmd8");
	}
	else {
		term_printf(term, " cmd8_err(0x%x, 0x%x)", tmp, resp);
		return 0;
	}

	// Send ACMD41 until powerup bit is received
	powerup = 0;
	while(!powerup) {
		*ARG1 = 0;
		*CMDTM = CMD_INDEX(CMD_APP) | TM_DAT_CARD_TO_HOST;
		while((*INTERRUPT & IR_CMD_DONE) == 0);
		*INTERRUPT = IR_ALL;

		idle(500);
		*ARG1 = 0x40ff0000;
		*CMDTM = CMD_INDEX(CMD_SEND_OP_COND) | CMD_RSPNS_48;
		while(*INTERRUPT == 0);

		if(*INTERRUPT & IR_ERR) {
			term_printf(term, " acmd41_err irpt=0x%x", *INTERRUPT);
			return 0;
		}

		powerup = (*RESP0 & 0x80000000);
	}
	term_printf(term, " acmd41");
	card_type = *RESP0 & 40000000;
	if(card_type)
		term_printf(term, " scsd");
	else
		term_printf(term, " hcsd");

	// Send CMD2 to get the CID
	*INTERRUPT = IR_ALL;
	*ARG1 = 0;
	*CMDTM = CMD_INDEX(CMD_ALL_SEND_CID) | CMD_RSPNS_136;
	while(*INTERRUPT == 0);
	if(*INTERRUPT & IR_ERR) {
		term_printf(term, " cmd2_err");
		return 0;
	}
	term_printf(term, " cmd2");

	// Send CMD3 to get the RCA
	*INTERRUPT = IR_ALL;
	*ARG1 = 0;
	*CMDTM = CMD_INDEX(CMD_SEND_RELATIVE_ADDR) | CMD_RSPNS_48;
	while(*INTERRUPT == 0);
	if(*INTERRUPT & IR_ERR) {
		term_printf(term, " cmd3_err");
		return 0;
	}
	resp = *RESP0;
	term_printf(term, " cmd3 resp=0x%x", resp);

	rca = resp & 0xffff0000;

	*INTERRUPT = IR_ALL;
	*ARG1 = rca;
	*CMDTM = CMD_INDEX(CMD_SEND_CSD) | CMD_RSPNS_136;
	while(*INTERRUPT == 0);
	if(*INTERRUPT & IR_ERR) {
		term_printf(term, " cmd9_err");
		return 0;
	}
	csd[0] = *RESP0;
	csd[1] = *RESP1;
	csd[2] = *RESP2;
	csd[3] = *RESP3;
	term_printf(term, " cmd9 csd=0x%x 0x%x 0x%x 0x%x", csd[0], csd[1], csd[2], csd[3]);

	return 1;
}
