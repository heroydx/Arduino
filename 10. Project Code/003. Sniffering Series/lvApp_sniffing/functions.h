// This-->tab == "functions.h"

// Expose Espressif SDK functionality
extern "C" {
#include "./user_interface.h"
  typedef void (*freedom_outside_cb_t)(uint8 status);
  int  wifi_register_send_pkt_freedom_cb(freedom_outside_cb_t cb);
  void wifi_unregister_send_pkt_freedom_cb(void);
  int  wifi_send_pkt_freedom(uint8 *buf, int len, bool sys_seq);
  extern unsigned short gnSecondSeq;
}

#include <ESP8266WiFi.h>
#include "./structures.h"

#define CONST_APP_SNIFF_MAX_APS_TRACKED 10
#define CONST_APP_SNIFF_MAX_CLIENTS_TRACKED 240
#define CONST_APP_SNIFF_MIN_SIGNAL -100

uint8_t homeSSID[CONST_SSID_LEN];
uint8_t homeMAC[CONST_ETH_MAC_LEN];
unsigned int homeChannel = 1;
signed minSingalThreshold = CONST_APP_SNIFF_MIN_SIGNAL;


beaconinfo aps_known[CONST_APP_SNIFF_MAX_APS_TRACKED];                    // Array to save MACs of known APs
int aps_known_count = 0;                                  // Number of known APs
int nothing_new = 0;
clientinfo clients_known[CONST_APP_SNIFF_MAX_CLIENTS_TRACKED];            // Array to save MACs of known CLIENTs
int clients_known_count = 0;                              // Number of known CLIENTs

void clean_beacon()
{
  aps_known_count = 0;
  memset(aps_known, 0, sizeof(aps_known));
}

void clean_client()
{
  clients_known_count = 0;
  memset(clients_known, 0, sizeof(clients_known));
}

void clean_data()
{
  clean_client();
  clean_beacon();

}

int register_beacon(beaconinfo beacon)
{
  int known = 0;   // Clear known flag
  for (int u = 0; u < aps_known_count; u++)
  {
    if (! memcmp(aps_known[u].bssid, beacon.bssid, CONST_ETH_MAC_LEN))
    {
      known = 1;
      //aps_known[u].count++;
      break;

    }   // AP known => Set known flag
  }
  if (! known)  // AP is NEW, copy MAC to array and return it
  {
    memcpy(&aps_known[aps_known_count], &beacon, sizeof(beacon));
    //aps_known[aps_known_count].count = 1;
    aps_known[aps_known_count].timeStamp = gnSecondSeq;
    aps_known_count++;

    if ((unsigned int) aps_known_count >=
        sizeof (aps_known) / sizeof (aps_known[0]) )
    {
#ifdef DEBUG_SIO
      DBGPRINT("exceeded max aps_known\n");
#endif
      aps_known_count = 0;
    }

  }
  return known;
}

int register_client(clientinfo ci)
{
  int known = 0;   // Clear known flag
  for (int u = 0; u < clients_known_count; u++)
  {
    if (! memcmp(clients_known[u].station, ci.station, CONST_ETH_MAC_LEN)) {
      known = 1;
      //clients_known[u].count++;
      break;
    }
  }
  if (! known)
  {
    memcpy(&clients_known[clients_known_count], &ci, sizeof(ci));
    //clients_known[clients_known_count].count = 1;
    clients_known[clients_known_count].timeStamp = gnSecondSeq;
    clients_known_count++;

    if ((unsigned int) clients_known_count >=
        sizeof (clients_known) / sizeof (clients_known[0]) )
    {
#ifdef DEBUG_SIO
      DBGPRINT("exceeded max clients_known\n");
#endif
      clients_known_count = 0;
    }
  }
  return known;
}

