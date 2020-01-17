//port info
#ifndef __SFF_IO_H
#define __SFF_IO_H


#define TEST_CODE
#define DEBUG_MODE (1)

#define RETRY_COUNT (5)
#define RETRY_DELAY_MS (100)


struct io_exp_input_t {
	u8 ch;
	u8 addr;
	int port_min;
	int bit_min;
	int bit_max;  /*reprsent the num of ports*/
};
struct io_exp_config_t {
	u8 ch;
	u8 addr;
	u16 val;
};
#define REC_SFF_IO_UNSUPPORTED (6)

typedef enum {
    SFF_IO_IDLE_EVENT = 0,
    SFF_IO_PLUG_IN_EVENT,
    SFF_IO_PLUG_OUT_EVENT,
};

int sff_io_init(void);
void sff_io_deinit(void);
int io_exp_isr_handler(void);
int present_st_get(int port);
int rxlos_get(int port);
int txfault_get(int port);
bool sff_io_is_plugged(int port);
bool is_any_plug_event(void);
u8 plug_event_get(int port);
int io_exps_check(void);
int cpld_ioexp_isr_enable(void);
#endif /*__SFF_IO_H*/
