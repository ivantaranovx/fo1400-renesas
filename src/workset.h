
#ifndef WORKSET_H
#define	WORKSET_H

#include <stdint.h>

#define prod_name_sz    16

typedef struct
{
    char        prod_name[prod_name_sz];

    uint16_t    prod_count;		// 0
    uint16_t    nozzle_out;		// 1
    uint16_t    pause;			// 2
    uint16_t    jump;			// 3
    uint16_t    evacuation;		// 4
    uint16_t    lub_period;		// 5
    uint16_t    lub_reload;		// 6
    uint16_t    lub_count;		// 7
    uint16_t    lub_time;		// 8
    uint16_t    temp_over;		// 9
    uint16_t    temp_under;		// 10

    uint16_t    tmr_T01;		// 11
    uint16_t    tmr_T02;		// 12
    uint16_t    tmr_T03;		// 13
    uint16_t    tmr_T04;		// 14
    uint16_t    tmr_T05;		// 15
    uint16_t    tmr_T06;		// 16
    uint16_t    tmr_T07;		// 17
    uint16_t    tmr_T08;		// 18
    uint16_t    tmr_T09;		// 19
    uint16_t    tmr_T10;		// 20
    uint16_t    tmr_T11;		// 21
    uint16_t    tmr_T12;		// 22
    uint16_t    tmr_T13;		// 23
    uint16_t    tmr_T14;		// 24
    uint16_t    tmr_T15;		// 25
    uint16_t    tmr_T16;		// 26
    uint16_t    tmr_T17;		// 27
    uint16_t    tmr_T18;		// 28
    uint16_t    tmr_T19;		// 29
    
    uint16_t    hyd_U01;		// 30
    uint16_t    hyd_U02;		// 31
    uint16_t    hyd_U03;		// 32
    uint16_t    hyd_U04;		// 33
    uint16_t    hyd_U05;		// 34
    uint16_t    hyd_U06;		// 35
    uint16_t    hyd_U07;		// 36
    uint16_t    hyd_U08;		// 37
    uint16_t    hyd_U09;		// 38
    uint16_t    hyd_U10;		// 39
    uint16_t    hyd_U11;		// 40
    uint16_t    hyd_U12;		// 41
    uint16_t    hyd_U13;		// 42
    uint16_t    hyd_U14;		// 43
    uint16_t    hyd_U15;		// 44
    uint16_t    hyd_U16;		// 45

    uint16_t    temp_Z1;		// 46
    uint16_t    temp_Z2;		// 47
    uint16_t    temp_Z3;		// 48
    uint16_t    temp_Z4;		// 49
    uint16_t    temp_Z5;		// 50

    uint16_t    pwm_Z1;			// 51
    uint16_t    pwm_Z2;			// 52
    uint16_t    pwm_Z3;			// 53
    uint16_t    pwm_Z4;			// 54
    uint16_t    pwm_Z5;			// 55

    uint16_t    pres_st1;		// 56
    uint16_t    pres_st2;		// 57
    uint16_t    pres_form_hi;		// 58
    uint16_t    pres_form_lo;		// 59
    uint16_t    pres_lock;		// 60

    uint16_t    end_end1st;		// 61
    uint16_t    end_end2st;		// 62
    uint16_t    end_form_hi;		// 63
}
WORKSET;

#define WORKSET_PARAM_COUNT	64
#define ADDR_WORKSET            0x0000

void get_param_limits(uint8_t num, uint16_t *min, uint16_t *max);
void check_param(uint8_t num, uint16_t *param);

unsigned workset_load(uint8_t num, WORKSET *set);
unsigned workset_save(uint8_t num, WORKSET *set);

#endif	/* WORKSET_H */

