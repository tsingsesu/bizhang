/*********************************************************************************************************************
* TC264 Opensourec Library 即（TC264 开源库）是一个基于官方 SDK 接口的第三方开源库
* Copyright (c) 2022 SEEKFREE 逐飞科技
*
* 本文件是 TC264 开源库的一部分
*
* TC264 开源库 是免费软件
* 您可以根据自由软件基金会发布的 GPL（GNU General Public License，即 GNU通用公共许可证）的条款
* 即 GPL 的第3版（即 GPL3.0）或（您选择的）任何后来的版本，重新发布和/或修改它
*
* 本开源库的发布是希望它能发挥作用，但并未对其作任何的保证
* 甚至没有隐含的适销性或适合特定用途的保证
* 更多细节请参见 GPL
*
* 您应该在收到本开源库的同时收到一份 GPL 的副本
* 如果没有，请参阅<https://www.gnu.org/licenses/>
*
* 额外注明：
* 本开源库使用 GPL3.0 开源许可证协议 以上许可申明为译文版本
* 许可申明英文版在 libraries/doc 文件夹下的 GPL3_permission_statement.txt 文件中
* 许可证副本在 libraries 文件夹下 即该文件夹下的 LICENSE 文件
* 欢迎各位使用并传播本程序 但修改内容时必须保留逐飞科技的版权声明（即本声明）
*
* 文件名称          cpu0_main
* 公司名称          成都逐飞科技有限公司
* 版本信息          查看 libraries/doc 文件夹内 version 文件 版本说明
* 开发环境          ADS v1.8.0
* 适用平台          TC264D
* 店铺链接          https://seekfree.taobao.com/
*
* 修改记录
* 日期              作者                备注
* 2022-09-15       pudding            first version
********************************************************************************************************************/
#include "zf_common_headfile.h"
#pragma section all "cpu0_dsram"

// ================= ★图传（逐飞助手） =================
// 1 = 开图传：每帧约 2 秒（115200 波特率），期间屏幕刷新和按键会变慢
// 0 = 关图传（在屏幕上调参时用）
#define CAMERA_SEND_ENABLE      (0)   // [2026-10-20] was 1: 115200 baud costs ~2 s per frame

#if CAMERA_SEND_ENABLE
static uint8 image_copy[MT9V03X_H][MT9V03X_W];
static seekfree_assistant_camera_struct camera_information;
#endif
// 将本语句与#pragma section all restore语句之间的全局变量都放在CPU0的RAM中

volatile uint32 main_loop_tick = 0;

