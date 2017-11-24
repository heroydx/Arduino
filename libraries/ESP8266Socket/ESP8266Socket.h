#ifndef ESP8266_SOCKET
#define ESP8266_SOCKET

#include "ESP8266SocketBase.h"

extern "C" {
	#include "ets_sys.h"
	#include "osapi.h"
	#include "mem.h"
	#include "ip_addr.h"
	#include "espconn.h"
}

/* TODO
 - remotePort necessary??
 - use internal buffer for data to send... if buffer is full yield?
 - use secure calls
 */


class ESP8266Socket : public ESP8266SocketBase
{
public:
	ESP8266Socket(const char* addr, int port, espconn_type type = TCP);
	ESP8266Socket(espconn_type type);
	ESP8266Socket(struct espconn* _esp_conn);
	~ESP8266Socket();
	
	void setAddress(uint32_t address);
	void setAddress(uint8 ip0, uint8 ip1, uint8 ip2, uint8 ip3);
	void setPort(int port);
	void setLocalPort(int port);
	
	uint32_t getAddress();
	int getPort();
	
	bool connect();
	bool disconnect();

	bool isConnected() { return m_bIsConnected; };
	bool isConnecting() { return m_bIsConnecting; };
	
	//----------------------------
	// internal callbacks - override
	void _onClientDataCb(struct espconn *pesp_conn, char *data, unsigned short length);
	
	void _onClientConnectCb(struct espconn *pesp_conn_client);
	void _onClientDisconnectCb(struct espconn *pesp_conn_client);
	void _onClientReconnectCb(struct espconn *pesp_conn_client, sint8 err);
	
	void *m_pHandle;
private:
	int					remotePort;
	
	bool				m_bIsConnected;
	bool				m_bIsConnecting;
};


#endif
