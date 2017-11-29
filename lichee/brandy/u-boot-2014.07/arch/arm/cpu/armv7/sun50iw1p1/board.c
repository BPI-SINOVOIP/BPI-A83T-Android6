/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	 See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 * MA 02111-1307 USA
 */

#include <common.h>
#include <asm/io.h>
#include <asm/arch/timer.h>
#include <asm/arch/ccmu.h>
#include <asm/arch/clock.h>
#include <asm/arch/sid.h>
#include <asm/arch/platform.h>
#include <asm/arch/sys_proto.h>
#include <asm/arch/key.h>
#include <asm/arch/dma.h>

#include <boot_type.h>
#include <sys_partition.h>
#include <sys_config.h>
#include <power/sunxi/pmu.h>
#include <smc.h>
#include <sunxi_board.h>
#include <fdt_support.h>



/* The sunxi internal brom will try to loader external bootloader
 * from mmc0, nannd flash, mmc2.
 * We check where we boot from by checking the config
 * of the gpio pin.
 */
DECLARE_GLOBAL_DATA_PTR;

extern void power_limit_init(void);

/* do some early init */
void s_init(void)
{
	watchdog_disable();
}

void reset_cpu(ulong addr)
{
	watchdog_enable();
	while(1);
}


void v7_outer_cache_enable(void)
{
	return ;
}

void v7_outer_cache_inval_all(void)
{
	return ;
}

void v7_outer_cache_flush_range(u32 start, u32 stop)
{
	return ;
}


void enable_caches(void)
{
	icache_enable();
	dcache_enable();
}

void disable_caches(void)
{
	icache_disable();
	dcache_disable();
}

int display_inner(void)
{
	printf("version: %s\n", uboot_spare_head.boot_head.version);

	return 0;
}

int script_init(void)
{
#if 0
	uint offset, length;
	char *addr;

	offset = uboot_spare_head.boot_head.uboot_length;
	length = uboot_spare_head.boot_head.length - uboot_spare_head.boot_head.uboot_length;

	addr   = (char *)CONFIG_SYS_TEXT_BASE + offset;

	if(length == 0)
	{
		script_parser_init(NULL);
		return 0;
	}

	if (!(gd->flags & GD_FLG_RELOC))
	{
		script_parser_init((char *)addr);
	}
	else
	{
		script_parser_init((char *)(gd->script_reloc_buf));
	}

	#if 0  //just for test
	{
	int dcdc_vol = 0;
	script_parser_fetch("power_sply", "dcdc2_vol", &dcdc_vol, 1);
	printf("----script test ---- dcdc2_vol = %d\n",dcdc_vol);
	}
	#endif

#endif

	return 0;
}

