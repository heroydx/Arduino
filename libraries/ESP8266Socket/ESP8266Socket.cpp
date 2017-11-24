#include "ESP8266Socket.h"


ESP8266Socket::ESP8266Socket(const char* addr, int port, espconn_type type) :
	ESP8266SocketBase(type)
{
	setPort(port);
	setAddress(ipaddr_addr(addr));
}

ESP8266Socket::ESP8266Socket(espconn_type type) :ESP8266SocketBase(type)
,m_bIsConnected(false)
,m_bIsConnecting(false)
,remotePort(0)
{
}

ESP8266Socket::ESP8266Socket(struct espconn* _esp_conn) : ESP8266SocketBase(_esp_conn)
	,m_bIsConnected(true)
	,m_bIsConnecting(false)
{
	// set remotePort from espconn
	if (isTcp()) {
		remotePort = esp_conn->proto.tcp->remote_port;
	} else if (isUdp()) {
		remotePort = esp_conn->proto.udp->remote_port;
	}
}


ESP8266Socket::~ESP8266Socket()
{
	if (!m_bIsExternal) {
		disconnect();
	}
}


void ESP8266Socket::setAddress(uint32_t address)
{
	if (isTcp()) {
		os_memcpy(esp_conn->proto.tcp->remote_ip, &address, 4);
	} else if (isUdp()) {
		os_memcpy(esp_conn->proto.udp->remote_ip, &address, 4);
	}
}

void ESP8266Socket::setAddress(uint8 ip0, uint8 ip1, uint8 ip2, uint8 ip3)
{
	if (isTcp()) {

		// set remote ip
		esp_conn->proto.tcp->remote_ip[0] = ip0;
		esp_conn->proto.tcp->remote_ip[1] = ip1;
		esp_conn->proto.tcp->remote_ip[2] = ip2;
		esp_conn->proto.tcp->remote_ip[3] = ip3;

	} else if (isUdp()) {
		
		// set remote ip
		esp_conn->proto.udp->remote_ip[0] = ip0;
		esp_conn->proto.udp->remote_ip[1] = ip1;
		esp_conn->proto.udp->remote_ip[2] = ip2;
		esp_conn->proto.udp->remote_ip[3] = ip3;
	}
}

uint32_t ESP8266Socket::getAddress()
{
	uint32_t addr = 0;
	
	if (isTcp()) {
		os_memcpy(&addr, esp_conn->proto.tcp->remote_ip, 4);
	} else if (isUdp()) {
//		os_memcpy(&addr, esp_conn->proto.udp->remote_ip, 4);
		remot_info *premot = NULL;
		if (espconn_get_connection_info(esp_conn,&premot,0) == ESPCONN_OK)
		{
			os_memcpy(&addr, premot->remote_ip, 4);
		}
	}
	return addr;
}

// port
void ESP8266Socket::setPort(int port)
{
	remotePort = port;
	
	if (isTcp()) {
		esp_conn->proto.tcp->remote_port = port;
		if(esp_conn->proto.tcp->local_port == 0)
		esp_conn->proto.tcp->local_port = espconn_port();
	} else if (isUdp()) {
		
		esp_conn->proto.udp->remote_port = port;
		if(esp_conn->proto.udp->local_port == 0)
		esp_conn->proto.udp->local_port = espconn_port();
	}
}

void ESP8266Socket::setLocalPort(int port)
{
	if (isTcp()) {
		esp_conn->proto.tcp->local_port = port;
	} else if (isUdp()) {
		esp_conn->proto.udp->local_port = port;
	}
}

int ESP8266Socket::getPort()
{
	if (isTcp()) {
		return esp_conn->proto.tcp->remote_port;
	} else if (isUdp()) {
		return esp_conn->proto.udp->remote_port;
	}
	
	return remotePort;
}


//----------------------------
// connect / disconnect
//----------------------------
bool ESP8266Socket::connect()
{
	// already connecting?
	if (m_bIsConnecting) {
		return false;
	}
	
	sint8 res = ESPCONN_OK;
	
	// if connected... disconnect first
	if (m_bIsConnected) {
		disconnect();
	}

	if (isTcp()) {
		res = espconn_connect(esp_conn);
		if (res == ESPCONN_OK) {
				m_bIsConnected = false;
				m_bIsConnecting = true;
		}
		else
		{
//asm("break 0,0");
		}
	} else if (isUdp()) {
		res = espconn_create(esp_conn);
		m_bIsConnected = true;
		m_bIsConnecting = false;
	}
	
	if (res != ESPCONN_OK) {
		error("could not connect: ", res);
	}
	
	return res == ESPCONN_OK;
}

bool ESP8266Socket::disconnect()
{
	sint8 res = ESPCONN_OK;
	
	m_bIsConnecting = false;
	
	if (isTcp()) {
		res = espconn_disconnect(esp_conn);
		m_bIsConnected = false;
	} else {
		espconn_delete(esp_conn);
		m_bIsConnected = false;
	}
	
	if (res != ESPCONN_OK) {
		error("could not disconnect: ", res);
	}
	
	return res == ESPCONN_OK;
}


//----------------------------
//----------------------------
// espconn callbacks
//----------------------------
//----------------------------
void ESP8266Socket::_onClientDataCb(struct espconn *pesp_conn, char *data, unsigned short length)
{
	if (onClientDataCb != 0) {
		onClientDataCb(*this, data, length);
	}
}

//----------------------------
//----------------------------
// TCP callbacks
//----------------------------
//----------------------------
void ESP8266Socket::_onClientConnectCb(struct espconn *pesp_conn_client)
{
	m_bIsConnected = true;
	m_bIsConnecting = false;
	
	if (onClientConnectCb != 0) {
		onClientConnectCb(*this);
	}
}

void ESP8266Socket::_onClientDisconnectCb(struct espconn *pesp_conn_client)
{
	// set internal state
	m_bIsConnected = false;
	m_bIsConnecting = false;
	
	if (onClientDisconnectCb != 0) {
		onClientDisconnectCb();
	}
}

void ESP8266Socket::_onClientReconnectCb(struct espconn *pesp_conn_client, sint8 err)
{
	// set internal state
	m_bIsConnected = false;
	m_bIsConnecting = false;
	
	if (onClientReconnectCb != 0) {
		onClientReconnectCb(*this, err);
	}
}

