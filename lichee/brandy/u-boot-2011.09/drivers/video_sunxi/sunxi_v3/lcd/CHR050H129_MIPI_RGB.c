#include "panels.h"
#include "CHR050H129_MIPI_RGB.h"

#define panel_rst(v)    (sunxi_lcd_gpio_set_value(0, 0, v))
#define panel_pwr(v)    (sunxi_lcd_gpio_set_value(0, 1, v))

static void LCD_power_on(u32 sel);
static void LCD_power_off(u32 sel);
static void LCD_bl_open(u32 sel);
static void LCD_bl_close(u32 sel);

static void LCD_panel_init(u32 sel);
static void LCD_panel_exit(u32 sel);

extern void sunxi_lcd_tcon_enable(u32 screen_id);
extern void sunxi_lcd_tcon_disable(u32 screen_id);
extern void sunxi_lcd_power_enable(u32 screen_id, u32 pwr_id);
extern void sunxi_lcd_power_disable(u32 screen_id, u32 pwr_id);
extern void sunxi_lcd_backlight_enable(u32 screen_id);
extern void sunxi_lcd_backlight_disable(u32 screen_id);
extern s32 sunxi_lcd_pwm_enable(u32 pwm_channel);
extern s32 sunxi_lcd_pwm_disable(u32 pwm_channel);
extern s32 sunxi_lcd_pin_cfg(u32 screen_id, u32 bon);
extern void LCD_OPEN_FUNC(u32 screen_id, LCD_FUNC func, u32 delay);
extern void LCD_CLOSE_FUNC(u32 screen_id, LCD_FUNC func, u32 delay);

static void LCD_cfg_panel_info(panel_extend_para * info)
{
    u32 i = 0, j=0;
    u32 items;
    u8 lcd_gamma_tbl[][2] =
    {
        //{input value, corrected value}
        {0, 0},
        {15, 15},
        {30, 30},
        {45, 45},
        {60, 60},
        {75, 75},
        {90, 90},
        {105, 105},
        {120, 120},
        {135, 135},
        {150, 150},
        {165, 165},
        {180, 180},
        {195, 195},
        {210, 210},
        {225, 225},
        {240, 240},
        {255, 255},
    };

    u32 lcd_cmap_tbl[2][3][4] = {
        {
            {LCD_CMAP_G0,LCD_CMAP_B1,LCD_CMAP_G2,LCD_CMAP_B3},
            {LCD_CMAP_B0,LCD_CMAP_R1,LCD_CMAP_B2,LCD_CMAP_R3},
            {LCD_CMAP_R0,LCD_CMAP_G1,LCD_CMAP_R2,LCD_CMAP_G3},
        },
        {
            {LCD_CMAP_B3,LCD_CMAP_G2,LCD_CMAP_B1,LCD_CMAP_G0},
            {LCD_CMAP_R3,LCD_CMAP_B2,LCD_CMAP_R1,LCD_CMAP_B0},
            {LCD_CMAP_G3,LCD_CMAP_R2,LCD_CMAP_G1,LCD_CMAP_R0},
        },
    };

    items = sizeof(lcd_gamma_tbl)/2;
    for(i=0; i<items-1; i++) {
        u32 num = lcd_gamma_tbl[i+1][0] - lcd_gamma_tbl[i][0];

        for(j=0; j<num; j++) {
            u32 value = 0;

            value = lcd_gamma_tbl[i][1] + ((lcd_gamma_tbl[i+1][1] - lcd_gamma_tbl[i][1]) * j)/num;
            info->lcd_gamma_tbl[lcd_gamma_tbl[i][0] + j] = (value<<16) + (value<<8) + value;
        }
    }
    info->lcd_gamma_tbl[255] = (lcd_gamma_tbl[items-1][1]<<16) + (lcd_gamma_tbl[items-1][1]<<8) + lcd_gamma_tbl[items-1][1];

    memcpy(info->lcd_cmap_tbl, lcd_cmap_tbl, sizeof(lcd_cmap_tbl));

}

