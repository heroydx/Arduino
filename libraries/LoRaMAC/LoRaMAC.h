/*
   LoRaMac.h
   
   The definition of LoRa air protocol CMD list
   
   2017  Steven Lian (steven.lian@gmail.com)
   
   
*/

#pragma once
#ifndef _LoRaMac_H
#define _LoRaMac_H

#include <arduino.h>

#include "LoRaMACDef.h"

#include <LoRa_AS62.h>
//#include <LoRa_LCZ.h>
#include <ListArray.h>

extern "C" {
#include "simpleHash.h"
#include "miscCommon.h"
}

#define CONST_LORA_MAC_VERSION 171008

#define CONST_LORA_DEFAULT_CHANNEL 425
#define CONST_LORA_AIR_BAUD_RATE 2400

#define CONST_HARDWARE_DEVICE_LENGTH 20

#define CONST_LORA_MAC_FILE_BUFFLEN 60
#define CONST_LORA_MAC_ADDRESS_FILENAME "/LoRa_MAC_addr.dat"

#define CONST_LORA_CONFIG_DATA_FILENAME "/LoRa_MAC_config.ini"

class LoRaHost {
  public:
    //data
    char hwDevice[CONST_HARDWARE_DEVICE_LENGTH];
    uint8_t hostAddr[LORA_MAC_ADDRESS_LEN]; //主机地址
    uint8_t resultAddr[LORA_MAC_ADDRESS_LEN];
    uint8_t addrBegin; //从那个号码开始分配，例如：addrBegin=5，则从 192.168.0.5开始 
    short maxCount; //最多可用地址个数
    short addrCount; //当前已经分配的地址个数
    short lastPos; //当前已经分配的最后地址
    unsigned long *addrListPtr; //指向地址分配表的指针
    uint8_t *seqListPtr; //相应的发送顺序数 seq 分配表指针
    uint8_t *feedbackListPtr; //相应的数据反馈标记,给发送slot的时候设置1，返回设置0
    short currPos; //当前位置
    short batchLen; //每一批申请地址个数，暂时没有用
    
    //function
    LoRaHost();
    virtual ~LoRaHost();

    short begin(uint8_t *Addr, uint8_t beginAddr, short subNum);
    bool isEmpty();
    bool isFull();
    short find(unsigned long devID);
    short find(unsigned long devID, uint8_t *addr );
    short add(unsigned long devID, uint8_t *addr);
    short set(unsigned long devID,uint8_t *addr);
    short del(unsigned long devID);
    unsigned long get_devID(uint8_t *addr); //通过某个地址的获取相应设备的devID
    unsigned long get_devID(uint8_t pos); //通过某个地址的位置获取相应设备的devID pos=*(addr + LORA_MAC_ADDRESS_LEN - 1)
    bool if_addr_exist(uint8_t *addr,uint8_t checkSum);
    void clean();
    short position(short index);
        
    bool set_send_flag(short index);
    bool clean_send_flag(short index);
    
    void debug_print_info();
    
    //save 
    bool bLoad_config();
    bool bSave_config();
    
  private:
    short stat();
};

extern short cmd_application_call(short CMD, uint8_t *ptr, short nLen);


class LoRaMAC
{
  public:
    //data
    unsigned long devID;
    uint8_t hostAddr[LORA_MAC_ADDRESS_LEN];//主机地址
    uint8_t hostMASK[LORA_MAC_ADDRESS_LEN]; //
    uint8_t meAddr[LORA_MAC_ADDRESS_LEN]; //本机地址
    uint8_t destAddr[LORA_MAC_ADDRESS_LEN];
    uint8_t key[LORA_MAC_KEY_LEN+2];
    uint8_t val[LORA_MAC_KEY_LEN+2];
    bool hostMasterExist;  //主机是否存在  
    bool linkReady; //LoRa 链路可以工作，登记/注册成功
    uint8_t chksum; //devID 4 字节的xor和
    uint8_t seq; //发送顺序字
    short workChannel; //LoRa当前工作的频道
    short preWorkchannel;//上次工作的频道
    short configChannel; //配置设备指定的频道
    bool sendWindowReady;

#define CONST_LORA_DEFAULT_MAX_CLIENT_NO_TIME_SLOT_LOOP 4
    uint8_t clientNoTimeSlotCount;
    uint8_t normalTokenLoopCount;
    
    short CMD;
    stDataStreamUnion recvData;
    stDataStreamUnion sendData;
    stDataStreamUnion emergencyData;

    //LoRa device parameters
    short loRaDevChannel;//send channel
    short loRaDevBandRate;
    unsigned short loRaDevAddress;
    
