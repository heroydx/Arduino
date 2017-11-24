/*
  文件名称：smartconfig.ino
  功能：ESP8266快速配置功能
  作者：www.doit.am
  日期：2015-12-31
  版本：1.0
  Modified by Steven lian
  
*/
#include <ESP8266WiFi.h>
//#include "ESP8266ClientDB.h"

//#include "ets_sys.h"
//#include "osapi.h"
#include "ip_addr.h"
#include "espconn.h"
#include "airkiss.h"

//#include "mem.h"

//#include "user_interface.h"
//#include "smartconfig.h"
//#include "airkiss.h"


#define LED 13
//int airkiss_get_result(airkiss_context_t* context,airkiss_result_t* result);


void smartConfig()
{
  WiFi.mode(WIFI_STA);
  Serial.println("\r\nWait for Smartconfig");
  WiFi.beginSmartConfig();
  while (1)
  {
    Serial.print(".");
    digitalWrite(LED, 0);
    delay(500);
    digitalWrite(LED, 1);
    delay(500);
    if (WiFi.smartConfigDone())
    {
      int err;
      //err = airkiss_get_result(&akcontex, &airkissResult);
      Serial.println("SmartConfig Success");
      Serial.printf("SSID:%s\r\n", WiFi.SSID().c_str());
      Serial.printf("PSW:%s\r\n", WiFi.psk().c_str());
      break;
    }
  }
}



#define CONST_AIRKISS_DEVICE_TYPE     "gh_3a65280f9a11 " //wechat public number
#define CONST_AIRKISS_DEVICE_ID       "27237" //model ID

#define CONST_DEFAULT_LAN_PORT  12476

LOCAL char deviceType[] = CONST_AIRKISS_DEVICE_TYPE;
LOCAL char deviceID[] = CONST_AIRKISS_DEVICE_ID;


//ESP8266Client *ghpUdpAirkiss;
esp_udp ssdp_udp;
struct espconn pssdpudpconn;

uint8_t  lan_buf[200];
uint16_t lan_buf_len;
uint8    udp_sent_cnt = 0;

unsigned long ul5stick = 0;

void vReset_interval(unsigned long &resetTick)
{
  resetTick = millis();
}

unsigned long ulGet_interval(unsigned long checkTick)
{
  unsigned long cur = millis();
  if (cur >= checkTick)
    return cur - checkTick;
  else
    return 0xffffffff - checkTick + cur;;
}

const airkiss_config_t akconf =
{
  (airkiss_memset_fn)&memset,
  (airkiss_memcpy_fn)&memcpy,
  (airkiss_memcmp_fn)&memcmp,
  0,
};

LOCAL void ICACHE_FLASH_ATTR
airkiss_wifilan_time_callback(void)
{
  uint16 i;
  //airkiss_lan_ret_t ret;
  int ret;
  
  if ((udp_sent_cnt++) > 30) {
    udp_sent_cnt = 0;
    //os_timer_disarm(&ssdp_time_serv);//s
    //return;
  }

  ssdp_udp.remote_port = CONST_DEFAULT_LAN_PORT;
  ssdp_udp.remote_ip[0] = 255;
  ssdp_udp.remote_ip[1] = 255;
  ssdp_udp.remote_ip[2] = 255;
  ssdp_udp.remote_ip[3] = 255;
  lan_buf_len = sizeof(lan_buf);
  ret = airkiss_lan_pack(AIRKISS_LAN_SSDP_NOTIFY_CMD,
                         (void *)deviceType, (void *)deviceID, 0, 0, lan_buf, &lan_buf_len, &akconf);
  if (ret != AIRKISS_LAN_PAKE_READY) {
    os_printf("Pack lan packet error!");
    return;
  }

  ret = espconn_sendto(&pssdpudpconn, lan_buf, lan_buf_len);
  if (ret != 0) {
    os_printf("UDP send error!");
  }
  os_printf("Finish send notify!\n");
}

LOCAL void ICACHE_FLASH_ATTR
airkiss_wifilan_recv_callbk(void *arg, char *pdata, unsigned short len)
{
  uint16 i;
  remot_info* pcon_info = NULL;

  //airkiss_lan_ret_t ret = airkiss_lan_recv(pdata, len, &akconf);
  //airkiss_lan_ret_t packret;
  int packret;
  int ret = airkiss_lan_recv(pdata, len, &akconf);

  switch (ret) {
    case AIRKISS_LAN_SSDP_REQ:
      espconn_get_connection_info(&pssdpudpconn, &pcon_info, 0);
      os_printf("remote ip: %d.%d.%d.%d \r\n", pcon_info->remote_ip[0], pcon_info->remote_ip[1],
                pcon_info->remote_ip[2], pcon_info->remote_ip[3]);
      os_printf("remote port: %d \r\n", pcon_info->remote_port);

      pssdpudpconn.proto.udp->remote_port = pcon_info->remote_port;
      os_memcpy(pssdpudpconn.proto.udp->remote_ip, pcon_info->remote_ip, 4);
      ssdp_udp.remote_port = CONST_DEFAULT_LAN_PORT;

      lan_buf_len = sizeof(lan_buf);
      packret = airkiss_lan_pack(AIRKISS_LAN_SSDP_RESP_CMD,
                                 (void *)deviceType, (void *) deviceID, 0, 0, lan_buf, &lan_buf_len, &akconf);

      if (packret != AIRKISS_LAN_PAKE_READY) {
        os_printf("Pack lan packet error!");
        return;
      }

      os_printf("\r\n\r\n");
      for (i = 0; i < lan_buf_len; i++)
        os_printf("%c", lan_buf[i]);
      os_printf("\r\n\r\n");

      packret = espconn_sendto(&pssdpudpconn, lan_buf, lan_buf_len);
      if (packret != 0) {
        os_printf("LAN UDP Send err!");
      }

      break;
    default:
      os_printf("Pack is not ssdq req!%d\r\n", ret);
      break;
  }
}

void ICACHE_FLASH_ATTR
airkiss_start_discover(void)
{
  ssdp_udp.local_port = CONST_DEFAULT_LAN_PORT;
  //ghpUdpAirkiss->setLocalPort(CONST_DEFAULT_LAN_PORT);
  pssdpudpconn.type = ESPCONN_UDP;
  pssdpudpconn.proto.udp = &(ssdp_udp);
  espconn_regist_recvcb(&pssdpudpconn, airkiss_wifilan_recv_callbk);
  espconn_create(&pssdpudpconn);

  //os_timer_disarm(&ssdp_time_serv);
  //os_timer_setfn(&ssdp_time_serv, (os_timer_func_t *)airkiss_wifilan_time_callback, NULL);
  //os_timer_arm(&ssdp_time_serv, 1000, 1);//1s
}

#define CONST_UDP_BUFF_SIZE 512 

void setup()
{
  Serial.begin(115000);
  Serial.println("Start module");
  pinMode(LED, OUTPUT);
  digitalWrite(LED, 0);
  smartConfig();
  //ghpUdpAirkiss = new ESP8266Client(UDP, CONST_UDP_BUFF_SIZE);
  airkiss_start_discover();
  //delete ghpUdpAirkiss;
}

void loop()
{
  //airkiss local finding Airkiss 2.0
  if (ulGet_interval(ul5stick) > 5000)
  {
    airkiss_wifilan_time_callback();
    vReset_interval(ul5stick);
  }

  delay(100);
}
