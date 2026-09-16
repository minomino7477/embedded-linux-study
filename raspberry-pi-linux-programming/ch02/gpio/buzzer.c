#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>

#define GPIO_CHIP "/dev/gpiochip0"
#define BUZZER_GPIO 12

void buzzer_beep(struct gpiod_line_request *request, int time_ms)
{
    /* 부저 ON */
    gpiod_line_request_set_value(request, BUZZER_GPIO, GPIOD_LINE_VALUE_ACTIVE);

    usleep(time_ms * 1000);

    /* 부저 OFF */
    gpiod_line_request_set_value(request, BUZZER_GPIO, GPIOD_LINE_VALUE_INACTIVE);
}

int main(void)
{
    struct gpiod_chip *chip;
    struct gpiod_line_settings *settings;
    struct gpiod_line_config *line_cfg;
    struct gpiod_request_config *req_cfg;
    struct gpiod_line_request *request;

    unsigned int offset = BUZZER_GPIO;

    chip = gpiod_chip_open(GPIO_CHIP);
    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    settings = gpiod_line_settings_new();
    line_cfg = gpiod_line_config_new();
    req_cfg = gpiod_request_config_new();

    if (!settings || !line_cfg || !req_cfg) {
        fprintf(stderr, "GPIO 설정 생성 실패 \n");
        return 1;
    }

    /* GPIO18을 출력으로 설정 */
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);

    gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings);

    gpiod_request_config_set_consumer(req_cfg, "buzzer");

    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);
    if (!request) {
        perror("gpiod_chip_request_lines");
        return 1;
    }

    printf("Buzzer Start\n");

    /* 짧게 3번 */
    for (int i = 0 ; i < 3 ; i++) {
        buzzer_beep(request, 200);
        usleep(200000);
    }

    /* 길게 1번 */
    buzzer_beep(request, 1000);

    printf("Buzzer End\n");

    gpiod_line_request_release(request);
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);

    return 0;
}