#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>

#define GPIO_CHIP "/dev/gpiochip0"

#define LED_GPIO    18
#define BUZZER_GPIO 12
#define CDS_GPIO    17

int main(void)
{
    struct gpiod_chip *chip;

    struct gpiod_line_settings *led_settings;
    struct gpiod_line_settings *buzzer_settings;
    struct gpiod_line_settings *cds_settings;

    struct gpiod_line_config *line_cfg;
    struct gpiod_request_config *req_cfg;
    struct gpiod_line_request *request;

    unsigned int led_offset = LED_GPIO;
    unsigned int buzzer_offset = BUZZER_GPIO;
    unsigned int cds_offset = CDS_GPIO;

    chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    led_settings = gpiod_line_settings_new();
    buzzer_settings = gpiod_line_settings_new();
    cds_settings = gpiod_line_settings_new();

    line_cfg = gpiod_line_config_new();
    req_cfg = gpiod_request_config_new();

    if (!led_settings || !buzzer_settings || !cds_settings || !line_cfg || !req_cfg) {
        fprintf(stderr, "GPIO 설정 생성 실패\n");
        return 1;
    }

    /* LED 출력 */
    gpiod_line_settings_set_direction(led_settings, GPIOD_LINE_DIRECTION_OUTPUT);

    /* 부저 출력 */
    gpiod_line_settings_set_direction(buzzer_settings, GPIOD_LINE_DIRECTION_OUTPUT);

    /* 조도센서 입력 */
    gpiod_line_settings_set_direction(cds_settings, GPIOD_LINE_DIRECTION_INPUT);

    /* 각 GPIO에 설정 적용 */
    gpiod_line_config_add_line_settings(line_cfg, &led_offset, 1, led_settings);

    gpiod_line_config_add_line_settings(line_cfg, &buzzer_offset, 1, buzzer_settings);

    gpiod_line_config_add_line_settings(line_cfg, &cds_offset, 1, cds_settings);

    gpiod_request_config_set_consumer(req_cfg, "cds-control");

    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    if (!request) {
        perror("gpiod_chip_request_lines");
        return 1;
    }

    printf("조도센서 제어 시작\n");

    while (1) {

        enum gpiod_line_value cds_value;

        cds_value = gpiod_line_request_get_value(request, CDS_GPIO);

        printf("\rGPIO17: %d  ", cds_value);
        fflush(stdout);

        usleep(100000);
        
        // if (cds_value == GPIOD_LINE_VALUE_ACTIVE) {

            /* 밝을 때 */
            // gpiod_line_request_set_value(request, LED_GPIO, GPIOD_LINE_VALUE_INACTIVE);

            // gpiod_line_request_set_value(request, BUZZER_GPIO, GPIOD_LINE_VALUE_INACTIVE);

            // printf("\r밝음 -> LED OFF / BUZZER OFF ");

        // } else {

            /* 어두울 때 */
            // gpiod_line_request_set_value(request, LED_GPIO, GPIOD_LINE_VALUE_ACTIVE);

            // gpiod_line_request_set_value(request, BUZZER_GPIO, GPIOD_LINE_VALUE_ACTIVE);

            // printf("\r어두움 -> LED ON / BUZZER ON ");

        // }
        
        // cds_value = gpiod_line_request_get_value(request, CDS_GPIO);
        // printf("\rCDS %d", cds_value);
        // fflush(stdout);

        // usleep(100000);
    }

    return 0;
}