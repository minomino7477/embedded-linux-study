#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>

#define GPIO_CHIP "/dev/gpiochip0"

/* 7-Segment GPIO */
#define SEG_A_GPIO 5
#define SEG_B_GPIO 6
#define SEG_C_GPIO 13
#define SEG_D_GPIO 19
#define SEG_E_GPIO 26
#define SEG_F_GPIO 20
#define SEG_G_GPIO 21

/*
 * 공통 애노드(Common Anode)
 *
 * 0 = LED ON   (LOW)
 * 1 = LED OFF  (HIGH)
 * 
 * 배열 순서: a, b, c, d, e, f, g
 */
static const int number[10][7] = {
    {0, 0, 0, 0, 0, 0, 1},  /* 0 */
    {1, 0, 0, 1, 1, 1, 1},  /* 1 */
    {0, 0, 1, 0, 0, 1, 0},  /* 2 */
    {0, 0, 0, 0, 1, 1, 0},  /* 3 */
    {1, 0, 0, 1, 1, 0, 0},  /* 4 */
    {0, 1, 0, 0, 1, 0, 0},  /* 5 */
    {0, 1, 0, 0, 0, 0, 0},  /* 6 */
    {0, 0, 0, 1, 1, 1, 1},  /* 7 */
    {0, 0, 0, 0, 0, 0, 0},  /* 8 */
    {0, 0, 0, 0, 1, 0, 0}   /* 9 */
};

static void display_number(struct gpiod_line_request *request, unsigned int offsets[], int num)
{
    for (int i = 0; i < 7; i++) {
        enum gpiod_line_value value;

        if (number[num][i] == 0) {
            value = GPIOD_LINE_VALUE_INACTIVE;
        } else {
            value = GPIOD_LINE_VALUE_ACTIVE;
        }

        gpiod_line_request_set_value(request, offsets[i], value);
    }
}

static void display_off(struct gpiod_line_request *request, unsigned int offsets[])
{
    for (int i = 0; i < 7; i++) {
        gpiod_line_request_set_value(request, offsets[i], GPIOD_LINE_VALUE_ACTIVE);
    }
}

int main(void)
{
    struct gpiod_chip *chip;
    struct gpiod_line_settings *settings;
    struct gpiod_line_config *line_cfg;
    struct gpiod_request_config *req_cfg;
    struct gpiod_line_request *request;

    unsigned int offsets[] = {
        SEG_A_GPIO,
        SEG_B_GPIO,
        SEG_C_GPIO,
        SEG_D_GPIO,
        SEG_E_GPIO,
        SEG_F_GPIO,
        SEG_G_GPIO
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

    gpiod_line_config_add_line_settings(line_cfg, offsets, 7, settings);

    gpiod_request_config_set_consumer(req_cfg, "seven-segment");

    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

    if (!request) {
        perror("gpiod_chip_request_lines");
        return 1;
    }

    printf("=== 7-Segment Test ===\n");

    /* 0 ~ 9 출력 */
    for (int num = 0; num <= 9; num++) {
        printf("Number: %d\n", num);
        display_number(request, offsets, num);
        sleep(1);
     }

    /* 전부 끄기 */
    display_off(request, offsets);

    printf("Test Complete\n");

    gpiod_line_request_release(request);
    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);
    gpiod_chip_close(chip);

    return 0;
}