void print_beacon(beaconinfo beacon)
{
#ifdef DEBUG_SIO
  if (beacon.err != 0) {
    //Serial.printf("BEACON ERR: (%d)  ", beacon.err);
  }
  else
  {
    DBGPRINTF("BEACON: <=============== [%32s]  ", beacon.ssid);
    for (int i = 0; i < 6; i++)
    {
      DBGPRINTF("%02x", beacon.bssid[i]);
    }
    DBGPRINTF("   %2d", beacon.channel);
    DBGPRINTF("   %4d", beacon.rssi);
    //DBGPRINTF("   %6d", beacon.count);
    DBGPRINTF("   %6d", beacon.timeStamp);
    DBGPRINT("\r\n");
  }
#endif
}

void print_client(clientinfo ci)
{
#ifdef DEBUG_SIO
  int u = 0;
  int known = 0;   // Clear known flag
  if (ci.err != 0)
  {
    // nothing
  }
  else
  {
    DBGPRINT("DEVICE: ");
    for (int i = 0; i < 6; i++)
      DBGPRINTF("%02x", ci.station[i]);
    DBGPRINT(" ==> ");
    for (u = 0; u < aps_known_count; u++)
    {
      if (! memcmp(aps_known[u].bssid, ci.bssid, CONST_ETH_MAC_LEN))
      {
        DBGPRINTF("[%32s]", aps_known[u].ssid);
        known = 1;     // AP known => Set known flag
        break;
      }
    }

    if (! known)
    {
      //DBGPRINT("   Unknown/Malformed packet \r\n");
      DBGPRINTF("[%32s]  ", "Unknown/Malformed packet");
      //for (int i = 0; i < 6; i++) DBGPRINTF("%02x", ci.bssid[i]);
      for (int i = 0; i < 6; i++)
        DBGPRINTF("%02x", ci.bssid[i]);
    }
    else
    {
      DBGPRINTF("%2s", " ");
      for (int i = 0; i < 6; i++)
        DBGPRINTF("%02x", ci.ap[i]);
    }
    DBGPRINTF("  %3d", aps_known[u].channel);
    DBGPRINTF("   %4d", ci.rssi);
    //DBGPRINTF("   %6d", ci.count);
    DBGPRINTF("   %6d", ci.timeStamp);
    DBGPRINT("\r\n");
  }
#endif
}

void promisc_cb(uint8_t *buf, uint16_t len)
{
  int i = 0;
  uint16_t seq_n_new = 0;
  if (len == 12)
  {
    struct RxControl *sniffer = (struct RxControl*) buf;
  }
  else if (len == 128)
  {
    struct sniffer_buf2 *sniffer = (struct sniffer_buf2*) buf;
    struct beaconinfo beacon = parse_beacon(sniffer->buf, 112, sniffer->rx_ctrl.rssi);
    //only home SSID
    if (!memcmp(beacon.ssid, homeSSID, sizeof(homeSSID)))
    {
      if (register_beacon(beacon) == 0)
      {
        print_beacon(beacon);
        nothing_new = 0;
      }
    }
  }
  else
  {
    struct sniffer_buf *sniffer = (struct sniffer_buf*) buf;
    //Is data or QOS?
    if ((sniffer->buf[0] == 0x08) || (sniffer->buf[0] == 0x88))
    {
      struct clientinfo ci = parse_data(sniffer->buf, 36, sniffer->rx_ctrl.rssi, sniffer->rx_ctrl.channel);
      if (memcmp(ci.bssid, ci.station, CONST_ETH_MAC_LEN))
      {
        //only home SSID
        int isHomeDevice = 0;
        for (i = 0; i < aps_known_count; i++)
        {
          if (!memcmp(ci.bssid, aps_known[i].bssid, CONST_ETH_MAC_LEN))
          {
            isHomeDevice = 1;
            break;
          }
        }
        //or very closed device
        if (ci.rssi > minSingalThreshold)
        {
          isHomeDevice = 1;
        }
        if (isHomeDevice)
        {
          if (register_client(ci) == 0)
          {
            print_client(ci);
            nothing_new = 0;
          }
        }
      }
    }
  }
}

