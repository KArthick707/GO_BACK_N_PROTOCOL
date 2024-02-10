/*
 * Server.cc
 *
 *  Created on: 20-Jan-2024
 *      Author: karthick
 */
#include<stdio.h>
#include<string.h>
#include<omnetpp.h>
#include<math.h>

using namespace omnetpp;

class Server: public cSimpleModule
{
private:
   int data_rate =0;
    int counter = 0;
    //Counter to receive the packets and send the acknowledgement when it is equal to the window size
    int seq_count = 0;
    //Variable to keep track of the latest sequence number received successfully
    int seq_count_copy = 0;
    //Keep a copy of the latest sequence number received successfully, to check if the packet is dropped later or not
    int window = 0;
    //Variable to store the window size and send it to toc
    int init_msg_flag = 0;
    //Variable to check if the message is an initial message from tic
    int lost_pkt_flag = 0;
    //Variable to check if there is a packet loss
    int multiple_lost_pkt_flag = 1;
    //Variable to check if there are multiple packet loss then send the seq_no one less than the initial packet loss
    int repeat_seq_flag = 0;
protected:
    void handleMessage(cMessage *msg) override;
    void generateMessage();
    void receiverMessage(cMessage *msg);
    void senderMessage(cMessage *msg);
    //void processFrames();

    //cQueue frameQueue;
};

Define_Module(Server);

void Server:: handleMessage(cMessage *msg) {
//The method decides whether to simulate message loss (%probability) or process the received message
    if (uniform(0, 1) < 0.1) {
        EV << "Losing message\n";
        bubble("message lost");

        lost_pkt_flag = 1;//If a message is lost it sets flags and generates a new message
        generateMessage();
        delete msg;

        //If a message is received, it calls the receivedMessage method and generates a new message
    } else {
        receiverMessage(msg);
        generateMessage();
    }
   // cPacket *frame = check_and_cast<cPacket *>(msg);
        //frameQueue.insert(frame);

       // processFrames();
}

//void Server::processFrames() {
    // Process frames from the queue
    //while (!frameQueue.isEmpty()) {
           //cMessage *msg = check_and_cast<cMessage *>(frameQueue.pop());
        // Process the frame...
   // }
//}

void Server:: generateMessage()
{
    char msgname[25];
    cMessage *msg;
    // This block is executed if init_msg_flag is not set (i.e., init_msg_flag is 0)
    // It generates and sends an initial control message indicating the window size
    if(!init_msg_flag) {
        strcpy(msgname, "Sending_Win_Size"); // Set the message name to "Window_size_is_5"
        init_msg_flag = 1; // Set init_msg_flag to 1 to indicate that the initial message has been sent

        msg = new cMessage(msgname);  // Create a new cMessage with the specified name
        msg->addPar("ack_no");  // Add parameters "ack_no" and "win_size" to the message
        msg->addPar("win_size");
        // Set the "ack_no" parameter to the current sequence count value
        msg->par("ack_no").setLongValue(seq_count);
        // Set the "win_size" parameter to the window size value
        msg->par("win_size").setLongValue(window);
        // Send the message to the "out" gate
        send(msg, "out");
    }


    else {

        // This block is executed when init_msg_flag is set, indicating that the initial message has been sent
            if (counter <= window) {
                // This condition checks if the counter is within the window size
                EV << "Inside_generating_ack_no_counter <= window\n";
                counter++;

                if ((counter < window) && (seq_count_copy == (seq_count + 1))) {
                    // This condition checks if the counter is less than the window size
                    // and the received sequence number is the next expected sequence number
                    EV << "receiving_the_packet_for_first_time\n";


                    if(seq_count_copy == 255) {
                        EV << " sequence_is_greater_than_255\n";
                        repeat_seq_flag = 1;
                        // This condition checks if the counter has reached the window size
                       // and the received sequence number is the next expected sequence number

                    }

                    seq_count = seq_count + 1;//it's incrementing the seq_count by 1
                    multiple_lost_pkt_flag = 1;//it assignes the flag to 1
                }
                else if ((counter == window) && (seq_count_copy == (seq_count + 1))) {
                        EV << "receiving_the_last_packet\n";

                        counter = 0;//This counter is reset to 0 because the frames has been recived
                        seq_count = seq_count + 1;
                        senderMessage(msg);
                        if (seq_count_copy == 255) {
                            EV<< "sequence_is_greater_than_255\n";
                            repeat_seq_flag = 1;
                        }

                    }
                    else if (lost_pkt_flag && multiple_lost_pkt_flag) {
                        // This condition checks if there is a lost packet and multiple losses are flagged
                        EV << "A_Packet_No=" <<seq_count_copy+1<<"is_Skipped/lost\n";
                        counter = 0;
                        lost_pkt_flag = 0;
                        multiple_lost_pkt_flag = 0;
                        senderMessage(msg);
                    } else {
                        // This block is executed when none of the above conditions are met
                        // It may indicate the loss of the first packet
                        counter = 0;
                EV<<"First_PacketLost";
                    }

                }

            }
}

void Server::senderMessage(cMessage *msg) {
    // Declare a character array to store the message name
    char msgname[20];
    // Set the message name to "Recive_Ready"
    strcpy(msgname, "R_Ready_from_SERVER");
    // Create a new cMessage with the specified name
    msg = new cMessage(msgname);
    // Add a parameter "ack_no" to the message
    msg->addPar("ack_no");
    // Set the value of the "ack_no" parameter to the current sequence count value
    msg->par("ack_no").setLongValue(seq_count);
    // Send the message to the "out" gate
    send(msg, "out");
}
void Server::receiverMessage(cMessage *msg) {
    // Check if the received message's "seq_no" parameter is 0
    if (msg->par("seq_no").longValue() == 0) {
        // Log that an initial message is received from the sender
        EV << "'Message_received_from CLIENT: " << msg->getName() << "\n";
        EV << "'Data_received SERVER: \n" << msg->par("seq_no").longValue() << "\n";
        // Set the window size to the specified value from the simulation parameters
        window = par("win_size");
        // Delete the received message
        delete msg;

    }


    else {
        // Log that a sequence message is received from the sender
        EV << "'Message_received_from CLIENT: " << msg->getName() << "\n";
        EV << "'sequence_received: \n" << msg->par("seq_no").longValue() << "\n";
        // Copy the received sequence number to the seq_count_copy variable
        seq_count_copy = msg->par("seq_no").longValue();
        // Log the value of the repeat_seq_flag variable
        EV << "'repeat_seq_flag: " << repeat_seq_flag << "\n";
        // Check if the received sequence number is 1 and repeat_seq_flag is set
        if (seq_count_copy == 1 && repeat_seq_flag) {
            // Reset repeat_seq_flag and seq_count if the conditions are met
            repeat_seq_flag = 0;
            seq_count = 0;
        }
        // Delete the received message
        delete msg;
}
}
