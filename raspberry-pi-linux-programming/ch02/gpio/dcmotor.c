#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>

#define GPIO_CHIP "/dev/gpiochip0"

/* TB6612FNG Control GPIO */
#define AIN1_GPIO 23
#define AIN2_GPIO 24
#define STBY_GPIO 25

static void motor_forward(struct gpiod_line_request *request)
{
    gpiod_line_request_set_value(request, AIN1_GPIO, GPIOD_LINE_VALUE_ACTIVE);
    gpiod_line_request_set_value(request, AIN2_GPIO, GPIOD_LINE_VALUE_INACTIVE);

    printf("Motor : Forward\n");
}

static void motor_reverse(struct gpiod_line_request *request)
{
    gpiod_line_request_set_value(request, AIN1_GPIO, GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_request_set_value(request, AIN2_GPIO, GPIOD_LINE_VALUE_ACTIVE);

    printf("Motor : Reverse\n");
}

static void motor_stop(struct gpiod_line_request *request)
{
    gpiod_line_request_set_value(request, AIN1_GPIO, GPIOD_LINE_VALUE_INACTIVE);
    gpiod_line_request_set_value(request, AIN2_GPIO, GPIOD_LINE_VALUE_INACTIVE);

    printf("Motor : Stop\n");
}

int main(void)
{
    struct gpiod_chip *chip;
    struct gpiod_line_settings *settings;
    struct gpiod_line_config *line_cfg;
    struct gpiod_request_config *req_cfg;
    struct gpiod_line_request *request;

    unsigned int offsets[] = {
        AIN1_GPIO,
        AIN2_GPIO,
        STBY_GPIO
    };

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

    gpiod_line_config_add_line_settings(line_cfg, offsets, 3, settings);

    gpiod_request_config_set_consumer(req_cfg, "tb6612-motor");

    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

    if (!request) {
        perror("gpiod_chip_request_lines");
        return 1;
    }

    /* TB6612FNG 활설화 */
    gpiod_line_request_set_value(request, STBY_GPIO, GPIOD_LINE_VALUE_ACTIVE);

    printf("=== DC Motor Test ===\n");

    motor_forward(request);
    sleep(2);

    motor_stop(request);
    sleep(1);

    motor_reverse(request);
    sleep(2);

    motor_stop(request);

    printf("Test Complete\n");

    gpiod_line_request_release(request);
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);

    return 0;
}