static s32 LCD_open_flow(u32 sel)
{
    LCD_OPEN_FUNC(sel, LCD_power_on, 100);   //open lcd power, and delay 50ms
    LCD_OPEN_FUNC(sel, LCD_panel_init, 200);   //open lcd power, than delay 200ms
    LCD_OPEN_FUNC(sel, sunxi_lcd_tcon_enable, 200);     //open lcd controller, and delay 100ms
    LCD_OPEN_FUNC(sel, LCD_bl_open, 0);     //open lcd backlight, and delay 0ms

    return 0;
}

static s32 LCD_close_flow(u32 sel)
{
    LCD_CLOSE_FUNC(sel, LCD_bl_close, 0);       //close lcd backlight, and delay 0ms
    LCD_CLOSE_FUNC(sel, sunxi_lcd_tcon_disable, 0);         //close lcd controller, and delay 0ms
    LCD_CLOSE_FUNC(sel, LCD_panel_exit,	200);   //open lcd power, than delay 200ms
    LCD_CLOSE_FUNC(sel, LCD_power_off, 500);   //close lcd power, and delay 500ms

    return 0;
}

static void LCD_power_on(u32 sel)
{
    sunxi_lcd_power_enable(sel, 0);//config lcd_power pin to open lcd power0
    sunxi_lcd_pin_cfg(sel, 1);
}

static void LCD_power_off(u32 sel)
{
    sunxi_lcd_power_disable(sel, 0);//config lcd_power pin to close lcd power0
    sunxi_lcd_pin_cfg(sel, 0);
}

static void LCD_bl_open(u32 sel)
{
    sunxi_lcd_pwm_enable(sel);//open pwm module
    sunxi_lcd_backlight_enable(sel);//config lcd_bl_en pin to open lcd backlight
}

static void LCD_bl_close(u32 sel)
{
    sunxi_lcd_backlight_disable(sel);//config lcd_bl_en pin to close lcd backlight
    sunxi_lcd_pwm_disable(sel);//close pwm module
}

#define REGFLAG_END_OF_TABLE     0x102
#define REGFLAG_DELAY            0x101

struct lcd_setting_table {
    u16 cmd;
    u32 count;
    u8 para_list[64];
};

static struct lcd_setting_table lcm_initialization_setting[] = {
      {0xB9, 3, {0xFF,0x83,0x94}}, 
{0xBA,6,{0x63,0x03,0x68,0x6b,0xb2,0xc0}},                                         
{0xB1, 10, {0x50,0x12,0x72,0x09,0x33,0x54,0xB1,0x31,0x6B,0x2F}},                         
{0xB2,6,{0x00,0x80,0x64,0x0e,0x0d,0x2f}},         
       
                               
{0xB4,21,{0x73,0x74,0x73,0x74,0x73,0x74,0x01,0x0C,0x86,0x75, 
         0x00,0x3F,0x73,0x74,0x73,0x74,0x73,0x74,0x01,0x0C, 
                 0x86}},   
      
