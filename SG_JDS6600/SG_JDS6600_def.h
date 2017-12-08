/*
   SG_JDS6600_def.h

   Library to support Sigal Generator, provied by 均测科技.
   taobao link 
   https://detail.tmall.com/item.htm?id=559945767309&cm_id=140105335569ed55e27b&abbucket=3
  
    ---

    Copyright (C) 2017/11/14  Steven Lian (steven.lian@gmail.com) 
   

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#ifndef _SG_JDS6600_DEF_H
#define _SG_JDS6600_DEF_H


// header defining the interface of the source.

#define CONST_APP_SG_RECV_SIO_BAUD_DATA 115200
//#define CONST_APP_SG_RECV_SIO_BAUD_DATA 9600
#define CONST_APP_SG_RECV_SENDREV_BUFFLEN 80
#define CONST_APP_SG_SEND_BUF_COUNT 32
#define CONST_APP_SG_RECV_BIG_BUFFLEN 160
#define CONST_APP_SG_RECV_SIO_DATA_BEGIN ':'
#define CONST_APP_SG_RECV_SIO_DATA_END 10
#define CONST_APP_SG_RECV_SIO_DATA_SPLIT '='

#define CONST_APP_SG_RECV_SIO_ERROR_COUNT 5

#define CONST_APP_SG_RECV_TIMEOUT_LEN (50)
#define CONST_APP_SG_SEND_SIO_CMD_INTERVAL 50
#define CONST_APP_SG_RECV_SIO_QUERY_INTERVAL  (1000*60)

#define CONST_APP_SG_RECV_SYS_COMMAND_BUFFLEN 80

#define CONST_APP_SG_STAT_QUERY_INTERVAL (1000*10)
#define CONST_APP_SG_STAT_QUERY_FAST (50)

#define CONST_APP_SG_RECV_FREQ_MAX_SAVE_POS 20
#define CONST_APP_SG_RECV_COMMAND_BUFF_LEN 10
#define CONST_APP_SG_RECV_COMMAND_PARAM_LEN 10

#define CONST_APP_SG_RECV_SAVE_CYCLE (1000*60*60)

// 操作符是ASCII字符表中其中的四个小写字符，“w”是写入指令用来设置各种参数，
// “r”是读取指令用来返回机器中的参数，“a”指令是用来任意波数据的写入，“b” 指令用来任意波数据的读取
#define CONST_APP_SG_CMD_TYPE_STAND_WRITE 'w'
#define CONST_APP_SG_CMD_TYPE_STAND_READ 'r'
#define CONST_APP_SG_CMD_TYPE_RANDOM_WRITE 'a'
#define CONST_APP_SG_CMD_TYPE_RANDOM_READ 'b'

//version Serial No
#define CONST_APP_SG_FMT_MODEL 0
#define CONST_APP_SG_FMT_SN 1

// 波形设置 
//:w21=0.<CR><LF>
#define CONST_APP_SG_FMT_WAVE_CH1 21
#define CONST_APP_SG_FMT_WAVE_CH2 22

//正弦波
#define CONST_WAVE_FMT_SINE_STAND 0
//方波
#define CONST_WAVE_FMT_SQUARE_STAND 1
//脉冲波
#define CONST_WAVE_FMT_SQUARE_PULSE 2
//三角波
#define CONST_WAVE_FMT_TRIANGLE_STAND 3
//偏正弦波
#define CONST_WAVE_FMT_SINE_PARTIAL 4
//为cmos波
#define CONST_WAVE_FMT_CMOS_STAND 5
//直流电平
#define CONST_WAVE_FMT_DC_LEVEL 6
//半波
#define CONST_WAVE_FMT_HALF 7
//全波
#define CONST_WAVE_FMT_FULL 8
//正阶梯波
#define CONST_WAVE_FMT_POSITIVE_STAIR 9
//反阶梯波
#define CONST_WAVE_FMT_ANTI_LADDER 10
//噪声波
#define CONST_WAVE_FMT_NOISE 11
//指数升
#define CONST_WAVE_FMT_INDEX_UP 12
//指数降
#define CONST_WAVE_FMT_INDEX_DOWN 13
//多音波
#define CONST_WAVE_FMT_MULTI_SONIC 14
//辛克脉冲
#define CONST_WAVE_FMT_SINC_PULSE 15
//洛伦兹脉冲
#define CONST_WAVE_FMT_LORENTZ_PULSE 16

//任意波
#define CONST_WAVE_FMT_SELF_DEF_MIN 101
#define CONST_WAVE_FMT_SELF_DEF_MAX 160

// 频率设置
#define CONST_APP_SG_FMT_FREQ_CH1 23
#define CONST_APP_SG_FMT_FREQ_CH2 24

//:w23=25786,0.<CR><LF>设置为257.86Hz
//:w23=25786,1.<CR><LF>设置为0.25786KHz
#define CONST_FREQ_FMT_UNIT_HZ  0
#define CONST_FREQ_FMT_UNIT_KHZ 1
#define CONST_FREQ_FMT_UNIT_MHZ 3
#define CONST_FREQ_FMT_UNIT_UHZ 4

// 幅度设置
//:w25=x.<CR><LF> 当x=30时设置通道1 幅度输出为0.03v
#define CONST_APP_SG_FMT_AMP_CH1 25
#define CONST_APP_SG_FMT_AMP_CH2 26

// 偏置设置
//:w27=9999.<CR><LF>设置通道1的偏置输出为9.99v
#define CONST_APP_SG_FMT_BIAS_CH1 27
#define CONST_APP_SG_FMT_BIAS_CH2 28

// 占空比设置
//:w29=x.<CR><LF> 当x=500时设置通道1 占空比输出为50%
#define CONST_APP_SG_FMT_DUTY_CH1 29
#define CONST_APP_SG_FMT_DUTY_CH2 30

// 相位设置
//:w31=100.<CR><LF>表示相位输出为10°，机器返回OK表示设置成功
//送:w31=3600.<CR><LF>表示相位为0°
#define CONST_APP_SG_FMT_PHASE_CH1 31
//#define CONST_APP_SG_FMT_PHASE_CH2 32

// 跟踪设置
//:w54=x,x,x,x,x.<CR><LF> 跟踪设置中操作数的数值（x的值）为1或者为0,1表示同步0表示异步，且同步时都是以通道1为操作对象。操作数的个数对应的参数为:w54=频率,波形,幅度,偏置,占空比。
//:w54=1,0,0,0,0.<CR><LF> 设置频率同步（波形幅度偏置占空比异步）
#define CONST_APP_SG_FMT_TRACE_CH2 54

// 设置跟踪模式数据格式 {freq,wave,amp,bias,duty}={1,0,0,0,0}
#define CONST_APP_SG_TRACE_MODE_LEN 5
#define CONST_APP_SG_TRACE_MODE_FREQ 0
#define CONST_APP_SG_TRACE_MODE_WAVE 1
#define CONST_APP_SG_TRACE_MODE_AMP  2
#define CONST_APP_SG_TRACE_MODE_BIAS 3
#define CONST_APP_SG_TRACE_MODE_DUTY 4


// 扩展功能
//:w32=x,x,x,x.<CR><LF>其中操作数的数值（x的值）只能为1或者为0）
//:w32=0,0,0,0.<CR><LF> 表示关闭 计数 扫频调幅 猝发并且开启测量的开始
#define CONST_APP_SG_FMT_EXTEND_CH2 32

// 功能面板的切换
//:w33=0.<CR><LF> 机器面板会切换到主面板并通道1为主通道
//:w33=1.<CR><LF> 机器面板会切换到主面板并通道2为主通道，机器返回OK表示设置成功。
#define CONST_APP_SG_FMT_SWITCH 33

// 扩展 耦合功能
//:w36=0.<CR><LF> 表示耦合切换到AC 
//:w36=1.<CR><LF> 表示耦合切换到DC
#define CONST_APP_SG_FMT_COUPLING 36

//耦合数据
#define CONST_APP_SG_COUPLING_MODE_AC 0
#define CONST_APP_SG_COUPLING_MODE_DC 1

// 扩展 设定闸门时间
//:w37=100.<CR><LF> 设定闸门时间1秒
#define CONST_APP_SG_FMT_GATE 37

// 扩展 测量模式
//:w38=0.<CR><LF>设定测量模式（计频）
//:w38=1.<CR><LF>设定测量模式（计周期）
#define CONST_APP_SG_FMT_MEASURE 38


// 扩展 扫频模式
//:w40=1000.<CR><LF> 设置起始频率为10Hz
//:w41=1000.<CR><LF>设置终止频率为10Hz
//:w42=10.<CR><LF>设置扫频时间为1秒
//:w43=0.<CR><LF> 扫频方向正常,1反向 2往返
//:w44=0.<CR><LF> 扫频模式为线性,:w44=1.<CR><LF>扫频模式为对数
#define CONST_APP_SG_FMT_SWEEP_BEGIN 40
#define CONST_APP_SG_FMT_SWEEP_END 41
#define CONST_APP_SG_FMT_SWEEP_TIME 42
#define CONST_APP_SG_FMT_SWEEP_DIRECTION 43
#define CONST_APP_SG_FMT_SWEEP_MODE 44

//扫频方向定义
#define CONST_APP_SG_SWEEP_DIRECTION_NORMAL 0
#define CONST_APP_SG_SWEEP_DIRECTION_REVERSE 1
#define CONST_APP_SG_SWEEP_DIRECTION_CYCLE 2

// 扩展 脉冲模式
//:w45=1000,0.<CR><LF> 设置脉宽为1000单位为ns
//:w45=1000,1.<CR><LF> 设置脉宽为1000单位为us
//:w46=1000,0.<CR><LF> 设置周期为1000单位为ns
//:w46=1000,1.<CR><LF> 设置周期为1000单位us
//:w47=100.<CR><LF> 设置偏移量为100%
//:w48=500.<CR><LF> 设置幅度为5.00V
#define CONST_APP_SG_FMT_PULSE_WIDTH 45
#define CONST_APP_SG_FMT_PULSE_CYCLE 46
#define CONST_APP_SG_FMT_PULSE_BIAS 47
#define CONST_APP_SG_FMT_PULSE_AMP 48

// 扩展 猝发模式
//:w49=5.<CR><LF> 脉冲数设置为5
//:w49=100.<CR><LF> 脉冲数设置为100
#define CONST_APP_SG_FMT_BURST 49
//猝发模式数据
#define CONST_APP_SG_BURST_MODE_MANUAL 0
#define CONST_APP_SG_BURST_MODE_CH2 1
#define CONST_APP_SG_BURST_MODE_AC 2
#define CONST_APP_SG_BURST_MODE_DC 3



// 调出与保存
//:w70=5. <CR><LF> 表示把参数存储到 5位置处参数
//:w71=5. <CR><LF> 表示调出 5 位置处参数
//:w72=5. <CR><LF> 表示清除 5 位置的参数
#define CONST_APP_SG_FMT_SAVE 70
#define CONST_APP_SG_FMT_LOAD 71
#define CONST_APP_SG_FMT_CLEAN 72

//自定义配置参数
//通道1幅度电位器(步进100mv)
#define CONST_APP_SG_CONFIG_AMP_STEP 100
#define CONST_APP_SG_CONFIG_AMP_MIN 0
#define CONST_APP_SG_CONFIG_AMP_MAX 20000


//通道1,2(两路频率同步)的频率调整,步长50hz,频率范围50hz—40khz
#define CONST_APP_SG_CONFIG_FREQ_STEP 5000
#define CONST_APP_SG_CONFIG_FREQ_MIN 5000
#define CONST_APP_SG_CONFIG_FREQ_MAX 4000000

//通道2的脉冲(占空比10%)位置调整电位器,步进0.5%
#define CONST_APP_SG_CONFIG_DUTY_DEFAULT 100
#define CONST_APP_SG_CONFIG_DUTY_STEP 5
#define CONST_APP_SG_CONFIG_DUTY_MIN 5
#define CONST_APP_SG_CONFIG_DUTY_MAX 9995

//默认数据
//初次上电后输出频率20khz，ch1锯齿波（直角三角波），ch2矩形波（10%占空比），双路频率同步，相位差36度，ch1满幅0---20v，ch2 0---3.3v，无直流偏置
#define CONST_APP_SG_DEFAULT_FREQ 20000
#define CONST_APP_SG_DEFAULT_WAVE_CH1 CONST_WAVE_FMT_TRIANGLE_STAND
#define CONST_APP_SG_DEFAULT_WAVE_CH2 CONST_WAVE_FMT_SQUARE_PULSE
#define CONST_APP_SG_DEFAULT_DUTY_CH2 100
#define CONST_APP_SG_DEFAULT_TRACE_MODE_FREQ 1
#define CONST_APP_SG_DEFAULT_PHASE_CH1 3600
#define CONST_APP_SG_DEFAULT_PHASE_CH2 360
#define CONST_APP_SG_DEFAULT_AMP_CH1 2000
#define CONST_APP_SG_DEFAULT_AMP_CH2  330
#define CONST_APP_SG_DEFAULT_BIAS_CH1 1000
#define CONST_APP_SG_DEFAULT_BIAS_CH2 1000

typedef struct{
  unsigned long freq;
  unsigned short phase;
  unsigned short amp;
  unsigned short bias;
  unsigned short duty;
  unsigned short gate;
  unsigned short sweepCycle;
  uint8_t freqUnit;
  uint8_t waveMode;
  uint8_t couplingMode;
  uint8_t measureMode;
  uint8_t bruseMode;
  uint8_t sweepDirection;
  uint8_t traceMode[CONST_APP_SG_TRACE_MODE_LEN];
} stChannelInfo;



#endif // _SG_JDS6600_DEF_H

