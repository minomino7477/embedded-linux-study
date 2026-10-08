#include <stdio.h>
#include <unistd.h>
#include <gpiod.h>

#define GPIO_CHIP "/dev/gpiochip0"

#define CLK_GPIO 17
#define DIO_GPIO 27

static struct gpiod_chip *chip;
static struct gpiod_line_request *request;

/* TM1637 세그먼트 패턴: 0 ~ 9 */
static const unsigned char digit[] = {
    0x3F,   // 0
    0x06,   // 1
    0x5B,   // 2
    0x4F,   // 3
    0x66,   // 4
    0x6D,   // 5
    0x7D,   // 6
    0x07,   // 7
    0x7F,   // 8
    0x6F    // 9
};

static void delay_us(void)
{
    usleep(10);
}

static void set_clk(int value)
{
    gpiod_line_request_set_value(request, CLK_GPIO, value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

static void set_dio(int value)
{
    gpiod_line_request_set_value(request, DIO_GPIO, value ? GPIOD_LINE_VALUE_ACTIVE : GPIOD_LINE_VALUE_INACTIVE);
}

/* 통신 시작 */
static void tm_start(void)
{
    set_clk(1);
    set_dio(1);
    delay_us();

    set_dio(0);
    delay_us();

    set_clk(0);
}

/* 통신 종료 */
static void tm_stop(void)
{
    set_clk(0);
    set_dio(0);
    delay_us();

    set_clk(1);
    delay_us();

    set_dio(1);
    delay_us();
}

/* 1바이트 전송 */
static void tm_write_byte(unsigned char data)
{
    int i;

    for (i = 0; i < 8; i++) {
        set_clk(0);

        set_dio(data & 0x01);

        delay_us();

        set_clk(1);
        delay_us();

        data >>= 1;
    }

    /*
     * ACK 구간.
     * 첫 실습에서는 DIO 입력 전환까지 하지 않고
     * 클럭만 한 번 발생시킨다.
     */

     set_clk(0);
     delay_us();

     set_clk(1);
     delay_us();

     set_clk(0);
}

/* 4자리 숫자 표시 */
static void tm_display_number(int number)
{
    int d[4];

    if (number < 0) {
        number = 0;
    }

    number %= 10000;

    d[0] = number / 1000;
    d[1] = (number / 100) % 10;
    d[2] = (number / 10) % 10;
    d[3] = number % 10;

    /* 자동 주소 증가 모드 */
    tm_start();
    tm_write_byte(0x40);
    tm_stop();

    /* 첫 번째 자리 주소 */
    tm_start();
    tm_write_byte(0xC0);

    tm_write_byte(digit[d[0]]);
    tm_write_byte(digit[d[1]]);
    tm_write_byte(digit[d[2]]);
    tm_write_byte(digit[d[3]]);

    tm_stop();

    /* Display ON + 밝기 최대 */
    tm_start();
    tm_write_byte(0x8F);
    tm_stop();
}

int main(void)
{
    struct gpiod_line_settings *settings;
    struct gpiod_line_config *line_cfg;
    struct gpiod_request_config *req_cfg;

    unsigned int offsets[] = {
        CLK_GPIO,
        DIO_GPIO
    };

    int count;

    chip = gpiod_chip_open(GPIO_CHIP);

    if (!chip) {
        perror("gpiod_chip_open");
        return 1;
    }

    settings = gpiod_line_settings_new();
    line_cfg = gpiod_line_config_new();
    req_cfg = gpiod_request_config_new();

    if (!settings || !line_cfg || !req_cfg) {
        fprintf(stderr, "GPIO config allocation failed\n");
        return 1;
    }

    gpiod_line_settings_set_direction(settings, GPIOD_LINE_DIRECTION_OUTPUT);

    gpiod_line_settings_set_output_value(settings, GPIOD_LINE_VALUE_ACTIVE);

    gpiod_line_config_add_line_settings(line_cfg, offsets, 2, settings);

    gpiod_request_config_set_consumer(req_cfg, "tm1637");

    request = gpiod_chip_request_lines(chip, req_cfg, line_cfg);

    if (!request) {
        perror("gpiod_chip_request_lines");
        return 1;
    }

    printf("TM1637 counter start\n");

    for (count = 0; count <= 9999; count++) {
        tm_display_number(count);

        printf("\rCount : %04d", count);
        fflush(stdout);

        usleep(100000);     // 0.1초
    }

    printf("\n");

    gpiod_line_request_release(request);

    gpiod_request_config_free(req_cfg);
    gpiod_line_config_free(line_cfg);
    gpiod_line_settings_free(settings);

    gpiod_chip_close(chip);

    return 0;
}