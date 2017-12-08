#include "ESP8266SocketBase.h"
#include "ESP8266Socket.h"
#include "ESP8266ClientDB.h"
#include "ESP8266WiFi.h"


void _onDataCb(ESP8266Socket& client, char *data, unsigned short length)
{
	ESP8266Client *pclt = (ESP8266Client *)client.m_pHandle;
	pclt->dataReceived((uint8_t *)data,length);

	uint32_t ip = pclt->getAddress();
	//Serial.printf("_onDataCb IP=[%x]\n",ip);
}

void _onConnectCb(ESP8266Socket& client)
{
	//Serial.printf("\n~TCP connected~");
	//Serial.printf("%x\n",(ESP8266Client *)client.m_pHandle);
}

void _onDisconnectCb()
{
	//Serial.println("\n~TCP disconnected~");
}

void _onReconnectCb(ESP8266Socket& client, sint8 err)
{
	//Serial.print("reconnect CB: ");
	//Serial.println(err);
}

//----------------------------
//----------------------------
// constructor
//----------------------------
//----------------------------
ESP8266Client::ESP8266Client(espconn_type type,int buffSize)
{
	m_psocket = new ESP8266Socket(type);
	m_psocket->onData(_onDataCb);
	m_psocket->onConnected(_onConnectCb);
	m_psocket->onDisconnected(_onDisconnectCb);
	m_psocket->onReconnect(_onReconnectCb);

	//m_fifo_buffer = new uint8_t[ESP8266_SOCKET_BUFFER_SIZE];
	m_fifo_buffer = new uint8_t[buffSize];
	memset(m_fifo_buffer,0,sizeof(m_fifo_buffer));
	m_fifo_pos_in = 0;
	m_fifo_pos_out = 0;
	m_psocket->m_pHandle = this;
}


ESP8266Client::~ESP8266Client()
{
	delete m_fifo_buffer;
	delete m_psocket;
}

int ESP8266Client::setLocalPort(int port)
{
	m_psocket->setLocalPort(port);
	return 0;
}

uint32_t ESP8266Client::getAddress(void)
{
	return m_psocket->getAddress();
}

int ESP8266Client::setRemote(const char *host, uint16_t port)
{
	IPAddress ip;
	int ret = WiFi.hostByName(host,ip);
	if(ret == 1)
	{
		m_psocket->setPort(port);
		m_psocket->setAddress(int(ip));
	}
	return ret;
}

int ESP8266Client::connect(const char *host, uint16_t port)
{
	IPAddress ip;
	int ret = WiFi.hostByName(host,ip);
	if(ret == 1)
	{
		m_psocket->setPort(port);
		m_psocket->setAddress(int(ip));
		bool res=m_psocket->connect();
		if(res) return 1;
	}
	return 0;
}

int ESP8266Client::reconnect()
{
	bool res=m_psocket->connect();
	m_fifo_pos_in = 0;
	m_fifo_pos_out = 0;
	memset(m_fifo_buffer,0,sizeof(m_fifo_buffer));
	if(res) return 1;
	return 0;
}

size_t ESP8266Client::write(const uint8_t *buf, size_t size)
{
	// waiting TCP connected 100*50 ms
	int i=0;
	while (i < 100) {
		if(m_psocket->isConnected())
		{
			break;
		}
		delay(50);
		i++;
	}
	if(i == 100)
	{
		//Serial.println("\n~TCP timeout~");
		return 0;
	}
	return m_psocket->send((uint8 *)buf,size);
}

int ESP8266Client::available()
{
	int len = 0;
	if(m_fifo_pos_out != m_fifo_pos_in)
	{
		len = (m_fifo_pos_in > m_fifo_pos_out)?(m_fifo_pos_in-m_fifo_pos_out):(m_fifo_pos_in+ESP8266_SOCKET_BUFFER_SIZE-m_fifo_pos_out);
	}
	return len;
}

void ESP8266Client::dataReceived(const uint8_t *buf, size_t size)
{
//	  Serial.println("dataReceived:");
//	  Serial.write(buf, size);

	int pos = 0, pos1 = 0, pos2 = 0;
	size_t len = 0, len1 = 0, len2 = 0;

	pos1 = m_fifo_pos_in;
	pos2 = 0;
	if(m_fifo_pos_in+size > ESP8266_SOCKET_BUFFER_SIZE)
	{
	    len1 = ESP8266_SOCKET_BUFFER_SIZE-m_fifo_pos_in;
		len2 = (m_fifo_pos_in+size) % ESP8266_SOCKET_BUFFER_SIZE;
	}
	else
	{
		len1 = size;
		len2 = 0;
	}
	os_memcpy(m_fifo_buffer+pos1,buf,len1);
	if(len2 > 0)
	{
		os_memcpy(m_fifo_buffer,buf+len1,len2);
	}

	pos = m_fifo_pos_in+size;
	if(pos > ESP8266_SOCKET_BUFFER_SIZE-1)
	{
		pos = pos % ESP8266_SOCKET_BUFFER_SIZE;
	}
	m_fifo_pos_in = pos;
}

int ESP8266Client::read(uint8_t *buf, size_t size)
{
	int pos = 0, pos1 = 0, pos2 = 0;
	size_t len = 0, len1 = 0, len2 = 0, len_read = 0;
	len = available();
	len_read = (len > size)?size:len;

	if(len_read == 0)
	{
		return len_read;
	}

	pos1 = m_fifo_pos_out;
	pos2 = 0;
	if(m_fifo_pos_out+len_read > ESP8266_SOCKET_BUFFER_SIZE)
	{
		len1 = ESP8266_SOCKET_BUFFER_SIZE-m_fifo_pos_out;
		len2 = (m_fifo_pos_out+len_read) % ESP8266_SOCKET_BUFFER_SIZE;
	}
	else
	{
		len1 = len_read;
		len2 = 0;
	}
	os_memcpy(buf,m_fifo_buffer+pos1,len1);
	if(len2 > 0)
	{
		os_memcpy(buf+len1,m_fifo_buffer,len2);
	}

	pos = m_fifo_pos_out+len_read;
	if(pos > ESP8266_SOCKET_BUFFER_SIZE-1)
	{
		pos = pos % ESP8266_SOCKET_BUFFER_SIZE;
	}
	m_fifo_pos_out = pos;

	return len_read;
}

uint8_t ESP8266Client::connected()
{
	return (m_psocket->isConnected()|m_psocket->isConnecting())?1:0;
}

void ESP8266Client::stop()
{
	m_psocket->disconnect();
}
