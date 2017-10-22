
#ifndef WORKSET_H
#define WORKSET_H

#include <stdint.h>

#define WORKSET_NAME_LENGTH     14
#define WORKSET_PARAM_COUNT     90
#define WORKSET_COUNT           330

typedef struct {

    uint16_t inj_pop; // 0
    uint16_t pause; // 1
    uint16_t jump; // 2
    uint16_t p_s; // 3
    uint16_t evacuation; // 4
    uint16_t lub_period; // 5
    uint16_t lub_reload; // 6
    uint16_t lub_count; // 7
    uint16_t lub_time; // 8
    uint16_t temp_over; // 9
    uint16_t temp_under; // 10

    uint16_t tmr_T01; // 11
    uint16_t tmr_T02; // 12
    uint16_t tmr_T03; // 13
    uint16_t tmr_T04; // 14
    uint16_t tmr_T05; // 15
    uint16_t tmr_T06; // 16
    uint16_t tmr_T07; // 17
    uint16_t tmr_T08; // 18
    uint16_t tmr_T09; // 19
    uint16_t tmr_T10; // 20
    uint16_t tmr_T11; // 21
    uint16_t tmr_T12; // 22
    uint16_t tmr_T13; // 23
    uint16_t tmr_T14; // 24
    uint16_t tmr_T15; // 25
    uint16_t tmr_T16; // 26
    uint16_t tmr_T17; // 27
    uint16_t tmr_T18; // 28
    uint16_t tmr_T19; // 29

    uint16_t hyd_U01; // 30
    uint16_t hyd_U02; // 31
    uint16_t hyd_U03; // 32
    uint16_t hyd_U04; // 33
    uint16_t hyd_U05; // 34
    uint16_t hyd_U06; // 35
    uint16_t hyd_U07; // 36
    uint16_t hyd_U08; // 37
    uint16_t hyd_U09; // 38
    uint16_t hyd_U10; // 39
    uint16_t hyd_U11; // 40
    uint16_t hyd_U12; // 41
    uint16_t hyd_U13; // 42
    uint16_t hyd_U14; // 43
    uint16_t hyd_U15; // 44
    uint16_t hyd_U16; // 45

    uint16_t hud_P01; // 46
    uint16_t hud_P02; // 47
    uint16_t hud_P03; // 48
    uint16_t hud_P04; // 49
    uint16_t hud_P05; // 50
    uint16_t hud_P06; // 51
    uint16_t hud_P07; // 52
    uint16_t hud_P08; // 53
    uint16_t hud_P09; // 54
    uint16_t hud_P10; // 55
    uint16_t hud_P11; // 56
    uint16_t hud_P12; // 57
    uint16_t hud_P13; // 58
    uint16_t hud_P14; // 59
    uint16_t hud_P15; // 60
    uint16_t hud_P16; // 61
    uint16_t hud_P17; // 62
    uint16_t hud_P18; // 63
    uint16_t hud_P19; // 64
    uint16_t hud_P20; // 65
    uint16_t hud_P21; // 66
    uint16_t hud_P22; // 67

    uint16_t temp_Z1; // 68
    uint16_t temp_Z2; // 69
    uint16_t temp_Z3; // 70
    uint16_t temp_Z4; // 71
    uint16_t temp_Z5; // 72

    uint16_t pwm_Z1; // 73
    uint16_t pwm_Z2; // 74
    uint16_t pwm_Z3; // 75
    uint16_t pwm_Z4; // 76
    uint16_t pwm_Z5; // 77

    uint16_t pres_st1; // 78
    uint16_t pres_st2; // 79
    uint16_t pres_form_hi; // 80
    uint16_t pres_form_lo; // 81
    uint16_t pres_lock; // 82
    uint16_t pres_e_disjunc; // 83
    uint16_t pres_e_save; // 84
    uint16_t pres_e_junc; // 85
    uint16_t pres_e_max; // 86

    uint16_t end_end1st; // 87
    uint16_t end_end2st; // 88
    uint16_t end_form_hi; // 89
}
WORKSET; // 180 bytes

void get_param_limits(uint8_t idx, uint16_t *min, uint16_t *max);
void check_limit(uint8_t idx, uint16_t *val);
void set_param(uint8_t idx, uint16_t val);

void trim_name(char *name, int length);

uint16_t get_workset_name_addr(uint16_t num);
uint16_t get_workset_addr(uint16_t num);

void load_name(uint16_t idx, char *buf);

int workset_save(uint16_t idx);
int workset_load(uint16_t idx);

#endif /* WORKSET_H */

