/*
 * Dual UART Test for RAK3172
 * USART1 (PB6/PB7): Console to PC
 * USART2 (PA2/PA3): GPS communication
 */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(dual_uart_test, LOG_LEVEL_INF);

/* LED setup if available */
#define LED0_NODE DT_ALIAS(led0)
static const struct gpio_dt_spec led = GPIO_DT_SPEC_GET_OR(LED0_NODE, gpios, {0});

/* GPS UART buffer */
static char gps_rx_buf[256];
static int gps_rx_buf_pos = 0;

static void gps_uart_rx_callback(const struct device *dev, void *user_data)
{
    uint8_t c;
    
    if (!uart_irq_update(dev)) {
        return;
    }
    
    if (!uart_irq_rx_ready(dev)) {
        return;
    }
    
    /* Read characters from GPS */
    while (uart_fifo_read(dev, &c, 1) == 1) {
        if (c == '\r' || c == '\n') {
            if (gps_rx_buf_pos > 0) {
                gps_rx_buf[gps_rx_buf_pos] = '\0';
                /* Print complete NMEA sentence to console */
                if (gps_rx_buf[0] == '$') {
                    LOG_INF("GPS: %s", gps_rx_buf);
                }
                gps_rx_buf_pos = 0;
            }
        } else if (gps_rx_buf_pos < (sizeof(gps_rx_buf) - 1)) {
            gps_rx_buf[gps_rx_buf_pos++] = c;
        }
    }
}

int main(void)
{
    const struct device *gps_uart;
    bool led_state = false;
    int counter = 0;
    
    /* Setup LED if available */
    if (gpio_is_ready_dt(&led)) {
        gpio_pin_configure_dt(&led, GPIO_OUTPUT_ACTIVE);
        LOG_INF("LED ready");
    }
    
    LOG_INF("=== RAK3172 Dual UART Test ===");
    LOG_INF("Console: USART1 (PB6/PB7) - you're seeing this!");
    LOG_INF("GPS:     USART2 (PA2/PA3) - will read NMEA data");
    
    /* Setup GPS UART */
    gps_uart = DEVICE_DT_GET(DT_ALIAS(gps_uart));
    if (!device_is_ready(gps_uart)) {
        LOG_ERR("GPS UART (USART2) not ready");
        return -ENODEV;
    }
    
    /* Configure GPS UART for interrupt-driven operation */
    uart_irq_callback_user_data_set(gps_uart, gps_uart_rx_callback, NULL);
    uart_irq_rx_enable(gps_uart);
    
    LOG_INF("GPS UART ready on USART2 (PA2/PA3)");
    LOG_INF("Connect LC86G: PA2→LC86G_RX, PA3→LC86G_TX, GND, 3V3");
    
    /* Send wake-up command to GPS */
    const char *wakeup_cmd = "$PAIR001*3C\r\n";
    uart_fifo_fill(gps_uart, (uint8_t*)wakeup_cmd, strlen(wakeup_cmd));
    LOG_INF("Sent wake-up command to GPS");
    
    LOG_INF("Starting main loop...");
    
    while (1) {
        /* Blink LED */
        if (gpio_is_ready_dt(&led)) {
            gpio_pin_set_dt(&led, led_state);
            led_state = !led_state;
        }
        
        counter++;
        LOG_INF("[%d] Console on USART1 working. Waiting for GPS data on USART2...", counter);
        
        if (counter % 10 == 0) {
            /* Send periodic command to GPS */
            const char *status_cmd = "$PAIR062*35\r\n";  // Query status
            uart_fifo_fill(gps_uart, (uint8_t*)status_cmd, strlen(status_cmd));
            LOG_INF("Sent status query to GPS");
        }
        
        k_sleep(K_SECONDS(3));
    }
    
    return 0;
}