// SPDX-License-Identifier: GPL-2.0-or-later
/*
 * JXL minimal board for U-Boot SPL learning.
 */

#include <cpu_func.h>
#include <init.h>
#include <spl.h>
#include <asm/armv8/mmu.h>
#include <asm/global_data.h>
#include <asm/io.h>
#include <asm/u-boot.h>

DECLARE_GLOBAL_DATA_PTR;

/*
 * MMU memory map for the cache/MMU driver (cache_v8.c).
 *   - DRAM region: normal memory, cacheable.
 *   - MMIO region (PL011 UART and any future peripherals): device-nGnRnE.
 *   - Terminated by a zero-size entry.
 */
static struct mm_region jxl_mem_map[] = {
	{
		/* DRAM */
		.virt = 0x40000000UL,
		.phys = 0x40000000UL,
		.size = 0x7f000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE,
	}, {
		/* Peripherals: GIC, MMCI and PL011 live in this window */
		.virt = 0x08000000UL,
		.phys = 0x08000000UL,
		.size = 0x03000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN,
	}, {
		/* SRAM for early boot code */
		.virt = 0x00000000UL,
		.phys = 0x00000000UL,
		.size = 0x00010000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_NORMAL) |
			 PTE_BLOCK_INNER_SHARE,
	}, {
		/* NOR flash is MMIO-like for CFI command sequences */
		.virt = 0x04000000UL,
		.phys = 0x04000000UL,
		.size = 0x01000000UL,
		.attrs = PTE_BLOCK_MEMTYPE(MT_DEVICE_NGNRNE) |
			 PTE_BLOCK_NON_SHARE |
			 PTE_BLOCK_PXN | PTE_BLOCK_UXN,
	}, {
		/* terminator */
		0,
	}
};

struct mm_region *mem_map = jxl_mem_map;

#define __W "w"

u8 flash_read8(void *addr)
{
	u8 ret;

	asm("ldrb %" __W "0, %1" : "=r"(ret) : "m"(*(u8 *)addr));
	return ret;
}

u16 flash_read16(void *addr)
{
	u16 ret;

	asm("ldrh %" __W "0, %1" : "=r"(ret) : "m"(*(u16 *)addr));
	return ret;
}

u32 flash_read32(void *addr)
{
	u32 ret;

	asm("ldr %" __W "0, %1" : "=r"(ret) : "m"(*(u32 *)addr));
	return ret;
}

void flash_write8(u8 value, void *addr)
{
	asm("strb %" __W "1, %0" : "=m"(*(u8 *)addr) : "r"(value));
}

void flash_write16(u16 value, void *addr)
{
	asm("strh %" __W "1, %0" : "=m"(*(u16 *)addr) : "r"(value));
}

void flash_write32(u32 value, void *addr)
{
	asm("str %" __W "1, %0" : "=m"(*(u32 *)addr) : "r"(value));
}

int board_init(void)
{
	return 0;
}

int dram_init(void)
{
	gd->ram_size = CFG_SYS_SDRAM_SIZE;
	return 0;
}

int dram_init_banksize(void)
{
	gd->bd->bi_dram[0].start = CFG_SYS_SDRAM_BASE;
	gd->bd->bi_dram[0].size = CFG_SYS_SDRAM_SIZE;
	return 0;
}

void reset_cpu(void)
{
	/* No sysreset controller — just spin. User can kill QEMU. */
	while (1)
		asm volatile ("wfi");
}

#ifdef CONFIG_SPL_BUILD
u32 spl_boot_device(void)
{
	return BOOT_DEVICE_NOR;
}

/*
 * Place the FIT load buffer at 0x42000000 (kernel scratch in DRAM) instead
 * of the default CONFIG_TEXT_BASE = 0x40080000. With CONFIG_TEXT_BASE the
 * FIT and U-Boot proper share the same address range, so once SPL loads
 * U-Boot proper it overwrites the in-memory FIT and subsequent
 * /images/<loadable> lookups (and /fit-images recording) read garbage.
 * That broke BL32 entry hand-off to BL31 in the optee chain.
 */
void *board_spl_fit_buffer_addr(ulong fit_size, int sectors, int bl_len)
{
	return (void *)0x42000000UL;
}
#endif
