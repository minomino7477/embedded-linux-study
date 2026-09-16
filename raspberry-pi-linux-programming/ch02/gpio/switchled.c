#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>

#define GPIO_CHIP "/dev/gpiochip0"

#define LED_GPIO    18
#define SW_GPIO     24

int main(void)
{
    struct gpiod_chip *chip;

    struct gpiod_line_settings *led_settings;
    struct gpiod_line_settings *sw_settings;

    struct gpiod_line_config *line_cfg;
    struct gpiod_request_config *req_cfg;
    struct gpiod_line_request *request;

    unsigned int led_offset = LED_GPIO;
    unsigned int sw_offset = SW_GPIO;

    chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    led_settings = gpiod_line_settings_new();
    sw_settings = gpiod_line_settings_new();
    line_cfg = gpiod_line_config_new();
    req_cfg = gpiod_request_config_new();

    if (!led_settings || !sw_settings || !line_cfg || !req_cfg) {
        fprintf(stderr, "GPIO 설정 생성 실패\n");
        return 1;
    }

    /* LED: 출력 */
    gpiod_line_settings_set_direction(led_settings, GPIOD_LINE_DIRECTION_OUTPUT);

    /* 스위치: 입력 */
    gpiod_line_settings_set_direction(sw_settings, GPIOD_LINE_DIRECTION_INPUT);

    /* GPIO18에 LED 설정 적용 */
    gpiod_line_config_add_line_settings(line_cfg, &led_offset, 1, led_settings);

    /* GPIO24에 스위치 설정 적용 */
    gpiod_line_config_add_line_settings(line_cfg, &sw_offset, 1, sw_settings);

    gpiod_request_config_set_consumer(req_cfg, "switch-led");

    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    if (!request) {
        perror("gpiod_chip_request_lines");
        return 1;
    }

    printf("스위치 입력 시작\n");

    while (1) {
        enum gpiod_line_value sw_value;

        sw_value = gpiod_line_request_get_value(request, SW_GPIO);

        printf("\rSW = %d", sw_value);
        fflush(stdout);

        /*
         * 풀업 회로
         * 스위치 OFF: HIGH
         * 스위치 ON: LOW
        */

        if (sw_value == GPIOD_LINE_VALUE_INACTIVE) {
            gpiod_line_request_set_value(request, LED_GPIO, GPIOD_LINE_VALUE_ACTIVE);
            printf("\rSwitch ON -> LED ON ");
        } else {
            gpiod_line_request_set_value(request, LED_GPIO, GPIOD_LINE_VALUE_INACTIVE);
            printf("\rSwitch OFF -> LED OFF");
        }

        fflush(stdout);
        usleep(10000);
    }

    return 0;
}