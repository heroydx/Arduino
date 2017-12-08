/*
   Meter_PZ_def.h
   Library to support Power Meter module, provied by PZ.
   taobao link 
   https://item.taobao.com/item.htm?id=45589922100&_u=b1im8oic076
   
    ---

    Copyright (C) 2017  Steven Lian (steven.lian@gmail.com)
   

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
#ifndef _Meter_PZ_DEF_H
#define _Meter_PZ_DEF_H

//确定计量的比率，影响电流和功率，以及电量值 >1,放大倍数，<0,缩小倍数
#define CONST_APP_METER_POWER_RATIO 1 

#define CONST_APP_METER_DEFAULT_TRY_TIMES 5

#define CONST_APP_METER_FILE_NAME "/application.data"
#define CONST_APP_METER_FILE_SIZE 1024

#define CONST_APP_METER_SIO_BAUD_DATA 9600
#define CONST_APP_METER_SENDREV_BUFFLEN 40
#define CONST_APP_METER_SIO_DATA_LEN 7

#define CONST_APP_METER_DIANYA_FLAG 0
#define CONST_APP_METER_DIANLIU_FLAG 1
#define CONST_APP_METER_GONGLV_FLAG 2
#define CONST_APP_METER_DIANLIANG_FLAG 3
#define CONST_APP_METER_POWERFACTOR_FLAG 4
#define CONST_APP_METER_FREQUENCY_FLAG 5
#define CONST_APP_METER_ZERO_FLAG 6
#define CONST_APP_METER_SETIP_FLAG 7


#define CONST_APP_METER_DIANYA_SEND 0xB0
#define CONST_APP_METER_DIANYA_RECE 0xA0
#define CONST_APP_METER_DIANLIU_SEND 0xB1
#define CONST_APP_METER_DIANLIU_RECE 0xA1
#define CONST_APP_METER_GONGLV_SEND 0xB2
#define CONST_APP_METER_GONGLV_RECE 0xA2
#define CONST_APP_METER_DIANLIANG_SEND 0xB3
#define CONST_APP_METER_DIANLIANG_RECE 0xA3
#define CONST_APP_METER_POWERFACTOR_SEND 0xB4
#define CONST_APP_METER_POWERFACTOR_RECE 0xA4
#define CONST_APP_METER_FREQUENCY_SEND 0xB5
#define CONST_APP_METER_FREQUENCY_RECE 0xA5
#define CONST_APP_METER_ZERO_SEND 0xB6
#define CONST_APP_METER_ZERO_RECE 0xA6
#define CONST_APP_METER_SETIP_SEND 0xB7
#define CONST_APP_METER_SETIP_RECE 0xA7
#define CONST_APP_METER_VERIFICATION_SEND 0xBF
#define CONST_APP_METER_VERIFICATION_RECE 0xAF


#define CONST_APP_METER_COLLECTION_INTERVAL 5000
//命令种类
#define CONST_APP_METER_SIO_COMMAND_COUNT 8

//轮询发送命令的个数
#define CONST_APP_METER_SIO_COMMAND_LOOP 4
 
#define CONST_APP_METER_SIO_QUERY_INTERVAL  2000
//两次电量读数的差值不应该超过100WH=0.2度电, 考虑100安培电流，查询间隔4*CONST_APP_METER_SIO_QUERY_INTERVAL
//如果是200A以上,这个值可以是400
#define CONST_APP_METER_SIO_DIANLIANG_MAX_ERROR  400
#define CONST_APP_METER_SIO_ERROR_COUNT 5

#define CONST_APP_METER_GONGLV_LAST_MAX_LEN 16

//补充发送数据的一些条件，电压变化，功率变化， 功率从某个值下降到某个范围等。
#define CONST_WARNING_DIANYA 10
#define CONST_WARNING_GONGLV 300
#define CONST_WARNING_GONGLV_LOW 20
#define CONST_WARNING_GONGLV_ZERO 5

#define CONST_AIR_CONDITION_GONGLV_THRESHOLD 10

#define CONST_AIR_CONDITION_GONGLV_DIV_VAL 10
#define CONST_AIR_CONDITION_GONGLV_EXT_VAL 10


// header defining the interface of the source.
#endif // _Meter_PZ_DEF_H

