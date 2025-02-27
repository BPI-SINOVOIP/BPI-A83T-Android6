#include "panels.h"

struct sunxi_lcd_drv g_lcd_drv;

extern __lcd_panel_t default_panel;
extern __lcd_panel_t lt070me05000_panel;
extern __lcd_panel_t t27p06_panel;
extern __lcd_panel_t S070WV20_MIPI_RGB_panel;
extern __lcd_panel_t LHR050H41_MIPI_RGB_panel;
extern __lcd_panel_t CHR050H129_MIPI_RGB_panel;

__lcd_panel_t* panel_array[] = {
	&default_panel,
	&lt070me05000_panel,
	&t27p06_panel,
	&S070WV20_MIPI_RGB_panel,
        &LHR050H41_MIPI_RGB_panel,
        &CHR050H129_MIPI_RGB_panel,
	/* add new panel below */

	NULL,
};

static void lcd_set_panel_funs(void)
{
	int i;

	for(i=0; panel_array[i] != NULL; i++) {
		sunxi_lcd_set_panel_funs(panel_array[i]->name, &panel_array[i]->func);
	}

	return ;
}

int lcd_init(void)
{
	sunxi_disp_get_source_ops(&g_lcd_drv.src_ops);
	lcd_set_panel_funs();

	return 0;
}
