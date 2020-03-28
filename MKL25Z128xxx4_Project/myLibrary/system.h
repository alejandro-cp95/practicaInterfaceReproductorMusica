
#ifndef SYSTEM_H_
#define SYSTEM_H_

/*! Macro to convert a 1/4 microsecond period to raw count value */
#define PORT_PCR_PSPE_MASK                       (0x3U)
#define PORT_PCR_PSPE_SHIFT                      (0U)
#define PORT_PCR_PSPE(x)                         (((uint32_t)(((uint32_t)(x)) << PORT_PCR_PSPE_SHIFT)) & PORT_PCR_PSPE_MASK)

static inline void PORT_SetPinPullConfig(PORT_Type *base, uint32_t pin, enum _port_pull config)
{
    base->PCR[pin] = (base->PCR[pin] & ~PORT_PCR_PS_MASK) | PORT_PCR_PSPE(config);
}

#endif /* SYSTEM_H_ */
