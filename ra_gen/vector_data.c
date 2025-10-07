/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = sci_uart_rxi_isr, /* SCI9 RXI (Receive data full) */
            [1] = sci_uart_txi_isr, /* SCI9 TXI (Transmit data empty) */
            [2] = sci_uart_tei_isr, /* SCI9 TEI (Transmit end) */
            [3] = sci_uart_eri_isr, /* SCI9 ERI (Receive error) */
            [4] = r_icu_isr, /* ICU IRQ7 (External pin interrupt 7) */
            [5] = sci_spi_rxi_isr, /* SCI0 RXI (Receive data full) */
            [6] = sci_spi_txi_isr, /* SCI0 TXI (Transmit data empty) */
            [7] = sci_spi_tei_isr, /* SCI0 TEI (Transmit end) */
            [8] = sci_spi_eri_isr, /* SCI0 ERI (Receive error) */
            [9] = sci_i2c_txi_isr, /* SCI2 TXI (Transmit data empty) */
            [10] = sci_i2c_tei_isr, /* SCI2 TEI (Transmit end) */
            [11] = adc_scan_end_isr, /* ADC0 SCAN END (End of A/D scanning operation) */
            [12] = ctsu_write_isr, /* CTSU WRITE (Write request interrupt) */
            [13] = ctsu_read_isr, /* CTSU READ (Measurement data transfer request interrupt) */
            [14] = ctsu_end_isr, /* CTSU END (Measurement end interrupt) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_SCI9_RXI,GROUP0), /* SCI9 RXI (Receive data full) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SCI9_TXI,GROUP1), /* SCI9 TXI (Transmit data empty) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SCI9_TEI,GROUP2), /* SCI9 TEI (Transmit end) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI9_ERI,GROUP3), /* SCI9 ERI (Receive error) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_ICU_IRQ7,GROUP4), /* ICU IRQ7 (External pin interrupt 7) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_SCI0_RXI,GROUP5), /* SCI0 RXI (Receive data full) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_SCI0_TXI,GROUP6), /* SCI0 TXI (Transmit data empty) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_SCI0_TEI,GROUP7), /* SCI0 TEI (Transmit end) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_SCI0_ERI,GROUP0), /* SCI0 ERI (Receive error) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_SCI2_TXI,GROUP1), /* SCI2 TXI (Transmit data empty) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_SCI2_TEI,GROUP2), /* SCI2 TEI (Transmit end) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_ADC0_SCAN_END,GROUP3), /* ADC0 SCAN END (End of A/D scanning operation) */
            [12] = BSP_PRV_VECT_ENUM(EVENT_CTSU_WRITE,GROUP4), /* CTSU WRITE (Write request interrupt) */
            [13] = BSP_PRV_VECT_ENUM(EVENT_CTSU_READ,GROUP5), /* CTSU READ (Measurement data transfer request interrupt) */
            [14] = BSP_PRV_VECT_ENUM(EVENT_CTSU_END,GROUP6), /* CTSU END (Measurement end interrupt) */
        };
        #endif
        #endif
