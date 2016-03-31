#include "Coap.h"
#include "application.h"

#define LOGGING
/* this is as short a delay as I could make it after testing */
#define REBUILD_SOCKET_DELAY_SECONDS 5
#define NO_PACKET_SENT_SIZE 0

Coap::Coap() {
  rebuildSocketTimer = new RobotTimer(false);
  connect();
}

void Coap::connect() {
    connected = true;
    rebuildSocket = true;
    rebuildSocketTimer->stop();
    rebuildSocketTimer->setDuration(REBUILD_SOCKET_DELAY_SECONDS, SECONDS);
    rebuildSocketTimer->start();
}

bool Coap::isConnected() {
  return connected;
}

void Coap::disconnect() {
    connected = false;
    rebuildSocket = false;
    rebuildSocketTimer->stop();
}

bool Coap::start() {
    this->start(COAP_DEFAULT_PORT);
    return true;
}

bool Coap::start(int port) {
    _udp.begin(port);
    return true;
}

bool Coap::isSocketReady() {
  return (connected && !rebuildSocket);
}

bool Coap::socketRequiresRebuild() {
  return (connected && rebuildSocket);
}

uint16_t Coap::sendPacket(CoapPacket &packet, IPAddress ip) {
    return this->sendPacket(packet, ip, COAP_DEFAULT_PORT);
}

uint16_t Coap::sendPacket(CoapPacket &packet, IPAddress ip, int port) {
    if (!isSocketReady()) {
        return NO_PACKET_SENT_SIZE;
    }

    static uint8_t buffer[BUF_MAX_SIZE];
    memset(buffer, 0, sizeof(buffer));
    uint8_t *p = buffer;
    uint16_t running_delta = 0;
    uint16_t packetSize = 0;

    // make coap packet base header
    *p = 0x01 << 6;
    *p |= (packet.type & 0x03) << 4;
    *p++ |= (packet.tokenlen & 0x0F);
    *p++ = packet.code;
    *p++ = (packet.messageid >> 8);
    *p++ = (packet.messageid & 0xFF);
    p = buffer + COAP_HEADER_SIZE;
    packetSize += 4;

    // make token
    if (packet.token != NULL && packet.tokenlen <= 0x0F) {
        memcpy(p, packet.token, packet.tokenlen);
        p += packet.tokenlen;
        packetSize += packet.tokenlen;
    }

    // make option header
    for (int i = 0; i < packet.optionnum; i++)  {
        uint32_t optdelta;
        uint8_t len, delta;

        if (packetSize + 5 + packet.options[i].length >= BUF_MAX_SIZE) {
            Serial.println("Packet over BUF_MAX_SIZE size");
            return 0;
        }
        optdelta = packet.options[i].number - running_delta;
        COAP_OPTION_DELTA(optdelta, &delta);
        COAP_OPTION_DELTA((uint32_t)packet.options[i].length, &len);

        *p++ = (0xFF & (delta << 4 | len));
        if (delta == 13) {
            *p++ = (optdelta - 13);
            packetSize++;
        } else if (delta == 14) {
            *p++ = ((optdelta - 269) >> 8);
            *p++ = (0xFF & (optdelta - 269));
            packetSize+=2;
        } if (len == 13) {
            *p++ = (packet.options[i].length - 13);
            packetSize++;
        } else if (len == 14) {
            *p++ = (packet.options[i].length >> 8);
            *p++ = (0xFF & (packet.options[i].length - 269));
            packetSize+=2;
        }

        memcpy(p, packet.options[i].buffer, packet.options[i].length);
        p += packet.options[i].length;
        packetSize += packet.options[i].length + 1;
        running_delta = packet.options[i].number;
    }

    // make payload
    if (packet.payloadlen > 0) {
        if ((packetSize + 1 + packet.payloadlen) >= BUF_MAX_SIZE) {
            Serial.println("Packet over BUF_MAX_SIZE size");
            return 0;
        }
        *p++ = 0xFF;
        memcpy(p, packet.payload, packet.payloadlen);
        packetSize += 1 + packet.payloadlen;
    }

    if(connected && !rebuildSocket) {
      _udp.beginPacket(ip, port);
      _udp.write(buffer, packetSize);

      int retval = _udp.endPacket();

      if (retval < 0) {
        errorCodeCount++;
        /* connect acts as reconnect */
        connect();
      } else {
        correctPacketCount++;
      }
    }

    return packet.messageid;
}