	// 0x75----0x66
                                       
{0xD3,33,{0x00,0x00,0x07,0x07,0x40,0x07,0x10,0x00,0x08,0x10, 
                 0x08,0x00,0x08,0x54,0x15,0x0e,0x05,0x0e,0x02,0x15, 
                 0x06,0x05,0x06,0x47,0x44,0x0a,0x0a,0x4b,0x10,0x07, 
                 0x07,0x0e,0x40}},                 
                       
{0xD5,44,{0x1a,0x1a,0x1b,0x1b,0x00,0x01,0x02,0x03,0x04,0x05, 
                 0x06,0x07,0x08,0x09,0x0a,0x0b,0x24,0x25,0x18,0x18, 
                 0x26,0x27,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, 
                 0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x20,0x21, 
                 0x18,0x18,0x18,0x18}}, 


{0xD6,44,{0x1a,0x1a,0x1b,0x1b,0x0b,0x0a,0x09,0x08,0x07,0x06, 
                 0x05,0x04,0x03,0x02,0x01,0x00,0x21,0x20,0x18,0x18, 
                 0x27,0x26,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18, 
                 0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x18,0x25,0x24, 
                 0x18,0x18,0x18,0x18}}, 
                                       
{0xE0,58,{0x00,0x0D,0x1B,0x22,0x25,0x2A,0x2F,0x2C,0x5A,0x6B, 
                 0x7A,0x77,0x7E,0x8E,0x92,0x95,0x9F,0x9E,0x99,0xa1, 
                 0xb0,0x57,0x55,0x5C,0x5F,0x5F,0x67,0x6F,0x7f,0x00, 
                 0x0D,0x1B,0x22,0x25,0x2A,0x2F,0x2C,0x5A,0x6B,0x7A, 
                 0x77,0x7E,0x8E,0x92,0x95,0x9F,0x9E,0x99,0xa1,0xb0, 
                 0x57,0x55,0x5C,0x5F,0x5F,0x67,0x6F,0x7f}},                                       
                                       
{0xC0,2,{0x1f,0x31}},                 
{0xCC,1,{0x0B}},                                 
{0xB6,2,{0x78,0x78}},                         
{0xD4,1,{0x02}}, 
{0xBD,1,{0x02}}, 
{0xD8,12,{0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF}}, 
{0xBD,1,{0x00}}, 
{0xBD,1,{0x01}}, 
{0xB1,1,{0x00}}, 
{0xBD,1,{0x00}}, 
{0xBF,7,{0x40,0x81,0x50,0x00,0x1A,0xFC,0x01}}, 

               
{0x11,0,{0x00}},         
{REGFLAG_DELAY, 120, {}},         
       
{0x29,0,{0x00}},         
{REGFLAG_DELAY, 5, {}}, 
{REGFLAG_END_OF_TABLE, 0x00, {}} 
};

static void LCD_panel_init(u32 sel)
{
    u32 i;

    panel_pwr(1);
    sunxi_lcd_delay_ms(5);
    panel_rst(1);
    sunxi_lcd_delay_ms(10);
    panel_rst(0);
    sunxi_lcd_delay_ms(10);
    panel_rst(1);
    sunxi_lcd_delay_ms(20);

    for (i = 0; ; i++) {
        if(lcm_initialization_setting[i].cmd == REGFLAG_END_OF_TABLE) {
            break;
        } else if (lcm_initialization_setting[i].cmd == REGFLAG_DELAY) {
            sunxi_lcd_delay_ms(lcm_initialization_setting[i].count);
        } else {
            dsi_dcs_wr(sel, (u8)lcm_initialization_setting[i].cmd, lcm_initialization_setting[i].para_list, lcm_initialization_setting[i].count);
        }
    }

    sunxi_lcd_dsi_clk_enable(sel);

    return;
}

static void LCD_panel_exit(u32 sel)
{
    panel_pwr(0);
    panel_rst(0);
    sunxi_lcd_dsi_clk_disable(sel);

    return ;
}

//sel: 0:lcd0; 1:lcd1
static s32 LCD_user_defined_func(u32 sel, u32 para1, u32 para2, u32 para3)
{
    return 0;
}

__lcd_panel_t CHR050H129_MIPI_RGB_panel = {
    /* panel driver name, must mach the name of lcd_drv_name in sys_config.fex */
    .name = "CHR050H129_MIPI_RGB",
    .func = {
        .cfg_panel_info = LCD_cfg_panel_info,
        .cfg_open_flow = LCD_open_flow,
        .cfg_close_flow = LCD_close_flow,
        .lcd_user_defined_func = LCD_user_defined_func,
    },
};
