#include "lvgl/lvgl.h"
#include "lv_drivers/display/fbdev.h"
#include "lv_examples/lv_examples.h"
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include <sys/time.h>
#include <stdlib.h>

#define DISP_BUF_SIZE (80 * LV_HOR_RES_MAX)

lv_obj_t* slider;
lv_chart_series_t * ser1;
lv_obj_t * chart;

void my_task(lv_task_t* task)
{
    int r = rand() % 100;
    lv_slider_set_value(slider, r, LV_ANIM_ON);

    for(int i = 0; i < 10;i++)
    {
        ser1->points[i] = rand()%100;
    }
    lv_chart_refresh(chart);
}

LV_IMG_DECLARE(img_cogwheel_argb);

void lv_ex_btn_1(void)
{
    /*Describe the color for the needles*/
    slider = lv_slider_create(lv_scr_act(), NULL);
    lv_obj_set_style_local_border_width(slider, LV_SLIDER_PART_BG, LV_STATE_DEFAULT, 1);
    lv_obj_set_size(slider, 30, 2);
    lv_obj_align(slider, NULL, LV_ALIGN_IN_TOP_LEFT, 0, 5);
    lv_slider_set_value(slider, 15, LV_ANIM_ON);

    lv_obj_t * label2 = lv_label_create(lv_scr_act(), NULL);
    lv_label_set_long_mode(label2, LV_LABEL_LONG_SROLL_CIRC);     /*Circular scroll*/
    lv_obj_set_size(label2, 30,15);
    lv_label_set_text(label2, "It is a circularly scrolling text. ");
    lv_obj_align(label2, slider, LV_ALIGN_OUT_BOTTOM_LEFT, 0, 0);

    lv_obj_t * btn1 = lv_btn_create(lv_scr_act(), NULL);
    lv_obj_set_size(btn1, 30,15);
    lv_obj_align(btn1, slider, LV_ALIGN_OUT_RIGHT_TOP, 2, 0);

    lv_obj_t * label3 = lv_label_create(btn1, NULL);
    lv_label_set_text(label3, "btn");
    
    chart = lv_chart_create(lv_scr_act(), NULL);
    lv_obj_set_size(chart, 68, 32);
    lv_obj_align(chart, btn1, LV_ALIGN_OUT_RIGHT_TOP, 0, -10);
    lv_chart_set_type(chart, LV_CHART_TYPE_LINE);   /*Show lines and points too*/
    lv_obj_set_style_local_size(chart,LV_CHART_PART_SERIES,LV_STATE_DEFAULT,0);
    lv_obj_set_style_local_line_width(chart,LV_CHART_PART_SERIES_BG,LV_STATE_DEFAULT,1);

    /*Add two data series*/
    ser1 = lv_chart_add_series(chart, LV_COLOR_BLACK);
    lv_chart_series_t * ser2 = lv_chart_add_series(chart, LV_COLOR_BLACK);

    /*Set the next points on 'ser1'*/
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 10);
    lv_chart_set_next(chart, ser1, 30);
    lv_chart_set_next(chart, ser1, 70);
    lv_chart_set_next(chart, ser1, 90);

    /*Directly set points on 'ser2'*/
    ser2->points[0] = 90;
    ser2->points[1] = 70;
    ser2->points[2] = 65;
    ser2->points[3] = 65;
    ser2->points[4] = 65;
    ser2->points[5] = 65;
    ser2->points[6] = 65;
    ser2->points[7] = 65;
    ser2->points[8] = 65;
    ser2->points[9] = 65;

    lv_chart_refresh(chart); /*Required after direct set*/

    static uint32_t user_data = 10;
    lv_task_create(my_task, 1000, LV_TASK_PRIO_MID, &user_data);
}

int main(void)
{
    /*LittlevGL init*/
    lv_init();

    /*Linux frame buffer device init*/
    fbdev_init();

    /*A small buffer for LittlevGL to draw the screen's content*/
    static lv_color_t buf[DISP_BUF_SIZE];

    /*Initialize a descriptor for the buffer*/
    static lv_disp_buf_t disp_buf;
    lv_disp_buf_init(&disp_buf, buf, NULL, DISP_BUF_SIZE);

    /*Initialize and register a display driver*/
    lv_disp_drv_t disp_drv;
    lv_disp_drv_init(&disp_drv);
    disp_drv.buffer   = &disp_buf;
    disp_drv.flush_cb = fbdev_flush;
    lv_disp_drv_register(&disp_drv);

    /*Create a Demo*/
    lv_ex_btn_1();

    /*Handle LitlevGL tasks (tickless mode)*/
    while(1) {
        lv_task_handler();
        usleep(5000);
    }

    return 0;
}

/*Set in lv_conf.h as `LV_TICK_CUSTOM_SYS_TIME_EXPR`*/
uint32_t custom_tick_get(void)
{
    static uint64_t start_ms = 0;
    if(start_ms == 0) {
        struct timeval tv_start;
        gettimeofday(&tv_start, NULL);
        start_ms = (tv_start.tv_sec * 1000000 + tv_start.tv_usec) / 1000;
    }

    struct timeval tv_now;
    gettimeofday(&tv_now, NULL);
    uint64_t now_ms;
    now_ms = (tv_now.tv_sec * 1000000 + tv_now.tv_usec) / 1000;

    uint32_t time_ms = now_ms - start_ms;
    return time_ms;
}
