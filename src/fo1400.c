
#include "fo1400.h"

#include "fo1400_common.h"
#include "fo1400_mode_adj.h"
#include "fo1400_mode_manual.h"
#include "fo1400_mode_auto.h"

static MAIN_STATE state;
WORKSET workset; // GLOBAL!

static int pwr_count = 0;
static MAIN_MODE mode = m_unknown;

void main_task(void)
{
    while (mode != state.mode)
    {
        stop();
        if (state.mode == m_auto) f_cycle_stop = true;
        state.mode = mode;
        state.oper = o_idle;
        state.error = e_success;
        FAIL(OFF);
        engine_enable(true);
        break;
    }

    switch (state.mode)
    {
    case m_adjust:
        op_mode_adj(&state);
        break;
    case m_manual:
        op_mode_manual(&state);
        break;
    case m_semi:
    case m_auto:
        op_mode_auto(&state);
        break;
    default:
        break;
    }
}

void check_mode_selector(void)
{
    int j = 0;
    int i = m_unknown;

    if (M_ADJUST)
    {
        i = m_adjust;
        j++;
    }
    if (M_MANUAL)
    {
        i = m_manual;
        j++;
    }
    if (M_SEMI_AUTO)
    {
        i = m_semi;
        j++;
    }
    if (M_AUTO)
    {
        i = m_auto;
        j++;
    }
    if (j > 1) return;
    if (i == mode) return;

    f_cycle_stop = true;
    mode = i;
}

void guard_task(void)
{
    const uint8_t gs[] = {3, 2, 0, 2, 3};
    static int gc = 0;
    uint8_t s = 0;

    if (BK56) s |= 1;
    if (BK57) s |= 2;

    if (!f_guard_chk && (gc == 5)) gc = 0;

    for (;;)
    {
        if (f_guard_chk) break;
        if (gs[gc] != s) break;
        gc++;
        set_timer(TMR_GUARD, 3000);
        if (gc < 5) break;
        f_guard_chk = true;
        kill_timer(TMR_GUARD);
        state.error = e_success;
        break;
    }

    if (get_timer(TMR_GUARD) == 0)
    {
        if (gc == 3) state.error = e_guard_57;
        if (gc == 4) state.error = e_guard_56;
        gc = 0;
    }

    f_guard_ok = ((s == 3) ? true : false) && f_guard_chk;
}

void engine_enable(bool e)
{
    if (e == false)
    {
        ENGINE(OFF);
        set_hydro(0);
        return;
    }
    if (!engine_ready(&state)) return;
    ENGINE(ON);
}

void engine_task(void)
{
    static int engine_state = 0;

    switch (engine_state)
    {
    case 0:
        if (check_kn(KH12, S_KH12) > 0)
        {
            if (f_engine_on) break;
            if (!f_ready) break;
            if (!f_power_on)
            {
                state.error = e_not_powered;
                break;
            }
            state.error = e_success;
            engine_state = 10;
            break;
        }
        if (!f_engine_on) break;
        if (check_kn(KH13, S_KH13) > 0)
        {
            if (state.oper != o_idle) break;
            engine_state = 20;
            break;
        }
        if (!f_power_on)
        {
            stop();
            state.error = e_not_powered;
            state.oper = o_idle;
            engine_state = 20;
            break;
        }
        if (!RD)
        {
            stop();
            state.error = e_engine_not_ready;
            state.oper = o_idle;
            engine_state = 20;
            break;
        }
        if (OT)
        {
            state.error = e_engine_overheat;
            f_cycle_stop = true;
            FAIL(ON);
            engine_state = 1;
            break;
        }
        break;
    case 1:
        if (state.oper != o_idle) break;
        engine_state = 20;
        break;
    case 10:
        set_hydro(0);
        KM10(ON);
        set_timer(TMR_ENGINE, 3000);
        engine_state = 11;
        break;
    case 11:
        if (get_timer(TMR_ENGINE)) break;
        KM5(ON);
        set_timer(TMR_ENGINE, 2000);
        engine_state = 12;
        break;
    case 12:
        if (RD)
        {
            f_engine_on = true;
            f_guard_chk = false;
            engine_enable(true);
            state.error = e_success;
            engine_state = 0;
            break;
        }
        if (get_timer(TMR_ENGINE)) break;
        state.error = e_engine_not_ready;
        engine_state = 20;
        break;
    case 20:
        engine_enable(false);
        KM5(OFF);
        KM10(OFF);
        engine_state = 21;
        break;
    case 21:
        if (RD) break;
        f_engine_on = false;
        engine_state = 0;
        break;
    }
}

