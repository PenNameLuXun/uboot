/* SPDX-License-Identifier: GPL-2.0-or-later */
/*
 * Configuration for JXL minimal board.
 */

#ifndef __JXL_H
#define __JXL_H

#define CFG_SYS_SDRAM_BASE		0x40000000
#define CFG_SYS_SDRAM_SIZE		0x08000000	/* 128 MiB */

#define CFG_SYS_FLASH_BASE		0x04000000
#define CFG_SYS_FLASH_BANKS_LIST	{ CFG_SYS_FLASH_BASE }

#define CFG_SYS_INIT_SP_ADDR		(CFG_SYS_SDRAM_BASE + 0x200000)

/*
 * jxl_defconfig enables the NOR flash command set, but env/flash.c checks the
 * preprocessor symbol directly before wiring up its save callback.
 */
#ifndef CONFIG_CMD_FLASH
#define CONFIG_CMD_FLASH		1
#endif

#endif /* __JXL_H */