uint16_t Coap::get(IPAddress ip, int port, char *url) {
    return this->send(ip, port, url, COAP_TYPE::COAP_CON, COAP_METHOD::COAP_GET, NULL, 0, NULL, 0);
}

uint16_t Coap::post(IPAddress ip, int port, char *url, char *payload, int payloadlen) {
    return this->send(ip, port, url, COAP_TYPE::COAP_NONCON, COAP_METHOD::COAP_POST, NULL, 0, (uint8_t *)payload, payloadlen);
}

uint16_t Coap::send(IPAddress ip, int port, char *url, COAP_TYPE type, COAP_METHOD method, uint8_t *token, uint8_t tokenlen, uint8_t *payload, uint32_t payloadlen) {
    DEBUG("Coap::send > make packet");

    if (!isSocketReady()) {
        return NO_PACKET_SENT_SIZE;
    }

    // make packet
    CoapPacket packet;

    packet.type = type;
    packet.code = method;
    packet.token = token;
    packet.tokenlen = tokenlen;
    packet.payload = payload;
    packet.payloadlen = payloadlen;
    packet.optionnum = 0;
    packet.messageid = rand();

    // if more options?
    packet.options[packet.optionnum].buffer = (uint8_t *)url;
    packet.options[packet.optionnum].length = strlen(url);
    packet.options[packet.optionnum].number = COAP_OPTION_NUMBER::COAP_URI_PATH;
    packet.optionnum++;

    // send packet
    return this->sendPacket(packet, ip, port);
}

int Coap::parseOption(CoapOption *option, uint16_t *running_delta, uint8_t **buf, size_t buflen) {
    uint8_t *p = *buf;
    uint8_t headlen = 1;
    uint16_t len, delta;

    if (buflen < headlen) return -1;

    delta = (p[0] & 0xF0) >> 4;
    len = p[0] & 0x0F;

    if (delta == 13) {
        headlen++;
        if (buflen < headlen) return -1;
        delta = p[1] + 13;
        p++;
    } else if (delta == 14) {
        headlen += 2;
        if (buflen < headlen) return -1;
        delta = ((p[1] << 8) | p[2]) + 269;
        p+=2;
    } else if (delta == 15) return -1;

    if (len == 13) {
        headlen++;
        if (buflen < headlen) return -1;
        len = p[1] + 13;
        p++;
    } else if (len == 14) {
        headlen += 2;
        if (buflen < headlen) return -1;
        len = ((p[1] << 8) | p[2]) + 269;
        p+=2;
    } else if (len == 15)
        return -1;

    if ((p + 1 + len) > (*buf + buflen))  return -1;
    option->number = delta + *running_delta;
    option->buffer = p+1;
    option->length = len;
    *buf = p + 1 + len;
    *running_delta += delta;

    return 0;
}