    //自动发送空中令牌(控制上传的时隙分配) 
    bool tokenAutoFlag; //是否打开token自动功能
    short tokenStrategyKey; //令牌(Normal/Emergency)发送间隔策略参数
    short tokenStrategyloopCount; //令牌(Normal/Emergency)发送间隔策略计数
    short emergencyTokenLen; //emergency token 个数
    short normalTokenLen; //normal token  个数
    short normalTokenPos; //normal token 需要给所有设备一次机会，因此需要记录当前发送到什么位置了。 
    unsigned long ulTokenSlotTick; //两个token之间的最小间隔计数
    unsigned long ulTokenMinTimeTicks; //两个token之间的最小间隔值
    stDataStreamUnion tokenData;  //将要发送的token数据
    
    //function
    LoRaMAC();
    //LoRaMAC(bool isHost, int subNum);
    short begin(bool isHost);
    short begin(bool isHost,short channel);
    short begin(bool isHost,short channel, uint8_t *addr);
    short begin(bool isHost,short channel, uint8_t *addr, short subNum);
    short begin(bool isHost,short channel, uint8_t *addr, uint8_t beginAddr, short subNum );
    
    uint8_t set_host_mode(uint8_t hostMode);
    
    virtual ~LoRaMAC();
    bool devReady();
    short available();
    short available(short debug);
    void LoRa_channel_config_check();
    
    short check_time_window_host();
    short check_time_window_client();
    short check_time_window_send();
    bool set_received_data_automatic(bool onOff);
    
    short handle_received_data_automatic();
    short handle_received_data_auto_client();
    short handle_received_data_auto_host_master();
    short handle_received_data_auto_host_slave();
    bool is_host_master_msg();
    bool is_to_me_msg();
    
    short send_data(uint8_t *u8Ptr);
    short send_data(uint8_t *u8Ptr, short nLen);
    short config_send_data(uint8_t *u8Ptr);

    short registration(short maid,short pid);
    
    unsigned long get_devID(uint8_t *addr);
    unsigned long get_devID(uint8_t pos);
    
    short cal_server_data_len(unsigned short type);

    bool is_valid_channel(short channel);

    //debug print function
    void debug_print_union(uint8_t *ptr);
    void debug_print_self();
    
    unsigned short get_version();

    bool bLoad_config();
    bool bSave_config();    
    
  private:
    //data
    unsigned short version;
    uint8_t hostFlag; //是否是主控设备
    bool deviceReady; //loRa 设备是否可以工作
    bool automaticFlag; //lora 接收自动处理开关
    short nRestartReason;

    unsigned long nextWindowTick;
    unsigned long nextWindowTimeSlotBegin;
    unsigned long normalWindowPeriod;
    unsigned long normalWindowTick;
    unsigned long emergencyWindowPeriod;
    unsigned long emergencyWindowTick;
    unsigned long slaveCheckHostMasterExistTick;
    unsigned long clientCheckHostMasterExistTick;
    unsigned long configCheckTick;

    //ListArray sendList(LORA_MAC_SEND_BUFF_MAX_LENGTH, sizeof(stDataStreamUnion));
    //ListArray sendList();

    //function
    unsigned long generate_devID();
    bool hash_verification_process(char *ptr, char *result);  //to verify host and client
    short address_me(uint8_t *addrPtr); // return 100 to me address, 4,3,2,1 broadcast address,;
    short send_registration_cleanup(uint8_t *addr);
    short func_cmd_config_channel_broardcast();
    short func_cmd_host_info_broardcast();
    bool func_cmd_registration_cleanup();
    short func_cmd_extend_cmd();
    short func_cmd_registration_request();
    short func_cmd_registration_feedback_client();
    short func_cmd_registration_feedback_slave();
    short func_cmd_addr_request();
    short func_cmd_addr_feedback_client();
    short func_cmd_addr_feedback_slave();
    short func_cmd_sync_cmd();
    short func_cmd_sync_time();
    short func_cmd_sync_random();
    short func_cmd_verify_request();
    bool func_cmd_verify_feedback();
    short func_cmd_frequency_change();
    unsigned long cal_host_time_slot();//计算主控机器(hostFlag==true)的发射时间窗口,默认利用发送数据(sendData)里面的参数计算。
    
    short send_normal_new_loop_flag();
    void check_insert_token_window();
    short token_type(short loopCount);
    //short token_data(short tokenType);
    void addr_conflict_detection();
    void vSystem_restart(short reason);
    
};

#endif // _LoRaMac_H
