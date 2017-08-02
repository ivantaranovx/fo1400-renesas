
#include "hal.h"
#include "sfr32c84.h"

#include <stdarg.h>
#include <stdio.h>

#define ISR __attribute__ ((interrupt))

#define I2C_WP  
#define BUS_RDY p7_4

#define BUS_STB p15_7
#define BUS_RD  p15_6
#define BUS_WR  p15_5

/* address p15[4:0] */
#define BUS_ADDR    p15

/* data */
#define BUS_LO      p12
#define BUS_HI      p13

static uint32_t timers[TMR_MAX];
static uint16_t scale_timers[TMR_SCALE_MAX];

static uint16_t code = 0;
static uint16_t adc[8];

#define uart_ring_buf_sz 1024
static uint8_t uart_ring_buf[uart_ring_buf_sz];
static uint8_t *uart_in_ptr = uart_ring_buf;
static uint8_t *uart_out_ptr = uart_ring_buf;
static uart_rx_cb_func uart_rx_cb = 0;

static uint8_t u0rx;
static bool u0rx_f;

static uint8_t i2c_addr;
static uint8_t *i2c_buf;
static uint16_t i2c_len;
static uint8_t i2c_flags;
static int i2c_status;

static uint16_t enc[2] = {0};

static char *tmr_names[] = {
    "TMR_SYS",
    "TMR_ENGINE",
    "TMR_LUB",
    "TMR_GUARD",
    "TMR_UI",
    "TMR_HYD",

    "TMR_ARP",
    "TMR_UIP",
    "TMR_EEPROM",
    "TMR_SRV",

    "TMR_1",
    "TMR_2",
    "TMR_3",
    "TMR_4",
    "TMR_5",
    "TMR_6",
    "TMR_7",
    "TMR_8",
    "TMR_9",
    "TMR_10",
    "TMR_11",
    "TMR_12",
    "TMR_13",
    "TMR_14",
    "TMR_15",
    "TMR_16",
    "TMR_17",
    "TMR_18",
    "TMR_19",
};

/*
 * reserved memory 2 byte
 * see linker script
 */
typedef struct
{
    unsigned rst_wdt : 1;

}
PERSISTENT_FLAGS;

#pragma ADDRESS pflags 0x63FE
PERSISTENT_FLAGS pflags;


void WDT_ISR(void) ISR;

void AD_ISR(void) ISR;
void TimerB0_ISR(void) ISR;
void TimerB1_ISR(void) ISR;
void UART0_Tx_ISR(void) ISR;
void UART0_Rx_ISR(void) ISR;
void UART1_Tx_ISR(void) ISR;
void UART1_Rx_ISR(void) ISR;
void UART3_TRx_ISR(void) ISR;
void UART3_BCn_ISR(void) ISR;
void TimerA3_ISR(void) ISR;
void TimerA4_ISR(void) ISR;

extern void start(void);

/* Interrupt vectors */

const void * __attribute__((section(".vec"))) _vec[8] = {

    0, //Undefined Instruction
    0, //Overflow
    0, //BRK Instruction
    0, //Address Match
    0, //Reserved space
    WDT_ISR, //Watchdog Timer
    0, //Reserved space
    0 //NMI
};