bool Coap::loop() {
    int32_t packetlen = 0;
    static uint8_t buffer[BUF_MAX_SIZE];
    memset(buffer, 0, sizeof(buffer));

    if(isSocketReady()) {
        packetlen = _udp.parsePacket();
    }

    if (socketRequiresRebuild() && rebuildSocketTimer->isComplete()) {
        /* only rebuild UDP socket if we are connected to WiFi */
        Serial.println("Rebuilding Socket");
        _udp.stop();
        _udp.begin(COAP_DEFAULT_PORT);
        rebuildSocket = false;
        rebuildSocketTimer->stop();
    }

    while (packetlen > 0) {
        packetlen = _udp.read(buffer, packetlen >= BUF_MAX_SIZE ? BUF_MAX_SIZE : packetlen);

        CoapPacket packet;

        // parse coap packet header
        if (packetlen < COAP_HEADER_SIZE || (((buffer[0] & 0xC0) >> 6) != 1)) {
            packetlen = _udp.parsePacket();
            continue;
        }

        packet.type = (buffer[0] & 0x30) >> 4;
        packet.tokenlen = buffer[0] & 0x0F;
        packet.code = buffer[1];
        packet.messageid = 0xFF00 & (buffer[2] << 8);
        packet.messageid |= 0x00FF & buffer[3];

        if (packet.tokenlen == 0)  packet.token = NULL;
        else if (packet.tokenlen <= 8)  packet.token = buffer + 4;
        else {
            packetlen = _udp.parsePacket();
            continue;
        }

        // parse packet options/payload
        if (COAP_HEADER_SIZE + packet.tokenlen < packetlen) {
            int optionIndex = 0;
            uint16_t delta = 0;
            uint8_t *end = buffer + packetlen;
            uint8_t *p = buffer + COAP_HEADER_SIZE + packet.tokenlen;
            while(optionIndex < MAX_OPTION_NUM && *p != 0xFF && p < end) {
                //packet.options[optionIndex];
                if (0 != parseOption(&packet.options[optionIndex], &delta, &p, end-p))
                    return false;
                optionIndex++;
            }
            packet.optionnum = optionIndex;

            if (p+1 < end && *p == 0xFF) {
                packet.payload = p+1;
                packet.payloadlen = end-(p+1);
            } else {
                packet.payload = NULL;
                packet.payloadlen= 0;
            }
        }

        if (packet.type == COAP_TYPE::COAP_ACK) {
            // call response function
            resp(packet, _udp.remoteIP(), _udp.remotePort());

        } else if (packet.type == COAP_TYPE::COAP_CON) {
            // call endpoint url function
            String url = "";
            String query = "";
            for (int i = 0; i < packet.optionnum; i++) {
                if (packet.options[i].number == COAP_URI_PATH && packet.options[i].length > 0) {
                    char urlname[packet.options[i].length + 1];
                    memcpy(urlname, packet.options[i].buffer, packet.options[i].length);
                    urlname[packet.options[i].length] = '\0';
                    if (strlen(url) == 0) {
                        url += String(urlname);
                    } else {
                        url += "/" + String(urlname);
                    }
                } else if (packet.options[i].number == COAP_URI_QUERY && packet.options[i].length > 0) {
                    char urlquery[packet.options[i].length + 1];
                    memcpy(urlquery, packet.options[i].buffer, packet.options[i].length);
                    urlquery[packet.options[i].length] = '\0';
                    query += String(urlquery);
                }
            }
            if (uri.find(url) == uri.end()) {
                DEBUG("can't find endpoint url");
            } else {
                uri[url](packet, _udp.remoteIP(), _udp.remotePort());
            }
        }

        // next packet
        packetlen = _udp.parsePacket();
    }

    return true;
}

uint16_t Coap::sendResponse(IPAddress ip, int port, uint16_t messageid) {
    return this->sendResponse(ip, port, messageid, NULL, 0, COAP_RESPONSE_CODE::COAP_CREATED, COAP_CONTENT_TYPE::COAP_TEXT_PLAIN, NULL, 0);
}

uint16_t Coap::sendResponse(IPAddress ip, int port, uint16_t messageid, uint8_t *payload, int payloadlen) {
    return this->sendResponse(ip, port, messageid, payload, payloadlen, COAP_RESPONSE_CODE::COAP_CREATED, COAP_CONTENT_TYPE::COAP_TEXT_PLAIN, NULL, 0);
}


uint16_t Coap::sendResponse(IPAddress ip, int port, uint16_t messageid, uint8_t *payload, int payloadlen,
                COAP_RESPONSE_CODE code, COAP_CONTENT_TYPE type, uint8_t *token, int tokenlen) {
    // make packet
    CoapPacket packet;

    packet.type = COAP_TYPE::COAP_ACK;
    packet.code = code;
    packet.token = token;
    packet.tokenlen = tokenlen;
    packet.payload = payload;
    packet.payloadlen = payloadlen;
    packet.optionnum = 0;
    packet.messageid = messageid;

    // if more options?
    char optionBuffer[2];
    optionBuffer[0] = ((uint16_t)type & 0xFF00) >> 8;
    optionBuffer[1] = ((uint16_t)type & 0x00FF) ;
    packet.options[packet.optionnum].buffer = (uint8_t *)optionBuffer;
    packet.options[packet.optionnum].length = 2;
    packet.options[packet.optionnum].number = COAP_OPTION_NUMBER::COAP_CONTENT_FORMAT;
    packet.optionnum++;

    return this->sendPacket(packet, ip, port);
}
