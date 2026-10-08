#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>

#define GPIO_CHIP "/dev/gpiochip0"
#define SERVO_GPIO 18

static struct gpiod_chip *chip;
static struct gpiod_line_request *request;

/* GPIO18 HIGH / LOW */
static void servo_gpio(int value)
{
    gpiod_line_request_set_value(request, SERVO_GPIO, value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

/* 서보모터에 PWM 신호 전송 */
static void servo_pulse(int pulse_us)
{
    /* HIGH */
    servo_gpio(1);
    usleep(pulse_us);

    /* LOW */
    servo_gpio(0);

    /* 전체 주기를 약 20ms로 맞춤 */
    usleep(20000 - pulse_us);
}

/* 원하는 각도로 이동 */
static void servo_angle(int angle)
{
    int pulse_us;
    int i;

    /* 
     * 0도      >   1500us
     * 90도     >   2000us
     * -90도    >   1000us
     */
    pulse_us = 1500 + (angle * 500 / 90);

    printf("Servo angle : %d degree\n", angle);

    /* 
     * 약 1초 동안 PWM 전송
     * 20ms x 50 = 약 1초
     */
    for (i = 0; i < 50; i++) {
        servo_pulse(pulse_us);
    }
}

int main(void)
{
    struct gpiod_line_settings *settings;
    struct gpiod_line_config *line_cfg;
    struct gpiod_request_config *req_cfg;

    unsigned int offsets[] = {
        SERVO_GPIO
    };

    /* GPIO chip 열기 */
    chip = gpiod_chip_open(GPIO_CHIP);

    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    /* GPIO 설정 객체 생성 */
    settings = gpiod_line_settings_new();
    line_cfg = gpiod_line_config_new();
    req_cfg = gpiod_request_config_new();

    if (!settings || !line_cfg || !req_cfg) {
        fprintf(stderr, "GPIO config allocation dailed\n");
        return 1;
    }

    /* GPIO18을 출력으로 설정 */
    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);

    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_INACTIVE);

    /* GPIO18 설정 적용 */
    gpiod_line_config_add_line_settings(line_cfg, offsets, 1, settings);

    gpiod_request_config_set_consumer(req_cfg, "servo");

    /* GPIO18 사용 요청 */
    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

    if (!request) {
        perror("gpiod_chip_request_lines");
        return 1;
    }

    printf("SG90 Servo Test Start\n");

    /* 0도 */
    servo_angle(-90);

    sleep(1);

    /* 90도 */
    servo_angle(0);

    sleep(1);

    /* 180도 */
    servo_angle(90);

    sleep(1);

    /* 다시 중양 */
    servo_angle(0);

    printf("Servo Test End\n");

    /* GPIO LOW */
    servo_gpio(0);

    /* GPIO 해제 */
    gpiod_line_request_release(request);

    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);

    gpiod_chip_close(chip);

    return 0;
}