extern int sunxi_arisc_probe(void);
int power_source_init(void)
{
	int pll_cpux;
	int cpu_vol;
	uint32_t dcdc_vol = 0;
	int axp_exist = 0;
	int nodeoffset;

	sunxi_arisc_probe();
	//PMU_SUPPLY_DCDC2 is for cpua
	nodeoffset =  fdt_path_offset(working_fdt,FDT_PATH_POWER_SPLY);
	if(nodeoffset >=0)
	{
		fdt_getprop_u32(working_fdt, nodeoffset, "dcdc2_vol", &dcdc_vol);
	}
	if(!dcdc_vol)
	{
		cpu_vol = 900;
	}
	else
	{
		cpu_vol = dcdc_vol%10000;
	}
	axp_exist =  axp_probe();
	if(axp_exist)
	{
		axp_probe_factory_mode();
		if(!axp_probe_power_supply_condition())
		{
			//PMU_SUPPLY_DCDC2 is for cpua
			if(!axp_set_supply_status(0, PMU_SUPPLY_DCDC2, cpu_vol, -1))
			{
				tick_printf("PMU: dcdc2 %d\n", cpu_vol);
				sunxi_clock_set_corepll(uboot_spare_head.boot_data.run_clock);
			}
			else
			{
				printf("axp_set_dcdc2 fail\n");
			}
		}
		else
		{
			printf("axp_probe_power_supply_condition error\n");
		}
	}
	else
	{
		printf("axp_probe error\n");
	}

	pll_cpux = sunxi_clock_get_corepll();
	tick_printf("PMU: cpux %d Mhz,AXI=%d Mhz\n", pll_cpux,sunxi_clock_get_axi());
	printf("PLL6=%d Mhz,AHB1=%d Mhz, APB1=%dMhz AHB2=%dMhz MBus=%dMhz\n", sunxi_clock_get_pll6(),
		sunxi_clock_get_ahb(),
		sunxi_clock_get_apb(),
		sunxi_clock_get_ahb2(),
		sunxi_clock_get_mbus());

	if(axp_exist)
	{
		axp_set_charge_vol_limit();
		axp_set_all_limit();
		axp_set_hardware_poweron_vol();
		axp_set_power_supply_output();
		//power_config_gpio_bias();
		power_limit_init();
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void sunxi_set_fel_flag(void)
{
	*((volatile unsigned int *)(SUNXI_RUN_EFEX_ADDR)) = SUNXI_RUN_EFEX_FLAG;
	asm volatile("DMB SY");
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void sunxi_clear_fel_flag(void)
{
	*((volatile unsigned int *)(SUNXI_RUN_EFEX_ADDR)) = 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
void sunxi_set_rtc_flag(u8 flag)
{
	*((volatile unsigned int *)(SUNXI_RUN_EFEX_ADDR)) = flag;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_probe_securemode(void)
{
	uint reg_val;

	writel(0xffffffff, SUNXI_SRAM_A2_BASE);
	reg_val = readl(SUNXI_SRAM_A2_BASE);
	printf("reg=0x%x\n", reg_val);
	if(!reg_val)  //all 0, secure is enable, normal mode
	{
		//sbrom  set  secureos_exist flag,
		//1: secure os exist 0: secure os not exist
		if(uboot_spare_head.boot_data.secureos_exist==1)
		{
			gd->securemode = SUNXI_SECURE_MODE_WITH_SECUREOS;
			printf("secure mode: with secureos\n");
		}
		else
		{
			gd->securemode = SUNXI_SECURE_MODE_NO_SECUREOS;
			printf("secure mode: no secureos\n");
		}
		gd->bootfile_mode = SUNXI_BOOT_FILE_TOC;
	}
	else
	{
		//boot0  set  secureos_exist flag,
		//1: secure monitor exist 0: secure monitor  not exist
		uint32_t burn_secure_mode=0;
		int nodeoffset;

		gd->securemode = SUNXI_NORMAL_MODE;
		gd->bootfile_mode = SUNXI_BOOT_FILE_PKG;
		printf("normal mode: with secure monitor\n");

		nodeoffset =  fdt_path_offset(working_fdt,FDT_PATH_TARGET);
		if(nodeoffset >=0)
		{
			fdt_getprop_u32(working_fdt, nodeoffset, "burn_secure_mode", &burn_secure_mode);
		}
		if(burn_secure_mode != 1)
		{
			return 0;
		}
		gd->bootfile_mode = SUNXI_BOOT_FILE_TOC;
	}
	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_set_secure_mode(void)
{
	int mode;

	if((gd->securemode == SUNXI_NORMAL_MODE) && (gd->bootfile_mode = SUNXI_BOOT_FILE_TOC))
	{
		mode = sid_probe_security_mode();
		if(!mode)
		{
			sid_set_security_mode();
			gd->bootfile_mode = SUNXI_BOOT_FILE_TOC;
		}
	}

	return 0;
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
int sunxi_get_securemode(void)
{
	return gd->securemode;
}

int sunxi_probe_secure_monitor(void)
{
	return uboot_spare_head.boot_data.secureos_exist == SUNXI_SECURE_MODE_USE_SEC_MONITOR?1:0;
}



void sunxi_board_close_source(void)
{
	//timer_exit should be call after sunxi_flash,because delay function
	sunxi_flash_exit(1);
	sunxi_sprite_exit(1);
#ifdef CONFIG_SUNXI_DMA
	sunxi_dma_exit();
#endif
	timer_exit();
	sunxi_key_exit();
	disable_interrupts();
	interrupt_exit();
	return ;
}

int sunxi_board_restart(int next_mode)
{
	if(!next_mode)
	{
		next_mode = PMU_PRE_SYS_MODE;
	}
	printf("set next mode %d\n", next_mode);
	axp_set_next_poweron_status(next_mode);

#ifdef CONFIG_SUNXI_DISPLAY
	board_display_set_exit_mode(0);
	drv_disp_exit();
#endif
	sunxi_board_close_source();
	reset_cpu(0);

	return 0;
}

int sunxi_board_shutdown(void)
{


	printf("set next system normal\n");
	axp_set_next_poweron_status(0x0);

#ifdef CONFIG_SUNXI_DISPLAY
	board_display_set_exit_mode(0);
	drv_disp_exit();
#endif
	sunxi_flash_exit(1);
	sunxi_sprite_exit(1);
	disable_interrupts();
	interrupt_exit();

	tick_printf("power off\n");
	axp_set_hardware_poweroff_vol();
	axp_set_power_off();

	return 0;

}

void sunxi_flush_allcaches(void)
{
	icache_disable();
	flush_dcache_all();
	dcache_disable();
}


int sunxi_board_run_fel(void)
{
	sunxi_set_fel_flag();
	printf("set next system status\n");
	axp_set_next_poweron_status(PMU_PRE_SYS_MODE);
#ifdef CONFIG_SUNXI_DISPLAY
	board_display_set_exit_mode(0);
	drv_disp_exit();
#endif
	printf("sunxi_board_close_source\n");
	sunxi_board_close_source();
	sunxi_flush_allcaches();
	printf("reset cpu\n");
	reset_cpu(0);
	return 0;
}

int sunxi_board_run_fel_eraly(void)
{
	sunxi_set_fel_flag();
	printf("set next system status\n");
	axp_set_next_poweron_status(PMU_PRE_SYS_MODE);
	timer_exit();
	sunxi_key_exit();
	printf("reset cpu\n");
	reset_cpu(0);

	return 0;
}