const void *_var_vects[64] = {

    0, // BRK Instruction
    0, // Reserved Space
    0, // Reserved Space
    0, // Reserved Space
    0, // Reserved Space
    0, // Reserved Space
    0, // Reserved Space
    0, // Reserved Space
    0, // DMA0
    0, // DMA1
    0, // DMA2
    0, // DMA3
    0, // Timer A0
    0, // Timer A1
    0, // Timer A2
    TimerA3_ISR,  // Timer A3
    TimerA4_ISR,  // Timer A4
    UART0_Tx_ISR, // UART0 Transmission, NACK
    UART0_Rx_ISR, // UART0 Reception, ACK
    UART1_Tx_ISR, // UART1 Transmission, NACK
    UART1_Rx_ISR, // UART1 Reception, ACK
    TimerB0_ISR, // Timer B0
    TimerB1_ISR, // Timer B1
    0, // Timer B2
    0, // Timer B3
    0, // Timer B4
    0, // INT5
    0, // INT4
    0, // INT3
    0, // INT2
    0, // INT1
    0, // INT0
    0, // Timer B5
    0, // UART2 Transmission, NACK
    0, // UART2 Reception, ACK
    UART3_TRx_ISR, // UART3 Transmission, NACK
    UART3_TRx_ISR, // UART3 Reception, ACK
    0, // UART4 Transmission, NACK
    0, // UART4 Reception, ACK
    0, // Bus Conflict Detect, Start Condition Detect, Stop Condition Detect (UART2)
    UART3_BCn_ISR, // Bus Conflict Detect, Start Condition Detect, Stop Condition Detect (UART3/UART0)
    0, // Bus Conflict Detect, Start Condition Select, Stop Condition Detect(UART4/UART1)
    AD_ISR, // A/D0
    0, // Key Input
    0, // Intelligent I/O Interrupt 0
    0, // Intelligent I/O Interrupt 1
    0, // Intelligent I/O Interrupt 2
    0, // Intelligent I/O Interrupt 3
    0, // Intelligent I/O Interrupt 4
    0, // Reserved Space
    0, // Reserved Space
    0, // Reserved Space
    0, // Intelligent I/O Interrupt 8
    0, // Intelligent I/O Interrupt 9, CAN 0
    0, // Intelligent I/O Interrupt 10, CAN 1
    0, // Reserved Space
    0, // Reserved Space
    0, // CAN 2
    0, // Reserved Space 
    0, // Reserved Space 
    0, // Reserved Space 
    0, // Reserved Space 
    0, // Reserved Space 
    0, // Reserved Space 
};

void WDT_ISR(void)
{
    // disable interrupts
    ad0ic = 0;
    s0tic = 0;
    s0ric = 0;
    s1ric = 0;
    s1tic = 0;
    s3tic = 0;
    s3ric = 0;
    bcn3ic = 0;
    tb1ic = 0;

    pflags.rst_wdt = true;

    // reset
    prc1 = 1;
    pm03 = 1;
    prc1 = 0;
}

void wdt_reset(void)
{
    wdts = 0xff;
    pflags.rst_wdt = false;
}

bool is_wdt_rst(void)
{
    return pflags.rst_wdt;
}

char *brd_id(void)
{
    return "M30845";
}

void brd_init(void)
{
    // not ready
    BUS_RDY = 1;
    pd7_4 = 1;

    // main clock divider set to 1:1 (clock = 32MHz)
    prc0 = 1;
    mcd = 0x12;
    prc0 = 0;

    for (int i = 0; i < TMR_MAX; i++) timers[i] = UINT32_MAX;
    for (int i = 0; i < TMR_SCALE_MAX; i++) scale_timers[i] = UINT16_MAX;
    for (int i = 0; i < 8; i++) adc[i] = 0;

    // check cold startup bit
    if (!wdc5)
    {
        wdc5 = 1;
        pflags.rst_wdt = false;
    }

    // Timer B0 setup (T=1us)
    // clock = f1 (32MHz)
    // timer mode
    tb0mr = 0b00000000;
    tb0ic = 1;

    // Timer B1 setup (T=1ms)
    // clock = f8 (4MHz)
    // timer mode
    tb1mr = 0b00000000;
    tb1s = 1;

    // keyboard
    // pull-up P10[7:4]
    pu31 = 1;

    // peripheral bus
    p12 = 0;
    p13 = 0;
    p15 = 0;
    pd15 = 0xFF;

    // pull-up data bus
    pu34 = 1;
    pu35 = 1;
    pu36 = 1;
    pu37 = 1;

    // lcd
    WH_RS = 0;
    WH_RW = 1;
    WH_OE = 0;
    WH_PORT_IN();

    pd8_6 = 1;
    pd8_7 = 1;
    pd11_4 = 1;
    pu32 = 1;

    // A/D setup
    // Single sweep mode, AN0[7:0]
    // 10bit, sample and hold function enabled
    // fAD divided by 8; Tcycle = 8.25us x 8 = 66us
    ad0con0 = 0b00010000;
    ad0con1 = 0b00101011;
    ad0con2 = 0b00000101;
    ad0con3 = 0b00000100;

    // D/A setup
    da0 = 0;
    da1 = 0;
    dacon = 0b00000011;

    // SPI port (serial0)
    // 8MHz
    pd6_4 = 0; // ENC /INT
    p6_5 = 1; // ENC /RESET
    pd6_5 = 1;
    p6_0 = 1; // ENC /CS
    pd6_0 = 1;
    ps0_1 = 1;
    ps0_3 = 1;
    u0brg = 1;
    u0mr = 0b00000001;
    u0c0 = 0b10010000;
    u0c1 = 0b00100100;
    ckph_u0smr3 = 1;

    // debug port (serial1)
    // 9600 8N1
    ps0_7 = 1;
    u1brg = 207;
    u1mr = 0b00000101;
    u1c0 = 0b00010000;
    re_u1c1 = 1;

    // I2C slaves reset
    prc2 = 1;
    pd9_1 = 1;
    prc2 = 0;
    for (int i = 0; i < 10; i++)
    {
        p9_1 = 0;
        _delay_us(8);
        p9_1 = 1;
        _delay_us(8);
    }
    prc2 = 1;
    pd9_1 = 0;
    prc2 = 0;

    // I2C bus (serial3)
    // 100 KHz
    prc2 = 1;
    ps3 = 0x06;
    prc2 = 0;
    u3brg = 39;
    u3c0 = 0b10110001;
    u3c1 = 0b00000100;
    u3smr = 0b00000011;
    ckph_u3smr3 = 1;
    sclhi_u3smr4 = 1;
    sdhi_u3smr2 = 1;
    u3mr = 0b00000010;
    ir_s3tic = 0;
    ir_s3ric = 0;
    ir_bcn3ic = 0;

    // two-phase processing
    ta3mr = 0b11010001;
    ta3p = 1;
    ta3 = 0;
    ta3s = 1;

    ta4mr = 0b11010001;
    ta4p = 1;
    ta4 = 0;
    ta4s = 1;

    // watchdog init ~131ms
    wdc7 = 1;
    wdts = 0xff;

    // enable interrupts
    ta3ic = 2;
    ta4ic = 2;
    tb1ic = 3;
    ad0ic = 4;
    s0tic = 5;
    s0ric = 5;
    s1ric = 5;
    s1tic = 5;
    s3tic = 5;
    s3ric = 5;
    bcn3ic = 5;
}

