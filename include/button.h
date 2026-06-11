#if !defined(BUTTON)
#define BUTTON

#include "soc/gpio_num.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <functional>

// Button event types
typedef enum
{
    BTN_EVENT_PRESSED,
    BTN_EVENT_RELEASED,
    BTN_EVENT_CLICK,
    BTN_EVENT_LONG_PRESS
} btn_event_t;

// Callback function type
typedef std::function<void(btn_event_t)> btn_callback_t;

class BTN
{
private:
    gpio_num_t pin;
    volatile bool pressed;
    volatile uint32_t last_interrupt_time;
    uint32_t debounce_time_ms;
    uint32_t long_press_time_ms;
    btn_callback_t callback;
    TaskHandle_t task_handle;
    QueueHandle_t event_queue; // Each button has its own queue

    static bool isr_service_installed;

    struct ButtonEvent
    {
        gpio_num_t pin;
        uint32_t timestamp;
    };

    static void IRAM_ATTR gpio_isr_handler(void *arg);
    [[noreturn]] static void button_task(void *arg);
    void handleButtonEvent(uint32_t press_time, uint32_t release_time);

public:
    BTN(gpio_num_t pin);
    ~BTN();

    void begin(uint32_t debounce_ms = 50, uint32_t long_press_ms = 1000);
    void setCallback(btn_callback_t cb);
    bool isPressed();
    void enableInterrupt();
    void disableInterrupt();
};

#endif // BUTTON
