#ifndef BENCH_HOST_CLOCK_H
#define BENCH_HOST_CLOCK_H
typedef struct {int Iphi_f,Bphi_f,Pphi_f;} clock_frequency_t;
const clock_frequency_t *clock_freq(void);
#endif
