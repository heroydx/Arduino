#ifndef ESP8266_CLIENT
#define ESP8266_CLIENT


extern "C" {
	#include "ets_sys.h"
	#include "osapi.h"
	#include "mem.h"
	#include "ip_addr.h"
	#include "espconn.h"
  
  //#include "miscCommon.h"
}

#include <ESP8266Socket.h>

#define ESP8266_SOCKET_BUFFER_SIZE 1024*3

//class ESP8266Socket;

class ESP8266Client
{
public:
  ESP8266Client(espconn_type type,int buffSize);
  virtual ~ESP8266Client();
  int setLocalPort(int port);
  uint32_t getAddress();
  int setRemote(const char *host, uint16_t port);
  int connect(const char *host, uint16_t port);
  int reconnect();

  size_t write(const uint8_t *buf, size_t size);

  void dataReceived(const uint8_t *buf, size_t size);
  int available();
  int read(uint8_t *buf, size_t size);

  uint8_t connected();
  void stop();
	
protected:
	
private:
//	uint8_t 	m_fifo_buffer[ESP8266_SOCKET_BUFFER_SIZE];
	uint8_t 	*m_fifo_buffer;
	int         m_fifo_pos_in;
	int         m_fifo_pos_out;
	ESP8266Socket *m_psocket;
};


#endif
