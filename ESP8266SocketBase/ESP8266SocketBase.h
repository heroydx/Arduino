#ifndef ESP8266_SOCKET_BASE
#define ESP8266_SOCKET_BASE

#include <stddef.h>
#include <string.h>


extern "C" {
	#include "ets_sys.h"
	#include "osapi.h"
	#include "mem.h"
	#include "ip_addr.h"
	#include "espconn.h"

	extern void esp_schedule();
	extern void esp_yield();
}


#define TCP ESPCONN_TCP
#define UDP ESPCONN_UDP


class ESP8266Socket;

class ESP8266SocketBase
{
public:
	// constructor, deconstructor
	ESP8266SocketBase(espconn_type type = TCP);
	ESP8266SocketBase(struct espconn* _esp_conn);
	~ESP8266SocketBase();

	// methods
	bool isTcp() { return esp_conn->type == ESPCONN_TCP; };
	bool isUdp() { return esp_conn->type == ESPCONN_UDP; };
	
	virtual sint8 send(uint8 *data, uint16 length);
	virtual sint8 send(const char* data);
	
	bool isSending() { return esp_conn->state == ESPCONN_WRITE; };
	
	//----------------------------
	// set user callback functions
	// general
	void onSent( void(*)() );
	void onData( void(*)(ESP8266Socket&, char *, unsigned short) );
	
	// tcp callbacks
	void onConnected( void(*)(ESP8266Socket&) );
	void onDisconnected( void(*)() );
	void onReconnect( void(*)(ESP8266Socket&, sint8) );
	
	// info, error
	void onInfo( void(*)(const char*) );
	void onError( void(*)(const char*, sint8) );
	
	//----------------------------
	// internal callbacks
	// general
	virtual void _onClientSentCb();
	virtual void _onClientDataCb(struct espconn *pesp_conn, char *data, unsigned short length) = 0;
	
	// tcp callbacks
	virtual void _onClientConnectCb(struct espconn *pesp_conn_client) = 0;
	virtual void _onClientDisconnectCb(struct espconn *pesp_conn_client) = 0;
	virtual void _onClientReconnectCb(struct espconn *pesp_conn_client, sint8 err) = 0;
	
protected:
	void info(const char* info);
	void error(const char* error, sint8 err);
	
	struct espconn*		esp_conn;
	bool				m_bIsExternal;
	
	// user callback functions
	// general
	void (*onClientSentCb)() = 0;
	void (*onClientDataCb)(ESP8266Socket& client, char *data, unsigned short length) = 0;
	
	// tcp callbacks
	void (*onClientConnectCb)(ESP8266Socket& client) = 0;
	void (*onClientDisconnectCb)() = 0;
	void (*onClientReconnectCb)(ESP8266Socket& client, sint8 err) = 0;
	
	// info, error
	void (*onInfoCb)(const char* error) = 0;
	void (*onErrorCb)(const char* error, sint8 err) = 0;
	
private:
	void* reverse_external;
	
};


#endif
