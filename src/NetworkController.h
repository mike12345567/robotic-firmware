#ifndef NETWORKCONTROLLER_H_
#define NETWORKCONTROLLER_H_

#include "libraries/Coap/Coap.h"

void callbackCommand(CoapPacket &packet, IPAddress ip, int port);
void callbackResponse(CoapPacket &packet, IPAddress ip, int port);

class NetworkController {
  private:
    IPAddress ip = IPAddress(234, 234, 234, 234);
    Coap coap;
  public:
    NetworkController();

    void sendCoapResponse(IPAddress ip, int port, uint16_t messageid, uint8_t* payload, int payloadlen,
                          COAP_RESPONSE_CODE code, COAP_CONTENT_TYPE type);
    void sendPackedBytes(char *url, char *bytes, uint32_t bytesSize);
    void process();
};

#endif