/***************** UART *****************/

void UART1_Tx_ISR(void)
{
    if (uart_out_ptr == uart_in_ptr)
    {
        te_u1c1 = 0;
    }
    else
    {
        u1tb = *(uart_out_ptr++);
        if (uart_out_ptr >= &uart_ring_buf[uart_ring_buf_sz]) uart_out_ptr = uart_ring_buf;
    }
}

void uart_rx_cb_register(uart_rx_cb_func f)
{
    uart_rx_cb = f;
}

void UART1_Rx_ISR(void)
{
    uint8_t r = u1rb;
    if (uart_rx_cb) uart_rx_cb(r);
}

void uart_print(char *buf)
{
    while (*buf)
    {
        while ((te_u1c1) && (uart_in_ptr == uart_out_ptr));

        *(uart_in_ptr++) = *(buf++);
        if (uart_in_ptr >= &uart_ring_buf[uart_ring_buf_sz]) uart_in_ptr = uart_ring_buf;

        if (!te_u1c1)
        {
            te_u1c1 = 1;
            ir_s1tic = 1;
        }
    }
}

void uart_printf(const char *fmt, ...)
{
    va_list args;
    char buf[256];
    va_start(args, fmt);
    vsnprintf(buf, sizeof (buf), fmt, args);
    va_end(args);
    uart_print(buf);
}

/***************** I2C *****************/

void i2c_io(uint8_t addr, uint8_t *buf, uint16_t len, uint8_t flags)
{
    if (len == 0) return;
    if (i2c_status < 0) return;

    i2c_addr = addr;
    i2c_buf = buf;
    i2c_len = len;
    i2c_flags = flags & 0x0F;
    i2c_status = -1;

    if (bbs_u3smr)
        rstareq_u3smr4 = 1;
    else
        stareq_u3smr4 = 1;
    stspsel_u3smr4 = 1;
}

void UART3_BCn_ISR(void)
{
    ir_bcn3ic = 0;
    if (bbs_u3smr)
    {
        sdhi_u3smr2 = 0;
        u3tb = 0x100 | i2c_addr;
        te_u3c1 = 1;
    }
    stspsel_u3smr4 = 0;
    if (i2c_flags & 0x40) i2c_status = 0;
    if (i2c_flags & 0x20) i2c_status = 1;
}