int core0_main(void)
{
   system_delay_ms(1500);
   clock_init();                   // 获取时钟频率<务必保留>
   debug_init();                   // 初始化默认调试串口
#if CAMERA_SEND_ENABLE
    seekfree_assistant_interface_init(SEEKFREE_ASSISTANT_DEBUG_UART);
    seekfree_assistant_camera_config(&camera_information, SEEKFREE_ASSISTANT_CAMERA_TYPE_MT9V03X,
                                     MT9V03X_W, MT9V03X_H, image_copy[0]);
#endif
    // 此处编写用户代码 例如外设初始化代码等
    System();
    // 此处编写用户代码 例如外设初始化代码等
    cpu_wait_event_ready();         // 等待所有核心初始化完毕
    while(TRUE)
    {
        main_loop_tick ++;
#if CAMERA_SEND_ENABLE
        // 图传：发送二值化图像（和屏幕 page1 看到的一样）
        memcpy(image_copy[0], imagebinary[0], MT9V03X_IMAGE_SIZE);
        seekfree_assistant_camera_send(&camera_information);
#endif
        LuUI_Log_Poll();   // [2026-10-20] CPU1-requested serial telemetry (off by default)
        LuUI_Key();
        LuUI_Show();    // ★屏幕已确认可用（官方例程能亮）；坐标已核对不会触发 zf_assert
                        //   LuUI_Key() 必须保留（按键扫描在里面）
        //////感应发车/////////////

        if(Stop_1==0)
        {


            if(dl1b_distance_mm<201)
            {
                time_dl1b_on=1;
            }
            if(dl1b_distance_mm<400&&dl1b_distance_mm>200)
            {

                if(Differ_light >= 3) Differ_light -= 3;   // [2026-10-20] uint8: no wrap-around
                // ★原来是 while(dl1b_distance_mm<=400); —— 会死等，主循环卡死
                //   实测 DL1B 读数一直落在 216~287，正好卡在 200~400 区间，CPU0 主循环永远出不来。
                //   现在改成不等待，靠后面 time_dl1b_s/time_dl1b 的计时逻辑判断发车。
            }
            // ★ 2 号 DL1B 已删除（V3.1 只有一个 ToF 座）——原来这段用 dl1b2_distance_mm 的发车逻辑已去掉
            if(tiaoguang==1&&time_dl1b_s>300)
            {
                time_dl1b_on=0;
                time_dl1b_s=0;
                time_dl1b=0;
                tiaoguang=0;
                Differ_light+=8;
            }
            gpio_set_level(P02_4,1);//左前转向
            gpio_set_level(P02_5,1);//左后转向
            gpio_set_level(P02_6,1);//右前转向
            gpio_set_level(P02_7,1);//右后转向
            gpio_set_level(P20_11,1);//左前大灯
            gpio_set_level(P20_12,1);//右前大灯
            gpio_set_level(P20_13,1);//左后刹灯
            gpio_set_level(P20_14,1);//右后刹灯
            if(dl1b_distance_mm<400)
            {
                time_dl1b_on=1;

            }

            if(time_dl1b_s>500)
            {
                if(time_dl1b>200)
                {

                    gpio_set_level(P02_4,0);//左前转向
                    gpio_set_level(P02_5,0);//左后转向
                    gpio_set_level(P02_6,0);//右前转向
                    gpio_set_level(P02_7,0);//右后转向
                    gpio_set_level(P20_11,0);//左前大灯
                    gpio_set_level(P20_12,0);//右前大灯
                    gpio_set_level(P20_13,0);//左后刹灯
                    gpio_set_level(P20_14,0);//右后刹灯
                    Flag_turn=3;
                    Flag_stop=0;
                    Deal_Flag=0;
                    mode_2024=2;

                }
                else
                {

                    gpio_set_level(P02_4,0);//左前转向
                    gpio_set_level(P02_5,0);//左后转向
                    gpio_set_level(P02_6,0);//右前转向
                    gpio_set_level(P02_7,0);//右后转向
                    gpio_set_level(P20_11,0);//左前大灯
                    gpio_set_level(P20_12,0);//右前大灯
                    gpio_set_level(P20_13,0);//左后刹灯
                    gpio_set_level(P20_14,0);//右后刹灯
                    Flag_turn=3;
                    Flag_stop=0;
                    Deal_Flag=0;
                    mode_2024=1;
                }

                time_dl1b_s=0;
                time_dl1b=0;
                time_dl1b_on=0;

                Stop_1=1;
            }
        }


    }
}

#pragma section all restore
// **************************** 代码区域 ****************************

// *************************** 例程常见问题说明 ***************************
// 遇到问题时请按照以下问题检查列表检查
// 问题1：LED 不闪烁
//      如果使用主板测试，主板必须要用电池供电
//      查看程序是否正常烧录，是否下载报错，确认正常按下复位按键
//      万用表测量对应 LED 引脚电压是否变化，如果不变化证明程序未运行，如果变化证明 LED 灯珠损坏
// 问题2：SWITCH1 / SWITCH2 更改组合流水灯频率无变化
//      如果使用主板测试，主板必须要用电池供电
//      查看程序是否正常烧录，是否下载报错，确认正常按下复位按键
//      万用表测量对应 LED 引脚电压是否变化，如果不变化证明程序未运行，如果变化证明 LED 灯珠损坏
//      万用表检查对应 SWITCH1 / SWITCH2 引脚电压是否正常变化，是否跟接入信号不符，引脚是否接错
// 问题2：KEY1 / KEY2 / KEY3 / KEY4 接GND或者按键按下无变化
//      如果使用主板测试，主板必须要用电池供电
//      查看程序是否正常烧录，是否下载报错，确认正常按下复位按键
//      万用表测量对应 LED 引脚电压是否变化，如果不变化证明程序未运行，如果变化证明 LED 灯珠损坏
//      万用表检查对应 KEY1 / KEY2 / KEY3 / KEY4 引脚电压是否正常变化，是否跟接入信号不符，引脚是否接错

