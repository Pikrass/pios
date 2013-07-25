extern void *__etext, *__data, *__edata;

void init() __attribute__((noreturn,naked));
void main() __attribute__((noreturn));

void init() {
	__asm("mov sp, #0x100000");

	unsigned int *t0 = (unsigned int*)(((unsigned int)&__edata & ~0xc0003fff) + 0x4000);
	unsigned int *t1_000 = t0 + 4096,
	             *t1_c00 = t1_000 + 256,
				 *t1_eff = t1_c00 + 256,
				 *t1_f00 = t1_eff + 256;

	for(int i=0 ; i<4096 ; ++i) {
		t0[i] = 0;
	}

	// Map the I/0 peripherals to 0xf2000000
	for(int i=0 ; i<16 ; ++i) {
		t0[0xf20 + i] = (0x20000000 + i*0x100000) | 0x412;
	}

	// Install the second-level tables
	t0[0x000] = (unsigned int)t1_000 | 1;
	t0[0xc00] = (unsigned int)t1_c00 | 1;
	t0[0xeff] = (unsigned int)t1_eff | 1;
	t0[0xf00] = (unsigned int)t1_f00 | 1;

	// Zero out the second-level tables
	for(int i=0 ; i<0x100 ; ++i) {
		t1_000[i] = 0;
		t1_c00[i] = 0;
		t1_eff[i] = 0;
		t1_f00[i] = 0;
	}

	// Produce a 1:1 mapping for pages 0 (irpt/excpt vectors, ATAGS) and 8
	// (this init code)
	t1_000[0] = 0x2a;
	t1_000[8] = 0x821a;

	// Map the top table to 0xf0000000 - 0xf0003fff
	for(int i=0 ; i<4 ; i++) {
		t1_f00[i] = (unsigned int)((void*)t0 + i*0x1000) | 0x12;
	}

	// Map the second-level tables to 0xf0004000 - 0xf0004fff
	t1_f00[4] = (unsigned int)((void*)t1_f00) | 0x12;

	// Map c0000000 to 9000, for as many pages as needed up to __etext
	unsigned int last_text_page = ((unsigned int)&__etext & ~0xc0000000) >> 12;
	int page;
	for(page=0 ; page <= last_text_page ; ++page) {
		t1_c00[page] = (0x9000 + page*0x1000) | 0x21a;
	}

	// Map data pages
	unsigned int last_data_page = ((unsigned int)&__edata & ~0xc0000000) >> 12;
	for( ; page <= last_data_page ; ++page) {
		t1_c00[page] = (0x9000 + page*0x1000) | 0x21b;
	}

	// Allocate a page for the kernel stack
	t1_eff[0xff] = (unsigned int)((void*)t1_000 + 0x1000) | 0x21b;

	// Allocate two pages for the kernel initial heap
	for(int i=0 ; i < 2 ; ++i)
		t1_c00[page++] = (unsigned int)((void*)t1_000 + 0x2000 + i*0x1000) | 0x21b;

	// Setup domain 0
	__asm("mcr p15, 0, %[val], c3, c0, 0" : : [val] "r" (3));

	// Register our table
	__asm("mcr p15, 0, %[table], c2, c0, 0" : : [table] "r" (t0));
	__asm("mcr p15, 0, %[table], c2, c0, 1" : : [table] "r" (t0));
	__asm("mcr p15, 0, %[n], c2, c0, 2" : : [n] "r" (0));

	// Activate the MMU
	int mmu_reg;
	__asm("mrc p15, 0, %[reg], c1, c0, 0" : [reg] "=r" (mmu_reg));
	mmu_reg |= 0x800001;
	__asm("mcr p15, 0, %[reg], c1, c0, 0" : : [reg] "r" (mmu_reg));

	__asm("mov sp, #0xf0000000");
	main();
}
