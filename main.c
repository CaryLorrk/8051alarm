#include<AT89x51.h>


#define LED0    P1_0
#define LED1    P1_1
#define LED2    P1_2
#define LED3    P1_3
#define LED4    P1_4
#define LED5    P1_5
#define LED6    P1_6
#define LED7    P1_7
#define LED_ON  0
#define LED_OFF 1

#define SEGMENT P0
#define SEG0    P2_0
#define SEG1    P2_1
#define SEG2    P2_2
#define SEG3    P2_3

#define B0      P2_4
#define B1      P2_5
#define B2      P2_6
#define B3      P2_7

#define SPK     P3_6
#define SPK_ON  0
#define SPK_OFF 1



struct Flags {
    unsigned t1ms:1;
    unsigned t5ms:1;
    unsigned t10ms:1;
    unsigned t20ms:1;
    unsigned t100ms:1;
    unsigned t200ms:1;
    unsigned t1s:1;

    unsigned b0_click:1;
    unsigned b1_click:1;
    unsigned b2_click:1;
    unsigned b3_click:1;

} flags;

enum State {
    CLOCK_RUN,
    SET_CLOCK_HOUR,
    SET_CLOCK_MIN,
    SET_ALARM_HOUR,
    SET_ALARM_MIN
} state;

unsigned char cnt1ms_for_5ms,
              cnt5ms_for_10ms,
              cnt10ms_for_20ms,
              cnt20ms_for_100ms,
              cnt100ms_for_200ms,
              cnt200ms_for_1s;


unsigned char seg_arr[10] = {0xc0,0xf9,0xa4,0Xb0,0x99,0x92,0x82,0xd8,0x80,0X90};
// 65535 - 1/fep/2*921583
unsigned char tune_high[8] = { 0, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFE, 0xFF};
unsigned char tune_low[8] = { 0, 0x46, 0x77, 0xA1, 0xB5, 0xD9, 0xF3, 0x16};
unsigned char song_bee[] = {
    5, 3, 3, 3, 4, 2, 2, 2, 1, 2, 3, 4, 5, 5, 5, 5,
    5, 3, 3, 3, 4, 2, 2, 2, 1, 3, 5, 5, 3, 3 ,3, 3,
    2, 2, 2, 2, 2, 3, 4, 4, 3, 3, 3, 3, 3, 4, 5, 5,
    5, 3, 3, 3, 4, 2, 2, 2, 1, 3, 5, 5, 1, 1, 1, 1};
unsigned char song_pos;
unsigned char tune_index;
unsigned char song_stop;
unsigned char  clock_hour, clock_min;
unsigned char  alarm_hour, alarm_min;
unsigned char alarm_enable;
unsigned char seg_number;

unsigned char button_status[4];


void init_8051(void);
void timer(void);
void reset_timer(void);
void check_button(void);
void display_seg(unsigned char hour, unsigned char min);
void check_state(void);
void clock_count_up(void);
void play_music(void);

void clock_run();
void set_clock_hour();
void set_clock_min();
void set_alarm_hour();
void set_alarm_min();

void main(void) {
    init_8051();
    while(1) {
        timer();
        if (flags.t200ms && LED6 == LED_ON) {
            flags.t200ms = 0;
            song_pos = (song_pos + 1) % sizeof(song_bee);
            tune_index = song_bee[song_pos];
        }
        switch(state) {
            case CLOCK_RUN:
                clock_run();
                break;
            case SET_CLOCK_HOUR:
                set_clock_hour();
                break;
            case SET_CLOCK_MIN:
                set_clock_min();
                break;
            case SET_ALARM_HOUR:
                set_alarm_hour();
                break;
            case SET_ALARM_MIN:
                set_alarm_min();
                break;
        }
    }
}

void init_8051(void) {
    TMOD |= 0x11;
    TH0 = 0xFC;
    TL0 = 0x66;
    TR0 = 1;
    EA = 1;
    ET0 = 1;
    LED0 = LED_ON;
}

void play_music() {
    song_pos = 0;
    tune_index = song_bee[song_pos];
    TH1 = tune_high[tune_index];
    TL1 = tune_low[tune_index];
    TR1 = 1;
    ET1 = 1;
    LED6 = LED_ON;
}

void stop_music() {
    TR1 = 0;
    ET1 = 0;
    SPK = SPK_OFF;
    LED6 = LED_OFF;
}

