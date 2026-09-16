#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>

#define GPIO_CHIP "/dev/gpiochip0"
#define GPIO_LINE 17

#define PERIOD_US 1000      // PWM 주기 1ms = 1KHz
#define STEP_DELAY_US 20000

void softWare_pwm(struct gpiod_line_request *request, int duty)
{
    int on_time;
    int off_time;

    on_time = PERIOD_US * duty / 100;
    off_time = PERIOD_US - on_time;

    if (on_time > 0) {
        gpiod_line_request_set_value(request, GPIO_LINE, GPIOD_LINE_VALUE_ACTIVE);

        usleep(on_time);
    }

    if (off_time > 0) {
        gpiod_line_request_set_value(request, GPIO_LINE, GPIOD_LINE_VALUE_INACTIVE);

        usleep(off_time);
    }
}

int main(void)
{
    struct gpiod_chip *chip;
    struct gpiod_line_settings *settings;
    struct gpiod_line_config *line_cfg;
    struct gpiod_request_config *req_cfg;
    struct gpiod_line_request *request;

    unsigned int offset = GPIO_LINE;

    chip = gpiod_chip_open(GPIO_CHIP);

    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    settings = gpiod_line_settings_new();
    line_cfg = gpiod_line_config_new();
    req_cfg = gpiod_request_config_new();

    if (!settings || !line_cfg || !req_cfg) {
        fprintf(stderr, "GPIO 설정 생성 실패\n");
        return 1;
    }

    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);

    gpiod_line_config_add_line_settings(line_cfg, &offset, 1, settings);

    gpiod_request_config_set_consumer(req_cfg, "pwmled");

    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

    if (!request) {
        perror("gpiod_chip_request_lines");
        return 1;
    }

    while (1) {
        // 점점 밝게
        for (int duty = 0 ; duty <= 100 ; duty++) {
            printf("\r밝기: %3d%%", duty);
            fflush(stdout);
            
            for (int i = 0 ; i < 20 ; i++) {
                softWare_pwm(request, duty);
            }
        }

        // 점점 어둡게
        for (int duty = 100 ; duty >= 0 ; duty--) {
            printf("\r밝기: %3d%%", duty);
            fflush(stdout);

            for (int i = 0 ; i < 20 ; i++) {
                softWare_pwm(request, duty);
            }
        }
    }

    gpiod_line_request_release(request);
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);

    return 0;

}