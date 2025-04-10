#ifndef RTC_H
#define RTC_H

#include <stdint.h>

struct dt {
    uint8_t hour;
    uint8_t min;
    uint8_t sec;
    uint16_t year;
    uint8_t mon;
    uint8_t day;
};

void rtc_init();
void read_rtc(struct dt* dt);

extern struct dt dt;

#endif /* RTC_H */
