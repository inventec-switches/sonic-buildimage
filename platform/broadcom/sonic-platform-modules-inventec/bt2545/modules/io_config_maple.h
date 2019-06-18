//port info
#ifndef __IO_CONFIG_MAPLE_H
#define __IO_CONFIG_MAPLE_H


#define CPLD_EXPANDER_INT_N (12) /*GPIO 12*/
#define CPLD_I2C_CH (2)
#define CPLD_I2C_ADDR (0x33)
/*reset mux gpio*/
#define RESET_MUX_GPIO_NUM (69)
#define PORT_NUM (56)
#define END_OF_TABLE (0xff)
/*io expander layout*/
const int io_i2c_ch_table[] =
{
    7,
    8,
    9,
    10,
    11,
    12,
    6
};

struct io_exp_input_t io_exp_present[] =
{
    /*sfp*/
    {.ch = 7, .addr = 0x20, .port_min = 0, .bit_min = 4, .bit_max = 7},
	{.ch = 7, .addr = 0x21, .port_min = 4, .bit_min = 4, .bit_max = 7},
	{.ch = 8, .addr = 0x20, .port_min = 8, .bit_min = 4, .bit_max = 7},
	{.ch = 8, .addr = 0x21, .port_min = 12, .bit_min = 4, .bit_max = 7},
	{.ch = 9, .addr = 0x20, .port_min = 16, .bit_min = 4, .bit_max = 7},
	{.ch = 9, .addr = 0x21, .port_min = 20, .bit_min = 4, .bit_max = 7},
	{.ch = 10, .addr = 0x20, .port_min = 24, .bit_min = 4, .bit_max = 7},
	{.ch = 10, .addr = 0x21, .port_min = 28, .bit_min = 4, .bit_max = 7},
	{.ch = 11, .addr = 0x20, .port_min = 32, .bit_min = 4, .bit_max = 7},
	{.ch = 11, .addr = 0x21, .port_min = 36, .bit_min = 4, .bit_max = 7},
	{.ch = 12, .addr = 0x20, .port_min = 40, .bit_min = 4, .bit_max = 7},
	{.ch = 12, .addr = 0x21, .port_min = 44, .bit_min = 4, .bit_max = 7},
    /*qsfp*/
    {.ch = 6,  .addr = 0x22, .port_min = 48, .bit_min = 8, .bit_max = 15},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct io_exp_input_t io_exp_txfault[] =
{
    /*sfp*/
    {.ch = 7, .addr = 0x20, .port_min = 0, .bit_min = 0, .bit_max = 3},
	{.ch = 7, .addr = 0x21, .port_min = 4, .bit_min = 0, .bit_max = 3},
	{.ch = 8, .addr = 0x20, .port_min = 8, .bit_min = 0, .bit_max = 3},
	{.ch = 8, .addr = 0x21, .port_min = 12, .bit_min = 0, .bit_max = 3},
	{.ch = 9, .addr = 0x20, .port_min = 16, .bit_min = 0, .bit_max = 3},
	{.ch = 9, .addr = 0x21, .port_min = 20, .bit_min = 0, .bit_max = 3},
	{.ch = 10, .addr = 0x20, .port_min = 24, .bit_min = 0, .bit_max = 3},
	{.ch = 10, .addr = 0x21, .port_min = 28, .bit_min = 0, .bit_max = 3},
	{.ch = 11, .addr = 0x20, .port_min = 32, .bit_min = 0, .bit_max = 3},
	{.ch = 11, .addr = 0x21, .port_min = 36, .bit_min = 0, .bit_max = 3},
	{.ch = 12, .addr = 0x20, .port_min = 40, .bit_min = 0, .bit_max = 3},
	{.ch = 12, .addr = 0x21, .port_min = 44, .bit_min = 0, .bit_max = 3},
   	{.ch = END_OF_TABLE} /*end of table*/
};
struct io_exp_input_t io_exp_rxlos[] =
{
    /*sfp*/
    {.ch = 7, .addr = 0x20, .port_min = 0, .bit_min = 12, .bit_max = 15},
	{.ch = 7, .addr = 0x21, .port_min = 4, .bit_min = 12, .bit_max = 15},
	{.ch = 8, .addr = 0x20, .port_min = 8, .bit_min = 12, .bit_max = 15},
	{.ch = 8, .addr = 0x21, .port_min = 12, .bit_min = 12, .bit_max = 15},
	{.ch = 9, .addr = 0x20, .port_min = 16, .bit_min = 12, .bit_max = 15},
	{.ch = 9, .addr = 0x21, .port_min = 20, .bit_min = 12, .bit_max = 15},
	{.ch = 10, .addr = 0x20, .port_min = 24, .bit_min = 12, .bit_max = 15},
	{.ch = 10, .addr = 0x21, .port_min = 28, .bit_min = 12, .bit_max = 15},
	{.ch = 11, .addr = 0x20, .port_min = 32, .bit_min = 12, .bit_max = 15},
	{.ch = 11, .addr = 0x21, .port_min = 36, .bit_min = 12, .bit_max = 15},
	{.ch = 12, .addr = 0x20, .port_min = 40, .bit_min = 12, .bit_max = 15},
	{.ch = 12, .addr = 0x21, .port_min = 44, .bit_min = 12, .bit_max = 15},
   	{.ch = END_OF_TABLE} /*end of table*/
};
/*only config input pin for our purpose*/
struct io_exp_config_t io_exp_config[] =
{
    /*sfp*/
    {.ch = 7, .addr = 0x20, .val = 0xf0ff},
    {.ch = 7, .addr = 0x21, .val = 0xf0ff},
    {.ch = 7, .addr = 0x22, .val = 0x0000},
    {.ch = 8, .addr = 0x20, .val = 0xf0ff},
    {.ch = 8, .addr = 0x21, .val = 0xf0ff},
    {.ch = 8, .addr = 0x22, .val = 0x0000},
    {.ch = 9, .addr = 0x20, .val = 0xf0ff},
    {.ch = 9, .addr = 0x21, .val = 0xf0ff},
    {.ch = 9, .addr = 0x22, .val = 0x0000},
    {.ch = 10, .addr = 0x20, .val = 0xf0ff},
    {.ch = 10, .addr = 0x21, .val = 0xf0ff},
    {.ch = 10, .addr = 0x22, .val = 0x0000},
    {.ch = 11, .addr = 0x20, .val = 0xf0ff},
    {.ch = 11, .addr = 0x21, .val = 0xf0ff},
    {.ch = 11, .addr = 0x22, .val = 0x0000},
    {.ch = 12, .addr = 0x20, .val = 0xf0ff},
    {.ch = 12, .addr = 0x21, .val = 0xf0ff},
    {.ch = 12, .addr = 0x22, .val = 0x0000},
    /*qsfp*/
    {.ch = 6, .addr = 0x20, .val = 0x0000},
    {.ch = 6, .addr = 0x21, .val = 0xff00},
    {.ch = 6, .addr = 0x22, .val = 0xffff},
   	{.ch = END_OF_TABLE} /*end of table*/
};
/*cpld 0x36, 0x37 maping*/
int input_change_table[] =
{
	0, /*port 0~3 , only record first port*/
	4,
	8,
	12,	
	16, 
    20,
	24,
	28,	
    32,
    36,
    40,
    44,
    48
};
int ioexp_port_2_port[PORT_NUM] =
{
    1, 0, 3, 2 ,5, 4, 7, 6,
    9, 8, 11, 10 ,13, 12, 15, 14,
    17, 16, 19, 18 ,21, 20, 23, 22,
    25, 24, 27, 26 ,29, 28, 31, 30,
    33, 32, 35, 34 ,37, 36, 39, 38,
    41, 40, 43, 42 ,45, 44, 47, 46,
    49, 48, 51, 50 ,53, 52, 55, 54,

};
#endif /*__IO_CONFIG_MAPLE_H*/
