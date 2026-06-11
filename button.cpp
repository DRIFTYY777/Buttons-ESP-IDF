#include "button.h"
#include "driver/gpio.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"

static const char *TAG = "BUTTON";

// Static members initialization
bool BTN::isr_service_installed = false;

BTN::BTN(gpio_num_t pin)
{
    this->pin = pin;
    this->pressed = false;
    this->last_interrupt_time = 0;
    this->debounce_time_ms = 50;
    this->long_press_time_ms = 1000;
    this->callback = nullptr;
    this->task_handle = nullptr;
    this->event_queue = nullptr;
}

BTN::~BTN()
{
    disableInterrupt();
    if (task_handle != nullptr)
    {
        vTaskDelete(task_handle);
        task_handle = nullptr;
    }
    if (event_queue != nullptr)
    {
        vQueueDelete(event_queue);
        event_queue = nullptr;
    }
}

void IRAM_ATTR BTN::gpio_isr_handler(void *arg)
{
    BTN *btn = static_cast<BTN *>(arg);
    uint32_t current_time = xTaskGetTickCountFromISR();

    // Software debouncing in ISR
    if ((current_time - btn->last_interrupt_time) > pdMS_TO_TICKS(btn->debounce_time_ms))
    {
        btn->last_interrupt_time = current_time;

        ButtonEvent event;
        event.pin = btn->pin;
        event.timestamp = current_time;

        // Send event to this button's dedicated queue
        BaseType_t xHigherPriorityTaskWoken = pdFALSE;
        if (btn->event_queue != nullptr)
        {
            if (xQueueSendFromISR(btn->event_queue, &event, &xHigherPriorityTaskWoken) != pdTRUE)
            {
                // Queue full - try to make space by receiving and discarding oldest item
                ButtonEvent dummy;
                xQueueReceiveFromISR(btn->event_queue, &dummy, &xHigherPriorityTaskWoken);
                xQueueSendFromISR(btn->event_queue, &event, &xHigherPriorityTaskWoken);
            }
        }

        if (xHigherPriorityTaskWoken)
        {
            portYIELD_FROM_ISR();
        }
    }
}

void BTN::button_task(void *arg)
{
    BTN *btn = static_cast<BTN *>(arg);
    ButtonEvent event;
    uint32_t press_time = 0;
    bool is_pressed = false;
    bool long_press_triggered = false;

    while (true)
    {
        // Wait for event with timeout to check for long press
        TickType_t wait_time = is_pressed ? pdMS_TO_TICKS(50) : portMAX_DELAY;

        if (xQueueReceive(btn->event_queue, &event, wait_time))
        {
            if (event.pin == btn->pin)
            {
                // Check current button state
                bool current_state = (gpio_get_level(btn->pin) == 0);

                if (current_state && !is_pressed)
                {
                    // Button pressed
                    press_time = xTaskGetTickCount();
                    is_pressed = true;
                    long_press_triggered = false;
                    btn->pressed = true;

                    if (btn->callback)
                    {
                        btn->callback(BTN_EVENT_PRESSED);
                    }
                }
                else if (!current_state && is_pressed)
                {
                    // Button released
                    uint32_t release_time = xTaskGetTickCount();
                    uint32_t press_duration = release_time - press_time;
                    is_pressed = false;
                    btn->pressed = false;

                    if (btn->callback)
                    {
                        btn->callback(BTN_EVENT_RELEASED);

                        // Generate click event if press was short and long press wasn't triggered
                        if (!long_press_triggered && press_duration < pdMS_TO_TICKS(btn->long_press_time_ms))
                        {
                            btn->callback(BTN_EVENT_CLICK);
                        }
                    }
                    long_press_triggered = false;
                }
            }
        }
        else if (is_pressed && !long_press_triggered)
        {
            // Timeout occurred while button is pressed - check for long press
            uint32_t current_time = xTaskGetTickCount();
            uint32_t press_duration = current_time - press_time;

            if (press_duration >= pdMS_TO_TICKS(btn->long_press_time_ms))
            {
                // Still pressed after long press time
                if (gpio_get_level(btn->pin) == 0)
                {
                    long_press_triggered = true;
                    if (btn->callback)
                    {
                        btn->callback(BTN_EVENT_LONG_PRESS);
                    }
                }
            }
        }
    }
}

void BTN::begin(uint32_t debounce_ms, uint32_t long_press_ms)
{
    this->debounce_time_ms = debounce_ms;
    this->long_press_time_ms = long_press_ms;

    // Configure button GPIO pin as input with pull-up resistor
    gpio_reset_pin(pin);
    gpio_set_direction(pin, GPIO_MODE_INPUT);
    gpio_set_pull_mode(pin, GPIO_PULLUP_ONLY);

    // Install ISR service if not already installed
    if (!isr_service_installed)
    {
        esp_err_t ret = gpio_install_isr_service(0);
        if (ret == ESP_OK)
        {
            isr_service_installed = true;
            ESP_LOGI(TAG, "GPIO ISR service installed");
        }
        else if (ret != ESP_ERR_INVALID_STATE) // Already installed
        {
            ESP_LOGE(TAG, "Failed to install GPIO ISR service: %s", esp_err_to_name(ret));
            return;
        }
    }

    // Create event queue for this button
    if (event_queue == nullptr)
    {
        event_queue = xQueueCreate(10, sizeof(ButtonEvent));
        if (event_queue == nullptr)
        {
            ESP_LOGE(TAG, "Failed to create event queue for GPIO %d", pin);
            return;
        }
        ESP_LOGI(TAG, "Event queue created for GPIO %d", pin);
    }

    ESP_LOGI(TAG, "Button on GPIO %d initialized", pin);
}

void BTN::setCallback(btn_callback_t cb)
{
    this->callback = cb;

    // Create task if callback is set and task doesn't exist
    if (cb && task_handle == nullptr)
    {
        char task_name[16];
        snprintf(task_name, sizeof(task_name), "btn_%d", pin);
        xTaskCreate(
            button_task,
            task_name,
            4096, // stack size to 4KB
            this,
            5,
            &task_handle);
    }
}

void BTN::enableInterrupt()
{
    // Clear any pending interrupts first
    gpio_intr_disable(pin);

    // Small delay to let GPIO stabilize
    vTaskDelay(pdMS_TO_TICKS(10));

    // Set interrupt type for both edges to detect press and release
    gpio_set_intr_type(pin, GPIO_INTR_ANYEDGE);

    // Add ISR handler
    esp_err_t ret = gpio_isr_handler_add(pin, gpio_isr_handler, this);
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "Failed to add ISR handler for GPIO %d: %s", pin, esp_err_to_name(ret));
        return;
    }

    // Enable the interrupt
    gpio_intr_enable(pin);

    ESP_LOGI(TAG, "Interrupt enabled for GPIO %d", pin);
}

void BTN::disableInterrupt()
{
    gpio_intr_disable(pin);
    gpio_set_intr_type(pin, GPIO_INTR_DISABLE);
    gpio_isr_handler_remove(pin);

    ESP_LOGI(TAG, "Interrupt disabled for GPIO %d", pin);
}

bool BTN::isPressed()
{
    // Read the GPIO level; active low
    return gpio_get_level(pin) == 0;
}

void BTN::handleButtonEvent(uint32_t press_time, uint32_t release_time)
{
    // This can be used for additional custom event handling
    uint32_t duration = release_time - press_time;
    ESP_LOGI(TAG, "Button %d event: duration %lu ms", pin, duration * portTICK_PERIOD_MS);
}