void UART3_TRx_ISR(void)
{
    if ((i2c_flags & 0x80) && (i2c_addr & 1))
    {
        *i2c_buf = u3rb;
        i2c_buf++;
        if (i2c_len > 0) i2c_len--;
    }
    else
    {
        if (u3rb & 0x100) // NACK
        {
            i2c_flags = 0x40;
            goto ex;
        }
    }
    i2c_flags |= 0x80;
    if (i2c_len == 0)
    {
        i2c_flags |= 0x20;
        goto ex;
    }
    if (i2c_addr & 1)
    {
        if (i2c_len > 1) u3tb = 0x0FF;
        else u3tb = 0x1FF;
    }
    else
    {
        u3tb = 0x100 | *i2c_buf;
        i2c_buf++;
        i2c_len--;
    }
    return;
ex:
    te_u3c1 = 0;
    if (i2c_flags & 0x01)
    {
        i2c_status = 1;
    }
    else
    {
        sdhi_u3smr2 = 1;
        stpreq_u3smr4 = 1;
        stspsel_u3smr4 = 1;
    }
}

int i2c_io_status(void)
{
    return i2c_status;
}

/***************** ADC *****************/

void AD_ISR(void)
{
    uint16_t *ptr = &ad00;
    for (uint8_t i = 0; i < 8; i++)
    {
        adc[i] -= adc[i] >> 4;
        adc[i] += *(ptr++) & 0x03FF;
    }
}

double get_adc_u(uint8_t ch)
{
    double res;
    res = adc[ch] >> 4;
    res *= 5;
    return res / 1023;
}

void set_dac(uint8_t ch, uint8_t val)
{
    if (ch == 0) da0 = val;
    if (ch == 1) da1 = val;
}

/* utilize B0 timer */
void _delay_us(uint16_t delay)
{
    tb0s = 0;
    tb0 = delay << 5;
    tb0s = 1;
    while (tb0s);
}

void TimerB0_ISR(void)
{
    tb0s = 0;
}

/* utilize B1 timer */
void set_timer(TMR_NUM num, uint32_t delay)
{
    if (num >= TMR_MAX) return;
    if (num >= TMR_1) uart_printf("set_timer %s delay %lu\r\n", tmr_names[num], delay);
    if (delay == UINT32_MAX) delay--;
    timers[num] = delay;
}

int8_t get_timer(TMR_NUM num)
{
    if (num >= TMR_MAX) return -1;
    if (timers[num] == UINT32_MAX) return -1;
    if (timers[num] > 0) return 1;
    timers[num] = UINT32_MAX;
    if (num >= TMR_1) uart_printf("timer %s is done\r\n", tmr_names[num]);
    return 0;
}

void kill_timer(TMR_NUM num)
{
    if (num >= TMR_MAX) return;
    if (num >= TMR_1) uart_printf("timer %s is killed\r\n", tmr_names[num]);
    timers[num] = UINT32_MAX;
}

void clr_scale_timer(TMR_SCALE_NUM num)
{
    if (num >= TMR_SCALE_MAX) return;
    scale_timers[num] = 0;
}

uint16_t get_scale_timer(TMR_SCALE_NUM num)
{
    if (num >= TMR_SCALE_MAX) return UINT16_MAX;
    return scale_timers[num];
}

void TimerB1_ISR(void)
{
    static uint8_t scan = 0x0E;
    static uint16_t pcode = 0;
    static uint16_t tcode = 0;
    static uint8_t bnc = 0;
    static uint8_t ad_delay = 0;
    int i;

    // set reload register, T = 1ms
    tb1 = 32000;

    for (i = 0; i < TMR_MAX; i++)
    {
        if (timers[i] == UINT32_MAX) continue;
        if (timers[i] > 0) timers[i]--;
    }
    for (i = 0; i < TMR_SCALE_MAX; i++)
    {
        if (scale_timers[i] < UINT16_MAX) scale_timers[i]++;
    }

    // keyboard scan
    pcode <<= 4;
    pcode |= ((p10 >> 4) ^ 0x0F) & 0x0F;
    p10 = scan;
    pd10 = scan ^ 0x0F;
    scan <<= 1;
    scan |= 1;
    scan &= 0x0F;
    if (scan == 0x0F)
    {
        if (tcode == pcode) bnc++;
        else bnc = 0;
        if (bnc > 10) // debounce 40ms
        {
            code = pcode;
        }
        tcode = pcode;
        pcode = 0;
        scan = 0x0E;
    }

    ad_delay++;
    if (ad_delay > 9)
    {
        ad_delay = 0;
        adst_ad0con0 = 1;
    }

}

