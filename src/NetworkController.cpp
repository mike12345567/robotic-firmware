#include "NetworkController.h"
#include "RoboticFirmware.h"
#include "EventController.h"

NetworkController::NetworkController() {

  coap.server(callbackMakeMove, "makeMove");
  coap.start();
}

void NetworkController::sendPackedBytes(char *url, uint8_t *bytes, uint32_t bytesSize) {
  // TODO: may want to store message IDs (if responses are needed)
  coap.post(ip, COAP_DEFAULT_PORT, url, bytes, bytesSize);
}

void NetworkController::sendCoapResponse(IPAddress ip, int port, uint16_t messageid, uint8_t* payload, int payloadlen,
                                         COAP_RESPONSE_CODE code, COAP_CONTENT_TYPE type) {
 coap.sendResponse(ip, port, messageid, payload, payloadlen, code, type, NULL, 0 );
}

void callbackMakeMove(CoapPacket &packet, IPAddress ip, int port) {
  NetworkController* networkController = getEventController()->getNetworkController();

  String params = (const char*)packet.payload;
  makeMove(params);

  networkController->sendCoapResponse(ip, port, packet.messageid, NULL, 0, COAP_RESPONSE_CODE::COAP_CREATED, COAP_CONTENT_TYPE::COAP_TEXT_PLAIN);
}

void NetworkController::process() {
  coap.loop();
}