void timer(void) {
    if(flags.t1ms)
    {
        flags.t1ms = 0;
        cnt1ms_for_5ms++;
    }

    if(cnt1ms_for_5ms >= 5) {
        cnt1ms_for_5ms = 0;
        flags.t5ms = 1;
        cnt5ms_for_10ms++;
    }

    if(cnt5ms_for_10ms >= 2) {
        cnt5ms_for_10ms = 0;
        flags.t10ms = 1;
        cnt10ms_for_20ms++;
    }

    if(cnt10ms_for_20ms >= 2) {
        cnt10ms_for_20ms = 0;
        flags.t20ms = 1;
        cnt20ms_for_100ms++;
    }

    if(cnt20ms_for_100ms >= 5) {
        cnt20ms_for_100ms = 0;
        flags.t100ms = 1;
        cnt100ms_for_200ms++;
    }

    if(cnt100ms_for_200ms >= 2) {
        cnt100ms_for_200ms = 0;
        flags.t200ms = 1;
        cnt200ms_for_1s++;
    }

    if(cnt200ms_for_1s >= 5) {
        cnt200ms_for_1s = 0;
        flags.t1s = 1;
    }
}

void reset_timer(void) {
    flags.t1ms = 0;
    flags.t5ms = 0;
    flags.t10ms = 0;
    flags.t20ms = 0;
    flags.t100ms = 0;
    flags.t1s = 0;

    cnt1ms_for_5ms = 0;
    cnt5ms_for_10ms = 0;
    cnt10ms_for_20ms = 0;
    cnt20ms_for_100ms = 0;
    cnt200ms_for_1s = 0;
}

void check_button(void) {
    button_status[0] |= 0x01;
    button_status[1] |= 0x01;
    button_status[2] |= 0x01;
    button_status[3] |= 0x01;

    if(B0 == 0 && B1 && B2 && B3) {
        button_status[0] &= 0xFE;
    }
    if(B1 == 0 && B0 && B2 && B3) {
        button_status[1] &= 0xFE;
    }
    if(B2 == 0 && B0 && B1 && B3) {
        button_status[2] &= 0xFE;
    }
    if(B3 == 0 && B0 && B1 && B2) {
        button_status[3] &= 0xFE;
    }

    flags.b0_click = button_status[0] == 0xFC ? 1 : 0;
    flags.b1_click = button_status[1] == 0xFC ? 1 : 0;
    flags.b2_click = button_status[2] == 0xFC ? 1 : 0;
    flags.b3_click = button_status[3] == 0xFC ? 1 : 0;


    button_status[0] <<= 1;
    button_status[1] <<= 1;
    button_status[2] <<= 1;
    button_status[3] <<= 1;
}

void display_seg(unsigned char hour, unsigned char min) {
    volatile unsigned char temp_hour = hour, 
                           temp_min = min;
    
    seg_number++;
    seg_number = seg_number % 4;

    SEGMENT = 0xff;
    SEG0 = 1;
    SEG1 = 1;
    SEG2 = 1;
    SEG3 = 1;

    switch(seg_number) {
        case 0:
            SEGMENT = seg_arr[temp_hour / 10];
            SEG0 = 0;
            break;
        case 1:
            SEGMENT = seg_arr[temp_hour % 10];
            SEG1 = 0;
            break;
        case 2:
            SEGMENT = seg_arr[temp_min / 10];
            SEG2 = 0;
            break;
        case 3:
            SEGMENT = seg_arr[temp_min % 10];
            SEG3 = 0;
            break;
        default:
            SEGMENT = 0xff;
            SEG0 = 1;
            SEG1 = 1;
            SEG2 = 1;
            SEG3 = 1;
    }
}

void clock_count_up(void) {
    clock_min++;
    if(clock_min >= 60) {
        clock_hour++;
        clock_min = 0;
    }
    if(clock_hour >= 24) {
        clock_hour = 0;
    }
}

void clock_run() {
    if(flags.t5ms) {
        flags.t5ms = 0;
        display_seg(clock_hour, clock_min);
    }

    if(flags.t20ms) {
        flags.t20ms = 0;
        check_button();
        if(flags.b0_click) {
            LED0 = LED_OFF;
            LED1 = LED_ON;
            state = SET_CLOCK_HOUR;
        }
        else if(flags.b1_click) {
            LED0 = LED_OFF;
            LED3 = LED_ON;
            state = SET_ALARM_HOUR;
        }
        else if(flags.b2_click) {
            stop_music();
        }
        else if(flags.b3_click) {
            LED7 = ~LED7;
            alarm_enable = ~alarm_enable;
        }
    }
    if(flags.t1s) {
        flags.t1s = 0;
        clock_count_up();
        if (alarm_enable) {
            if (clock_hour == alarm_hour &&
                clock_min == alarm_min) {
                play_music();
            }
        }
    }
}