uint16_t get_key_code(void)
{
    return code;
}

char get_key(void)
{
    switch (code)
    {
    case 0x0000: return 0;
    case 0x1000: return 'A';
    case 0x0100: return '1';
    case 0x0010: return '2';
    case 0x0001: return '3';
    case 0x2000: return 'B';
    case 0x0200: return '4';
    case 0x0020: return '5';
    case 0x0002: return '6';
    case 0x4000: return 'C';
    case 0x0400: return '7';
    case 0x0040: return '8';
    case 0x0004: return '9';
    case 0x8000: return 'D';
    case 0x0800: return '*';
    case 0x0080: return '0';
    case 0x0008: return '#';
    }
    return 0xFF;
}

void bus_enable(bool e)
{
    BUS_RDY = !e;
}

void bus_write(uint8_t addr, uint16_t data)
{
    BUS_ADDR = addr & 0x1F;
    BUS_LO = data;
    BUS_HI = data >> 8;
    pd12 = 0xFF;
    pd13 = 0xFF;
    _delay_us(2);
    BUS_WR = 1;
    _delay_us(2);
    BUS_WR = 0;
    pd12 = 0;
    pd13 = 0;
}

uint16_t bus_read(uint8_t addr)
{
    uint16_t r;

    BUS_ADDR = addr & 0x1F;
    BUS_RD = 1;
    _delay_us(2);
    r = BUS_HI;
    r <<= 8;
    r |= BUS_LO & 0xFF;
    BUS_RD = 0;
    return r;
}

void lcd_write(unsigned char v)
{
    WH_RW = 0;
    WH_PORT_OUT();
    _delay_us(1);

    WH_OE = 1;
    _delay_us(1);
    WH_PORT = (WH_PORT & 0xF0) | (v >> 4);
    _delay_us(1);
    WH_OE = 0;
    _delay_us(1);

    WH_OE = 1;
    _delay_us(1);
    WH_PORT = (WH_PORT & 0xF0) | (v & 0x0F);
    _delay_us(1);
    WH_OE = 0;
    _delay_us(1);

    WH_RW = 1;
    WH_PORT_IN();
}

bool lcd_busy(void)
{
    WH_RS = 0;
    _delay_us(1);
    for (int tmp = 500; tmp > 0; tmp--)
    {
        WH_OE = 1;
        _delay_us(1);
        unsigned char t = WH_PORT;
        WH_OE = 0;
        _delay_us(1);

        WH_OE = 1;
        _delay_us(1);
        WH_OE = 0;
        _delay_us(1);

        if ((t & 0x08) == 0) return false;
        _delay_us(1);
        
        wdts = 0xff;
    }
    return true;
}

void lcd_set_rs(unsigned v)
{
    WH_RS = v;
}

/***************** SPI *****************/

void spi_cs(bool val)
{
    p6_0 = !val;
}

void UART0_Tx_ISR(void)
{
    te_u0c1 = 0;
}

void UART0_Rx_ISR(void)
{
    u0rx = u0rb;
    ir_s0ric = 0;
    u0rx_f = true;
}

uint8_t spi_io(uint8_t b)
{
    u0rx_f = false;
    u0tb = b;
    te_u0c1 = 1;
    while (!u0rx_f);
    return u0rx;
}

bool eth_int(void)
{
    return !p6_4;
}

void eth_rst(void)
{
    p6_5 = 0;
    _delay_us(1);
    p6_5 = 1;
}

void TimerA3_ISR(void)
{
    if (ta3 & 0x8000)
    {
        enc[0]--;
    }
    else
    {
        enc[0]++;
    }
}

void TimerA4_ISR(void)
{
    if (ta4 & 0x8000)
    {
        enc[1]--;
    }
    else
    {
        enc[1]++;
    }
}

int32_t get_enc(uint8_t ch)
{
    uint16_t r[2] = {0};
    switch(ch)
    {
    case 0:
        r[0] = ta3;
        r[1] = enc[0];
        break;
    case 1:
        r[0] = ta4;
        r[1] = enc[1];
        break;
    default:
        break;
    }
    return *((int32_t*)&r);
}

