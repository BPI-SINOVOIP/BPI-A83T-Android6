# generic gpio support: platform drivers, dedicated expander chips, etc

ccflags-$(CONFIG_DEBUG_GPIO)	+= -DDEBUG

obj-$(CONFIG_GPIOLIB)		+= gpiolib.o devres.o

# Device drivers. Generally keep list sorted alphabetically
obj-$(CONFIG_GPIO_GENERIC)	+= gpio-generic.o

obj-$(CONFIG_GPIO_74X164)	+= gpio-74x164.o
obj-$(CONFIG_GPIO_AB8500)	+= gpio-ab8500.o
obj-$(CONFIG_GPIO_ADP5520)	+= gpio-adp5520.o
obj-$(CONFIG_GPIO_ADP5588)	+= gpio-adp5588.o
obj-$(CONFIG_GPIO_BT8XX)	+= gpio-bt8xx.o
obj-$(CONFIG_GPIO_CS5535)	+= gpio-cs5535.o
obj-$(CONFIG_GPIO_DA9052)	+= gpio-da9052.o
obj-$(CONFIG_ARCH_DAVINCI)	+= gpio-davinci.o
obj-$(CONFIG_GPIO_EP93XX)	+= gpio-ep93xx.o
obj-$(CONFIG_GPIO_GE_FPGA)	+= gpio-ge.o
obj-$(CONFIG_GPIO_IT8761E)	+= gpio-it8761e.o
obj-$(CONFIG_GPIO_JANZ_TTL)	+= gpio-janz-ttl.o
obj-$(CONFIG_ARCH_KS8695)	+= gpio-ks8695.o
obj-$(CONFIG_GPIO_LANGWELL)	+= gpio-langwell.o
obj-$(CONFIG_ARCH_LPC32XX)	+= gpio-lpc32xx.o
obj-$(CONFIG_GPIO_MAX730X)	+= gpio-max730x.o
obj-$(CONFIG_GPIO_MAX7300)	+= gpio-max7300.o
obj-$(CONFIG_GPIO_MAX7301)	+= gpio-max7301.o
obj-$(CONFIG_GPIO_MAX732X)	+= gpio-max732x.o
obj-$(CONFIG_GPIO_MC33880)	+= gpio-mc33880.o
obj-$(CONFIG_GPIO_MC9S08DZ60)	+= gpio-mc9s08dz60.o
obj-$(CONFIG_GPIO_MCP23S08)	+= gpio-mcp23s08.o
obj-$(CONFIG_GPIO_ML_IOH)	+= gpio-ml-ioh.o
obj-$(CONFIG_GPIO_MPC5200)	+= gpio-mpc5200.o
obj-$(CONFIG_GPIO_MPC8XXX)	+= gpio-mpc8xxx.o
obj-$(CONFIG_GPIO_MSM_V1)	+= gpio-msm-v1.o
obj-$(CONFIG_GPIO_MSM_V2)	+= gpio-msm-v2.o
obj-$(CONFIG_GPIO_MXC)		+= gpio-mxc.o
obj-$(CONFIG_GPIO_MXS)		+= gpio-mxs.o
obj-$(CONFIG_PLAT_NOMADIK)	+= gpio-nomadik.o
obj-$(CONFIG_ARCH_OMAP)		+= gpio-omap.o
obj-$(CONFIG_GPIO_PCA953X)	+= gpio-pca953x.o
obj-$(CONFIG_GPIO_PCF857X)	+= gpio-pcf857x.o
obj-$(CONFIG_GPIO_PCH)		+= gpio-pch.o
obj-$(CONFIG_GPIO_PL061)	+= gpio-pl061.o
obj-$(CONFIG_GPIO_PXA)		+= gpio-pxa.o
obj-$(CONFIG_GPIO_RDC321X)	+= gpio-rdc321x.o
obj-$(CONFIG_PLAT_SAMSUNG)	+= gpio-samsung.o
obj-$(CONFIG_ARCH_SA1100)	+= gpio-sa1100.o
obj-$(CONFIG_GPIO_SCH)		+= gpio-sch.o
obj-$(CONFIG_GPIO_SODAVILLE)	+= gpio-sodaville.o
obj-$(CONFIG_GPIO_STMPE)	+= gpio-stmpe.o
obj-$(CONFIG_GPIO_SX150X)	+= gpio-sx150x.o
obj-$(CONFIG_GPIO_TC3589X)	+= gpio-tc3589x.o
obj-$(CONFIG_ARCH_TEGRA)	+= gpio-tegra.o
obj-$(CONFIG_GPIO_TIMBERDALE)	+= gpio-timberdale.o
obj-$(CONFIG_ARCH_DAVINCI_TNETV107X) += gpio-tnetv107x.o
obj-$(CONFIG_GPIO_TPS65910)	+= gpio-tps65910.o
obj-$(CONFIG_GPIO_TPS65912)	+= gpio-tps65912.o
obj-$(CONFIG_GPIO_TWL4030)	+= gpio-twl4030.o
obj-$(CONFIG_GPIO_UCB1400)	+= gpio-ucb1400.o
obj-$(CONFIG_GPIO_VR41XX)	+= gpio-vr41xx.o
obj-$(CONFIG_GPIO_VX855)	+= gpio-vx855.o
obj-$(CONFIG_GPIO_WM831X)	+= gpio-wm831x.o
obj-$(CONFIG_GPIO_WM8350)	+= gpio-wm8350.o
obj-$(CONFIG_GPIO_WM8994)	+= gpio-wm8994.o
obj-$(CONFIG_GPIO_XILINX)	+= gpio-xilinx.o
obj-m	+= gpio-sunxi.o   #Justin Porting 20171206 Start

