#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/uart.h>


#define SLEEP_TIME_IN_MS                     500
#define RECEIVE_BUFF_SIZE                    10
#define RECEIVE_TIMEOUT                      100


#define LED_NODE0                            DT_ALIAS(led0)
#define LED_NODE1                            DT_ALIAS(led1)
#define LED_NODE2                            DT_ALIAS(led2)

// Determine the GPIO device tree specifications for the specific pin
static const struct gpio_dt_spec led0 = GPIO_DT_SPEC_GET(LED_NODE0, gpios);
static const struct gpio_dt_spec led1 = GPIO_DT_SPEC_GET(LED_NODE1, gpios);
static const struct gpio_dt_spec led2 = GPIO_DT_SPEC_GET(LED_NODE2, gpios);


static const struct device *uart= DEVICE_DT_GET(DT_NODELABEL(usart3));

static uint8_t tx_buf[] =   {"Working on Zephyr Fundamental course\r\n"
   "Press 1-3 on your keyboard to toggle LEDS 1-3 on your Nucleo-F413ZH development kit\r\n"};

static uint8_t rx_buf[RECEIVE_BUFF_SIZE] = {0};

static void uart_cb(const struct device *dev, struct uart_event *evt, void *user_data)
{
   switch (evt->type)
   {

      case UART_RX_RDY:
         if ((evt->data.rx.len) == 1) {

            if (evt->data.rx.buf[evt->data.rx.offset] == '1')
            {
               gpio_pin_toggle_dt(&led0);
            }
            else if (evt->data.rx.buf[evt->data.rx.offset] == '2')
            {
               gpio_pin_toggle_dt(&led1);
            }
            else if (evt->data.rx.buf[evt->data.rx.offset] == '3')
            {
               gpio_pin_toggle_dt(&led2);
            }
         }
         break;
      case UART_RX_DISABLED:
         uart_rx_enable(dev, rx_buf, sizeof rx_buf, RECEIVE_TIMEOUT);
         break;

      default:
         break;
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
   ret = uart_callback_set(uart, uart_cb, NULL);
   if (ret) {
      return 1;
   }

   // Transmit some data on UART
   ret = uart_tx(uart, tx_buf, sizeof(tx_buf), SYS_FOREVER_US);
   if (ret) {
      return 1;
   }

   ret = uart_rx_enable(uart ,rx_buf,sizeof rx_buf,RECEIVE_TIMEOUT);
   if (ret) {
      return 1;
   }

   while(1)
   {
      k_msleep(SLEEP_TIME_IN_MS);
   }
}
