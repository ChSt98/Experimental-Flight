#include "kraft_kommunication.h"



bool KraftKommunication::decodeMessageFromBuffer(ReceivedPayloadData* payloadData, bool* ackRequested, uint8_t* buffer, uint32_t bufferSize) {

    if (buffer[0] != (uint8_t)(c_kraftPacketStartMarker + c_kraftPacketVersion)) return false;
    if (buffer[1] != (uint8_t)((c_kraftPacketStartMarker + c_kraftPacketVersion)<<8)) return false;


    payloadData->messageData.transmitterID = static_cast<eKraftPacketNodeID_t>(buffer[2]);
    payloadData->messageData.receiverID = static_cast<eKraftPacketNodeID_t>(buffer[3]);

    if (payloadData->messageData.transmitterID == eKraftPacketNodeID_t::eKraftPacketNodeID_broadcast) return false; //That would be pretty wierd if the sender was a broadcast id

    payloadData->messageData.payloadID = buffer[4];

    *ackRequested = buffer[5];
    
    payloadData->messageData.messageCounter = buffer[6];

    uint32_t payloadSize = buffer[7];

    if (payloadSize > bufferSize) return false;
    
    for (uint32_t i = 0; i < payloadSize; i++) payloadData->dataBuffer[i] = buffer[8+i]; //Move data from buffer into output

    uint8_t crc = buffer[8+payloadSize];

    if (buffer[9+payloadSize] != c_kraftPacketEndMarker) return false;

    //if (crc != calculateCRC(buffer, payloadSize + 7)) return false; //Commented for testing, Should be readded and tested later!

    return true;

}



uint32_t KraftKommunication::encodeMessageToBuffer(KraftMessage_Interface* messagePointer, const uint8_t &receiveNode, const bool &requestAck, uint8_t* buffer, const uint32_t &bufferSize) {

    if (bufferSize < 9 + messagePointer->getDataSize()) return 0; //Make sure dataByte is big enough to fit all the data.

    buffer[0] = (uint8_t)(c_kraftPacketStartMarker + c_kraftPacketVersion);
    buffer[1] = (uint8_t)((c_kraftPacketStartMarker + c_kraftPacketVersion)<<8);

    buffer[2] = selfID_;
    buffer[3] = receiveNode;

    buffer[4] = messagePointer->getDataTypeID();

    buffer[5] = requestAck;

    buffer[6] = sentPacketsCounter_[receiveNode];

    uint32_t messageSize = messagePointer->getDataSize();
    buffer[7] = messageSize;

    uint8_t messageData[messageSize];

    messagePointer->getRawData(messageData, messageSize);
    
    for (uint32_t i = 0; i < sizeof(bufferSize) && messageSize; i++) buffer[8+i] = messageData[i]; //Move data from buffer into output

    buffer[messageSize + 8] = calculateCRC(buffer, messageSize + 7);

    buffer[messageSize + 9] = c_kraftPacketEndMarker;

    return messageSize + 9 + 1;

}



uint8_t KraftKommunication::calculateCRC(uint8_t* buffer, const uint32_t &stopByte) {

    uint8_t crc = 0;

    for (uint32_t i = 0; i < stopByte; i++) crc += buffer[i];

    return crc;

}



bool KraftKommunication::sendMessage(KraftMessage_Interface* kraftMessage, const eKraftPacketNodeID_t &receiveNodeID, const bool &requiresAck) {

    SendPacketData packetData;

    packetData.bufferSize = encodeMessageToBuffer(kraftMessage, receiveNodeID, false, packetData.dataBuffer, sizeof(packetData.dataBuffer));

    if (packetData.bufferSize == 0) return false; //encode failure if buffer size is 0. Return false to indicate failure.

    packetData.power_dB = c_sendPowerDefault;
    packetData.receivingNodeID = receiveNodeID;
    packetData.waitforAck = requiresAck;

    if (packetData.waitforAck) {

        packetData.sendAttempts = c_sendAttempts;
        packetData.sendInterval = c_sendTimeout/c_sendAttempts;

    }

    if (requiresAck) sendPacketsACK_.push_front(packetData);
    else sendPackets_.push_front(packetData);

    return true;

}



bool KraftKommunication::getMessage(KraftMessage_Interface* kraftMessage, const bool &peek) {

    ReceivedPayloadData payloadData;
    
    if (!peek) payloadData = receivedPackets_.pop_back();
    else payloadData = *receivedPackets_.peek_back();

    return kraftMessage->setRawData(payloadData.dataBuffer, sizeof(payloadData.dataBuffer));

}



void KraftKommunication::loop() {


    if (!dataLink_->busy()) {

        if (sendPackets_.available()) {

            SendPacketData packet = sendPackets_.pop_back();
            dataLink_->sendBuffer(packet.dataBuffer, packet.bufferSize);
            
        } else if (sendPacketsACK_.available()) {

            SendPacketData* packet = sendPacketsACK_.peek_back();

            if (nodeData_[packet->receivingNodeID].waitingOnPacket == nullptr) { 

                dataLink_->sendBuffer(packet->dataBuffer, packet->bufferSize);

                if (packet->sendAttempts > 0) packet->sendAttempts--;
                packet->sendTimestamp = micros();

                nodeData_[packet->receivingNodeID].waitingOnPacket = packet;

            } else {

                if (packet->sendTimestamp + packet->sendInterval < micros()) {
                    
                    if (packet->sendAttempts == 0) {

                        sendPacketsACK_.pop_back();
                        nodeData_[packet->receivingNodeID].waitingOnPacket = nullptr;

                    } else {

                        packet->sendAttempts--;
                        packet->sendTimestamp = micros();

                        dataLink_->sendBuffer(packet->dataBuffer, packet->bufferSize);

                    }
                    
                }

            }

        }

    }



    if (dataLink_->available()) {

        uint8_t packet[dataLink_->available()];

        uint32_t bytes = dataLink_->receiveBuffer(packet, sizeof(packet));

        if (bytes > 0) { //Only continue if receive worked.

            ReceivedPayloadData message;
            bool ackRequested;

            if (decodeMessageFromBuffer(&message, &ackRequested, packet, sizeof(packet))) {

                if (message.messageData.receiverID == selfID_ || message.messageData.receiverID == eKraftPacketNodeID_t::eKraftPacketNodeID_broadcast) { //Make sure packet is for us.

                    switch (message.messageData.payloadID) {
                    case eKraftMessageType_t::eKraftMessageType_Ack_ID:
                        
                        for (uint32_t i = 0; i < sendPackets_.available(); i++) {

                            if (sendPackets_.peekPointer() == nodeData_[message.messageData.transmitterID].waitingOnPacket) {
                                nodeData_[message.messageData.transmitterID].waitingOnPacket = nullptr;
                                sendPackets_.remove(i);
                                break;
                            }

                        }  
                        break;

                    case eKraftMessageType_t::eKraftMessageType_Heartbeat_ID:

                        break;

                    case eKraftMessageType_t::eKraftMessageType_RadioSettings_ID:

                        break;

                    default:

                        receivedPackets_.push_front(message);
                        break;

                    } 

                }

            }

        }

    }


    

}