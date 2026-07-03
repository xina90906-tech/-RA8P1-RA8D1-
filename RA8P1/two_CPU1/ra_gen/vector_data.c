/* generated vector source file - do not edit */
#include "bsp_api.h"
/* Do not build these data structures if no interrupts are currently allocated because IAR will have build errors. */
#if VECTOR_DATA_IRQ_COUNT > 0
        BSP_DONT_REMOVE const fsp_vector_t g_vector_table[BSP_ICU_VECTOR_NUM_ENTRIES] BSP_PLACE_IN_SECTION(BSP_SECTION_APPLICATION_VECTORS) =
        {
                        [0] = sci_b_uart_rxi_isr, /* SCI9 RXI (Receive data full) */
            [1] = sci_b_uart_txi_isr, /* SCI9 TXI (Transmit data empty) */
            [2] = sci_b_uart_tei_isr, /* SCI9 TEI (Transmit end) */
            [3] = sci_b_uart_eri_isr, /* SCI9 ERI (Receive error) */
            [4] = sci_b_uart_rxi_isr, /* SCI6 RXI (Receive data full) */
            [5] = sci_b_uart_txi_isr, /* SCI6 TXI (Transmit data empty) */
            [6] = sci_b_uart_tei_isr, /* SCI6 TEI (Transmit end) */
            [7] = sci_b_uart_eri_isr, /* SCI6 ERI (Receive error) */
            [8] = gpt_counter_overflow_isr, /* GPT1 COUNTER OVERFLOW (Overflow) */
            [9] = gpt_counter_overflow_isr, /* GPT2 COUNTER OVERFLOW (Overflow) */
            [10] = sci_b_uart_rxi_isr, /* SCI0 RXI (Receive data full) */
            [11] = sci_b_uart_txi_isr, /* SCI0 TXI (Transmit data empty) */
            [12] = sci_b_uart_tei_isr, /* SCI0 TEI (Transmit end) */
            [13] = sci_b_uart_eri_isr, /* SCI0 ERI (Receive error) */
            [14] = agt_int_isr, /* AGT0 INT (AGT interrupt) */
            [15] = ipc_isr, /* IPC IRQ0 (CPU Mutual Interrupt 0) */
        };
        #if BSP_FEATURE_ICU_HAS_IELSR
        const bsp_interrupt_event_t g_interrupt_event_link_select[BSP_ICU_VECTOR_NUM_ENTRIES] =
        {
            [0] = BSP_PRV_VECT_ENUM(EVENT_SCI9_RXI,GROUP0), /* SCI9 RXI (Receive data full) */
            [1] = BSP_PRV_VECT_ENUM(EVENT_SCI9_TXI,GROUP1), /* SCI9 TXI (Transmit data empty) */
            [2] = BSP_PRV_VECT_ENUM(EVENT_SCI9_TEI,GROUP2), /* SCI9 TEI (Transmit end) */
            [3] = BSP_PRV_VECT_ENUM(EVENT_SCI9_ERI,GROUP3), /* SCI9 ERI (Receive error) */
            [4] = BSP_PRV_VECT_ENUM(EVENT_SCI6_RXI,GROUP4), /* SCI6 RXI (Receive data full) */
            [5] = BSP_PRV_VECT_ENUM(EVENT_SCI6_TXI,GROUP5), /* SCI6 TXI (Transmit data empty) */
            [6] = BSP_PRV_VECT_ENUM(EVENT_SCI6_TEI,GROUP6), /* SCI6 TEI (Transmit end) */
            [7] = BSP_PRV_VECT_ENUM(EVENT_SCI6_ERI,GROUP7), /* SCI6 ERI (Receive error) */
            [8] = BSP_PRV_VECT_ENUM(EVENT_GPT1_COUNTER_OVERFLOW,GROUP0), /* GPT1 COUNTER OVERFLOW (Overflow) */
            [9] = BSP_PRV_VECT_ENUM(EVENT_GPT2_COUNTER_OVERFLOW,GROUP1), /* GPT2 COUNTER OVERFLOW (Overflow) */
            [10] = BSP_PRV_VECT_ENUM(EVENT_SCI0_RXI,GROUP2), /* SCI0 RXI (Receive data full) */
            [11] = BSP_PRV_VECT_ENUM(EVENT_SCI0_TXI,GROUP3), /* SCI0 TXI (Transmit data empty) */
            [12] = BSP_PRV_VECT_ENUM(EVENT_SCI0_TEI,GROUP4), /* SCI0 TEI (Transmit end) */
            [13] = BSP_PRV_VECT_ENUM(EVENT_SCI0_ERI,GROUP5), /* SCI0 ERI (Receive error) */
            [14] = BSP_PRV_VECT_ENUM(EVENT_AGT0_INT,GROUP6), /* AGT0 INT (AGT interrupt) */
            [15] = BSP_PRV_VECT_ENUM(EVENT_IPC_IRQ0,GROUP7), /* IPC IRQ0 (CPU Mutual Interrupt 0) */
        };
        #endif
        #endif
