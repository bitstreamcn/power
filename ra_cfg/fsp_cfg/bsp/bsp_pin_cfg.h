/* generated configuration header file - do not edit */
#ifndef BSP_PIN_CFG_H_
#define BSP_PIN_CFG_H_
#include "r_ioport.h"

/* Common macro for FSP header files. There is also a corresponding FSP_FOOTER macro at the end of this file. */
FSP_HEADER

#define LED_D2 (BSP_IO_PORT_00_PIN_02)
#define MISO (BSP_IO_PORT_01_PIN_00)
#define SPI_CS (BSP_IO_PORT_01_PIN_03)
#define DC (BSP_IO_PORT_01_PIN_04)
#define RES (BSP_IO_PORT_05_PIN_00)

extern const ioport_cfg_t g_bsp_pin_cfg; /* R7FA4M2AD3CFL.pincfg */

void BSP_PinConfigSecurityInit();

/* Common macro for FSP header files. There is also a corresponding FSP_HEADER macro at the top of this file. */
FSP_FOOTER
#endif /* BSP_PIN_CFG_H_ */
