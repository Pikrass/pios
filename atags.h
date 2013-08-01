#ifndef ATAGS_H
#define ATAGS_H

#define ATAG_NONE    0x0
#define ATAG_CORE    0x54410001
#define ATAG_MEM     0x54410002
#define ATAG_CMDLINE 0x54410009

struct atag_core {
	unsigned int flags, pagesize, rootdev;
};

struct atag_mem {
	unsigned int size, start;
};

struct atag_cmdline {
	char str;
};

struct atag {
	unsigned int size;
	unsigned int type;
	union {
		struct atag_core core;
		struct atag_mem mem;
		struct atag_cmdline cmdline;
	};
};

unsigned int atags_get_mem();

#endif