void set_clock_hour() {
    if(flags.t5ms) {
        flags.t5ms = 0;
        display_seg(clock_hour, clock_min);
    }

    if(flags.t20ms) {
        flags.t20ms = 0;
        check_button();
        if(flags.b0_click) {
            LED1 = LED_OFF;
            LED0 = LED_ON;
            state = CLOCK_RUN;
            reset_timer();
        }
        else if(flags.b1_click) {
            LED1 = LED_OFF;
            LED2 = LED_ON;
            state = SET_CLOCK_MIN;
        }
        else if(flags.b2_click) {
            if (clock_hour <= 0) {
                clock_hour = 23;
            } else{
                clock_hour--;
            }
        }
        else if(flags.b3_click) {
            if (clock_hour >= 23) {
                clock_hour = 0;
            } else {
                clock_hour++;
            }
        }
    }
}

void set_clock_min() {
    if(flags.t5ms) {
        flags.t5ms = 0;
        display_seg(clock_hour, clock_min);
    }

    if(flags.t20ms) {
        flags.t20ms = 0;
        check_button();
        if(flags.b0_click) {
            LED2 = LED_OFF;
            LED0 = LED_ON;
            state = CLOCK_RUN;
            reset_timer();
        }
        else if(flags.b1_click) {
            LED2 = LED_OFF;
            LED1 = LED_ON;
            state = SET_CLOCK_HOUR;
        }
        else if(flags.b2_click) {
            if (clock_min <= 0) {
                clock_min = 59;
            } else {
                clock_min--;
            }
        }
        else if(flags.b3_click) {
            if (clock_min >= 59) {
                clock_min = 0;
            } else {
                clock_min++;
            }
        }
    }
}

void set_alarm_hour() {
    if(flags.t5ms) {
        flags.t5ms = 0;
        display_seg(alarm_hour, alarm_min);
    }

    if(flags.t20ms) {
        flags.t20ms = 0;
        check_button();
        if(flags.b0_click) {
            LED3 = LED_OFF;
            LED0 = LED_ON;
            state = CLOCK_RUN;
            reset_timer();
        }
        else if(flags.b1_click) {
            LED3 = LED_OFF;
            LED4 = LED_ON;
            state = SET_ALARM_MIN;
        }
        else if(flags.b2_click) {
            if (alarm_hour <= 0) {
                alarm_hour = 23;
            } else{
                alarm_hour--;
            }
        }
        else if(flags.b3_click) {
            if (alarm_hour >= 23) {
                alarm_hour = 0;
            } else {
                alarm_hour++;
            }
        }
    }
    if(flags.t1s) {
        flags.t1s = 0;
        clock_count_up();
    }
}

void set_alarm_min() {
    if(flags.t5ms) {
        flags.t5ms = 0;
        display_seg(alarm_hour, alarm_min);
    }

    if(flags.t20ms) {
        flags.t20ms = 0;
        check_button();
        if(flags.b0_click) {
            LED4 = LED_OFF;
            LED0 = LED_ON;
            state = CLOCK_RUN;
            reset_timer();
        }
        else if(flags.b1_click) {
            LED4 = LED_OFF;
            LED3 = LED_ON;
            state = SET_ALARM_HOUR;
        }
        else if(flags.b2_click) {
            if (alarm_min <= 0) {
                alarm_min = 59;
            } else {
                alarm_min--;
            }
        }
        else if(flags.b3_click) {
            if (alarm_min >= 59) {
                alarm_min = 0;
            } else {
                alarm_min++;
            }
        }
    }
    if(flags.t1s) {
        flags.t1s = 0;
        clock_count_up();
    }
}

void timer0(void) interrupt 1 {
    TH0 = 0xFC;
    TL0 = 0x66;
    TF0 = 0;
    flags.t1ms = 1;
}

void timer1(void) interrupt 3 {
    TH1 = tune_high[tune_index];
    TL1 = tune_low[tune_index];
    TF1 = 0;
    SPK = ~SPK;
}
