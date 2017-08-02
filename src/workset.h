
#ifndef WORKSET_H
#define WORKSET_H

#include <stdint.h>

#define WORKSET_NAME_LENGTH     16
#define WORKSET_PARAM_COUNT     69
#define WORKSET_COUNT           400

#define USER_NAME_LENGTH        16
#define USER_COUNT              200

typedef struct {
    uint16_t prod_count; // 0
    uint16_t inj_pop; // 1
    uint16_t pause; // 2
    uint16_t jump; // 3
    uint16_t p_s; // 4
    uint16_t evacuation; // 5
    uint16_t lub_period; // 6
    uint16_t lub_reload; // 7
    uint16_t lub_count; // 8
    uint16_t lub_time; // 9
    uint16_t temp_over; // 10
    uint16_t temp_under; // 11

    uint16_t tmr_T01; // 12
    uint16_t tmr_T02; // 13
    uint16_t tmr_T03; // 14
    uint16_t tmr_T04; // 15
    uint16_t tmr_T05; // 16
    uint16_t tmr_T06; // 17
    uint16_t tmr_T07; // 18
    uint16_t tmr_T08; // 19
    uint16_t tmr_T09; // 20
    uint16_t tmr_T10; // 21
    uint16_t tmr_T11; // 22
    uint16_t tmr_T12; // 23
    uint16_t tmr_T13; // 24
    uint16_t tmr_T14; // 25
    uint16_t tmr_T15; // 26
    uint16_t tmr_T16; // 27
    uint16_t tmr_T17; // 28
    uint16_t tmr_T18; // 29
    uint16_t tmr_T19; // 30

    uint16_t hyd_U01; // 31
    uint16_t hyd_U02; // 32
    uint16_t hyd_U03; // 33
    uint16_t hyd_U04; // 34
    uint16_t hyd_U05; // 35
    uint16_t hyd_U06; // 36
    uint16_t hyd_U07; // 37
    uint16_t hyd_U08; // 38
    uint16_t hyd_U09; // 39
    uint16_t hyd_U10; // 40
    uint16_t hyd_U11; // 41
    uint16_t hyd_U12; // 42
    uint16_t hyd_U13; // 43
    uint16_t hyd_U14; // 44
    uint16_t hyd_U15; // 45
    uint16_t hyd_U16; // 46

    uint16_t temp_Z1; // 47
    uint16_t temp_Z2; // 48
    uint16_t temp_Z3; // 49
    uint16_t temp_Z4; // 50
    uint16_t temp_Z5; // 51

    uint16_t pwm_Z1; // 52
    uint16_t pwm_Z2; // 53
    uint16_t pwm_Z3; // 54
    uint16_t pwm_Z4; // 55
    uint16_t pwm_Z5; // 56

    uint16_t pres_st1; // 57
    uint16_t pres_st2; // 58
    uint16_t pres_form_hi; // 59
    uint16_t pres_form_lo; // 60
    uint16_t pres_lock; // 61
    uint16_t pres_e_disjunc; // 62
    uint16_t pres_e_save; // 63
    uint16_t pres_e_junc; // 64
    uint16_t pres_e_max; // 65

    uint16_t end_end1st; // 66
    uint16_t end_end2st; // 67
    uint16_t end_form_hi; // 68
}
WORKSET; // 138bytes

void get_param_limits(uint8_t idx, uint16_t *min, uint16_t *max);
void check_limit(uint8_t idx, uint16_t *val);
void set_param(uint8_t idx, uint16_t val);

void trim_name(char *name, int length);

uint16_t get_workset_name_addr(uint16_t num);
uint16_t get_workset_addr(uint16_t num);
uint16_t get_user_name_addr(uint16_t num);

int workset_save(uint16_t idx);
int workset_load(uint16_t idx);

#endif /* WORKSET_H */

