#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>


#define SLEEP_TIME_IN_MS                     500


#define LED_NODE0                            DT_ALIAS(led0)
#define LED_NODE1                            DT_ALIAS(led1)
#define LED_NODE2                            DT_ALIAS(led2)

// Determine the GPIO device tree specifications for the specific pin
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED_NODE0, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED_NODE1, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED_NODE2, gpios);


static const struct device *uart= DEVICE_DT_GET(DT_NODELABEL(usart3));

static const uint8_t tx_buf[] =   {"Working on Zephyr Fundamental course\r\n"
   "Press 1-3 on your keyboard to toggle LEDS 1-3 on your Nucleo-F413ZH development kit\r\n"};

static const uint8_t rx_mismatched_buf[] =   {"Invalid Number for LED opearation\r\n"};


static void uart_send_string(const struct device *dev, const char *str)
{
    while (*str) {
        uart_poll_out(dev, *str++);
    }
}


static void uart_cb(const struct device *dev, void *user_data)
{
   uint8_t byte;

   if (!uart_irq_update(dev))
   {
      return;
   }

   while (uart_irq_rx_ready(dev))
   {

      if (uart_fifo_read(dev, &byte, 1) == 1)
      {
         /* Optional: echo back */
         uart_fifo_fill(dev, &byte, 1);

         if(byte == '1')
         {
            gpio_pin_toggle_dt(&led0);
         }
         else if(byte == '2')
         {
            gpio_pin_toggle_dt(&led1);
         }
         else if(byte == '3')
         {
            gpio_pin_toggle_dt(&led2);
         }
         else
         {
            printk("%s", &rx_mismatched_buf[0]);
         }

      }
   }
}


int main()
{
   int ret = 0;

   if (!(gpio_is_ready_dt(&led0) && gpio_is_ready_dt(&led1)
                                       && gpio_is_ready_dt(&led2)))
   {
      printk("One or the other LED is not ready\r\n");
   }
   else
   {
      printk("All LEDs are ready\r\n");
   }

   ret = gpio_pin_configure_dt(&led0, GPIO_OUTPUT_ACTIVE);
   ret = gpio_pin_configure_dt(&led1, GPIO_OUTPUT_ACTIVE);
   ret = gpio_pin_configure_dt(&led2, GPIO_OUTPUT_ACTIVE);

   if(!device_is_ready(uart))
   {
      printk("UART device not ready\r\n");
   }
   else
   {
      printk("UART device is ready\r\n");
   }

   // Register UART callback
   // ret = uart_callback_set(uart, uart_cb, NULL);
   // if (ret) {
   //    return 1;
   // }
   ret = uart_irq_callback_user_data_set(uart, uart_cb, NULL);
   if (ret) {
      return 1;
   }
   printk("UART Callback setting successful\r\n");

   // Transmit some data on UART
   // ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
   uart_send_string(uart, tx_buf);

   printk("UART Tx successful\r\n");

   uart_irq_rx_enable(uart);

   printk("UART Rx Enbled\r\n");

   while(1)
   {
      k_msleep(SLEEP_TIME_IN_MS);
   }
}
