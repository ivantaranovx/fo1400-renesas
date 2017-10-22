
#include "fo1400.h"

#include "fo1400_common.h"
#include "fo1400_mode_adj.h"
#include "fo1400_mode_manual.h"
#include "fo1400_mode_auto.h"

WORKSET workset; // GLOBAL!

static MAIN_STATE state;
static MAIN_MODE mode = m_unknown;
static uint32_t pwr_count = 0;
static uint32_t pwr_count_t = 0;

void main_task(void)
{
    MAIN_STATE *ps = &state;

    while (mode != state.mode)
    {
        if ((state.mode == m_semi) && (state.oper != o_idle)) break;
        if ((state.mode == m_auto) && (state.oper != o_idle)) break;

        stop();
        state.mode = mode;
        state.oper = o_idle;
        FAIL(OFF);
        engine_enable(true);
        pwr_count_t = pwr_count;
        state.err[0] = e_success;

        state.stat[1] = s_none;
        state.stat[2] = s_none;
        run_scale_timer(TMR_SCALE_D1, false);
        run_scale_timer(TMR_SCALE_D2, false);
        clr_scale_timer(TMR_SCALE_D1);
        clr_scale_timer(TMR_SCALE_D2);

        ps = 0;
        break;
    }

    // wait hydro stop
    if (get_timer(TMR_HYD) > 0) return;

    switch (state.mode)
    {
    case m_adjust:
        op_mode_adj(ps);
        break;
    case m_manual:
        op_mode_manual(ps);
        break;
    case m_semi:
    case m_auto:
        op_mode_auto(ps);
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

    f_cycle_run = false;
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
        state.err[2] = e_success;
        break;
    }

    if (get_timer(TMR_GUARD) == 0)
    {
        if (gc == 3) state.err[2] = e_guard_57;
        if (gc == 4) state.err[2] = e_guard_56;
        gc = 0;
    }

    f_guard_ok = ((s == 3) ? true : false) && f_guard_chk;
}

void engine_enable(bool e)
{
    if (e == false)
    {
        ENGINE(OFF);
        set_hydro_u(0);
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
                state.err[1] = e_not_powered;
                break;
            }
            state.err[1] = e_success;
            engine_state = 10;
            break;
        }
        if (!f_engine_on) break;
        if (check_kn(KH13, S_KH13) > 0) // check inversion
        {
            if (state.oper != o_idle) break;
            engine_state = 20;
            break;
        }
        if (!f_power_on)
        {
            stop();
            state.err[1] = e_not_powered;
            state.oper = o_idle;
            engine_state = 20;
            break;
        }
        if (!RD)
        {
            stop();
            state.err[1] = e_engine_not_ready;
            state.oper = o_idle;
            engine_state = 20;
            break;
        }
        if (OT)
        {
            state.err[1] = e_engine_overheat;
            f_cycle_run = false;
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
        set_hydro_u(0);
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
            state.err[1] = e_success;
            engine_state = 0;
            break;
        }
        if (get_timer(TMR_ENGINE)) break;
        state.err[1] = e_engine_not_ready;
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
            state.err[1] = e_lub_low;
            //f_cycle_stop = true;
            FAIL(ON);
            lub_state = 8;
            break;
        }
        state.err[1] = e_success;
        set_timer(TMR_LUB, (uint32_t) (workset.lub_time) * 1000);
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

void wait_keypress(void)
{
    while (!get_key()) wdt_reset();
    return;
}

int main(void)
{
    int r;

    brd_init();

    uart_rx_cb_register(uart_rx_cb);
    tcp_conn_cb_register(tcp_conn_cb);
    tcp_rx_cb_register(tcp_rx_cb);

    uart_print("\fHardware init complete\r\n");
    uart_printf("Board: %s\r\n", brd_id());
    uart_printf("Machine: %s\r\n", msg_machine);
    uart_printf("Version: %s\r\n", msg_version);

    while (true)
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
    f_cycle_run = false;
    f_cycle_report = false;

    dio_init();

    check_kn(false, S_KH10);
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
    for(r = 0; r < 4; r++) state.err[0] = e_success;

    state.job.id = 0;
    state.job.count = 0;

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

    bus_enable(true);
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
        if (r < 0)
        {
            f_ready = false;
            RDY(OFF);
            if (state.oper != o_idle)
            {
                stop();
                ENGINE(OFF);
                state.err[3] = e_emergency_stop;
                state.oper = o_idle;
            }
        }
        if (r > 0)
        {
            state.err[3] = e_success;
            f_ready = true;
            RDY(ON);
        }

        if ((check_kn(KH0, S_KH0) > 0) && !KH10)
        {
            state.err[3] = e_emergency_stop;
        }

        if (check_kn(CE, S_CE) > 0) pwr_count++;

        if (f_cycle_report)
        {
            f_cycle_report = false;

            char buf[10];
            tcpip_send("{\"type\":\"cycle\",\"m_id\":");
            snprintf(buf, sizeof (buf) - 1, "%hu", tcpip_get_id());
            tcpip_send(buf);
            tcpip_send(",\"p_id\":");
            snprintf(buf, sizeof (buf) - 1, "%hu", state.job.id);
            tcpip_send(buf);
            tcpip_send(",\"pwr\":");
            snprintf(buf, sizeof (buf) - 1, "%lu", pwr_count - pwr_count_t);
            tcpip_send(buf);
            tcpip_send("}");

            pwr_count_t = pwr_count;
        }

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

void tcp_conn_cb(bool connected)
{
    if (!connected) return;

    char buf[10];
    tcpip_send("{\"type\":\"helo\",\"id\":");
    snprintf(buf, sizeof (buf) - 1, "%hu", tcpip_get_id());
    tcpip_send(buf);
    tcpip_send(",\"name\":\"");
    tcpip_send((char*) msg_machine);
    tcpip_send("\",\"version\":\"");
    tcpip_send((char*) msg_version);
    tcpip_send("\"}");
}

void tcp_rx_cb(char *data, uint16_t len)
{
    char buf[20];
    data[len] = 0;
    if (!json_get_param("type", data, buf, sizeof(buf))) return;
    if (!strcmp("users", buf)) ui_users_cb(data);
}

void uart_rx_cb(uint8_t b)
{

}

