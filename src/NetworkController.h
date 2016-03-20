#ifndef NETWORKCONTROLLER_H_
#define NETWORKCONTROLLER_H_

#include "libraries/Coap/Coap.h"

void callbackMakeMove(CoapPacket &packet, IPAddress ip, int port);

class NetworkController {
  private:
    IPAddress ip = IPAddress(224, 27, 39, 26);
    Coap coap;
  public:
    NetworkController();

    void sendCoapResponse(IPAddress ip, int port, uint16_t messageid, uint8_t* payload, int payloadlen,
                          COAP_RESPONSE_CODE code, COAP_CONTENT_TYPE type);
    void sendPackedBytes(char *url, uint8_t *bytes, uint32_t bytesSize);
    void process();
};

#endif
