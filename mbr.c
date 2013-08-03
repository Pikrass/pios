#include "mbr.h"

#include "term.h"

void parse_partition_table(struct mbr *mbr, struct partition parts[4]) {
	for(int i=0 ; i<4 ; ++i) {
		struct mbr_part_entry *part = &mbr->partitions[i];

		if(part->lba_first != 0) {
			parts[i].start = part->lba_first;
			parts[i].size = part->nb_sectors;
			parts[i].type = part->type;
		}
		else {
			parts[i].size = 0;
		}
	}
}