void lub_task(void)
{
    static int lub_period;
    static int lub_push_count;
    static int lub_state = 0;

    if (check_kn(f_engine_on, S_ENG) > 0) lub_period = workset.lub_reload * 60;

    switch (lub_state)
    {
    case 0:
        if (!engine_ready(&state)) break;
        if (KH11)
        {
            lub_push_count = 1;
            lub_state = 2;
            break;
        }
        if (get_timer(TMR_LUB) > 0) break;
        set_timer(TMR_LUB, 1000);
        lub_period--;
        if (lub_period > 0) break;
        lub_push_count = workset.lub_count;
        lub_state = 2;
        break;
    case 2:
        if (BK58)
        {
            state.error = e_lub_low;
            //f_cycle_stop = true;
            FAIL(ON);
            lub_state = 8;
            break;
        }
        set_timer(TMR_LUB, workset.lub_time * 1000);
        KM7(ON);
        lub_state = 4;
        break;
    case 4:
        if ((get_timer(TMR_LUB) > 0) && f_engine_on) break;
        KM7(OFF);
        set_timer(TMR_LUB, 1000);
        lub_state = 6;
        break;
    case 6:
        if (get_timer(TMR_LUB) > 0) break;
        lub_state = 2;
        lub_push_count--;
        if (lub_push_count == 0) lub_state = 8;
        break;
    case 8:
        if (KH11) break;
        lub_period = workset.lub_period * 60;
        lub_state = 0;
        break;
    }
}

int main(void)
{
    int r;

    brd_init();

    uart_rx_cb_register(uart_rx_cb);

    uart_print("\fHarware init complete\r\n");
    uart_printf("Board: %s\r\n", brd_id());
    uart_printf("Machine: %s\r\n", msg_machine);
    uart_printf("Version: %s\r\n", msg_version);

    while (1)
    {
        uart_print("LCD init: ");
        if (lcd_init())
        {
            uart_print("ok\r\n");
            break;
        }
        uart_print("error\r\n");
        set_timer(TMR_SYS, 100);
        while (get_timer(TMR_SYS) > 0) wdt_reset();
    }

    lcd_clear();
    lcd_print_rom(STR1_ADDR, msg_machine);
    lcd_print_rom(STR2_ADDR, msg_version);

    set_timer(TMR_SYS, 2000);

    f_ready = false;
    f_power_on = false;
    f_guard_chk = false;
    f_guard_ok = false;
    f_engine_on = false;
    f_heat_on = false;
    f_cycle_stop = false;

    dio_init();

    check_kn(true, S_KH10);
    check_kn(true, S_KH12);
    check_kn(true, S_KH13);
    check_kn(true, S_KH14);
    check_kn(false, S_ENG);
    check_kn(false, S_CE);
    check_kn(true, S_KM1);

    thermo_heat_enable(false);
    engine_enable(false);

    state.mode = m_unknown;
    state.oper = o_idle;
    state.error = e_success;

    if (is_wdt_rst())
    {
        lcd_print_rom(STR4_ADDR, err_wdt);
        wait_keypress();
    }

    for (r = 0; r < 3; r++)
    {
        if (workset_load(0)) break;
    }
    if (r == 3)
    {
        lcd_print_rom(STR4_ADDR, err_eeprom);
        wait_keypress();
    }

    while (get_timer(TMR_SYS) > 0) wdt_reset();

    lcd_clear();
    uart_print("Run\r\n");

    for (;;)
    {
        r = check_kn(KM1, S_KM1);
        if (r > 0)
        {
            f_power_on = true;
            f_guard_chk = false;
        }
        if (r < 0)
        {
            f_power_on = false;
        }

        r = check_kn(KH14, S_KH14);
        if ((r > 0) && f_power_on)
        {
            thermo_heat_enable(true);
            f_heat_on = true;
        }
        if (r < 0)
        {
            thermo_heat_enable(false);
            f_heat_on = false;
        }

        r = check_kn(KH10, S_KH10);
        if (r > 0)
        {
            f_ready = false;
            RDY(OFF);
            if (state.oper != o_idle)
            {
                stop();
                ENGINE(OFF);
                state.error = e_emergency_stop;
                state.oper = o_idle;
            }
        }
        if (r < 0)
        {
            f_ready = true;
            RDY(ON);
        }

        if (check_kn(CE, S_CE) > 0) pwr_count++;

        check_mode_selector();
        guard_task();
        engine_task();
        lub_task();
        thermo_task();
        main_task();
        dio_task();
        tcpip_task();
        ui_task(&state);
        
        wdt_reset();
    }
}

void uart_rx_cb(uint8_t b)
{
    
}

void wait_keypress(void)
{
    while (!get_key()) wdt_reset();
    return;
}

