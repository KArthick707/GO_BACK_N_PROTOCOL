/*
 * Client.cc
 *
 *  Created on: 20-Jan-2024
 *      Author: karthick
 */
#include<stdio.h>
#include<string.h>
#include<omnetpp.h>
#include <cmath>

using namespace omnetpp;

class Client: public cSimpleModule
{
private:
    simtime_t timeout;
    // A variable to store simulation time
    cMessage *timeoutEvent;
    // A pointer to a message that will be used to trigger events related to timeouts
    int data_rate =0;
    // An integer variable to store the data rate (currently initialized to 3)
    int counter = 3;
    // An integer variable to keep track of some counter
    int init_msg_flag = 0;
    // An integer flag to indicate whether an initial message has been sent
    int seq_count = 0;
    // An integer variable to store a sequence count
    int win_size = 0;
    // An integer variable to store the window size
public:
    Client();
    virtual ~Client();
protected:
     void generateMessage();
  void senderMessage(cMessage *msg);
     void receiverMessage(cMessage *msg);
     void initialize() override;
     void handleMessage(cMessage *msg) override;
};

Define_Module(Client);

Client::Client()
{
    timeoutEvent = nullptr;
    // Initialize timeoutEvent pointer to nullptr
}

Client::~Client()
{
    cancelAndDelete(timeoutEvent);
    // Cancel and delete the timeout event when the object is destroyed
}

void Client::initialize() {
    timeout = 5.0;
    // Set the timeout value in seconds
    timeoutEvent = new cMessage("timeoutEvent");
    // Create a new timeout event message
    generateMessage();
    // Generate the initial message
    EV << "start timer\n";
    // Log a message indicating the timer has started
    scheduleAt(simTime() + timeout, timeoutEvent);
    // Schedule the timeout event to occur after the specified time
}

void Client::handleMessage(cMessage *msg) {
    if (msg == timeoutEvent) {
        // Check if the received message is the timeout event
        if (counter) {
            // Check if the counter is not zero
            counter--;
            // Decrease the counter
            EV << "Timeout expired, re-sending message\n";
            // Log a message indicating the timeout expired and re-sending the message
            bubble("retransmission_the_Frames_that_are_missing");
            // Display a bubble with the given text
            if (seq_count == 0) {
                init_msg_flag = 0;
                // Reset the init_msg_flag if seq_count is 0
                generateMessage();
                // Generate a new message
            } else {
                seq_count = seq_count - win_size;
                // Adjust seq_count by subtracting win_size
                EV << "seq at client=" << seq_count << "\n";
                // Log the current value of seq_count
                generateMessage();
                // Generate a new message
            }
            scheduleAt(simTime() + timeout, timeoutEvent);
            // Schedule the timeout event to occur after the specified time
        } else {
            EV << "No response from server, Exiting the program\n";
            // Log a message indicating no response from the server and exiting the program
        }
    } else {
        receiverMessage(msg);
        // Handle the received message using the receiverMessage function
        EV << "timer cancelled";
        // Log a message indicating the timer is cancelled
        cancelEvent(timeoutEvent);
        // Cancel the timeout event
        counter = 3;
        // Reset the counter to 3
        generateMessage();
        // Generate a new message
        scheduleAt(simTime() + timeout, timeoutEvent);
        // Schedule the timeout event to occur after the specified time
    }
}

void Client::generateMessage() {
    char msgname[40];

    int i = 0;

    if (!init_msg_flag) {
        // Check if the init_msg_flag is not set
        init_msg_flag = 1;
        // Set the init_msg_flag to 1
        strcpy(msgname, "Window_Request_FromClient");
        // Copy the message name "Window_Request_FromClient" to msgname
        cMessage *msg = new cMessage(msgname);
        // Create a new message with the specified name
        msg->addPar("seq_no");
        // Add the "seq_no" parameter to the message
        msg->par("seq_no").setLongValue(seq_count);
        // Set the value of the "seq_no" parameter to seq_count
        senderMessage(msg);
        // Send the message using the senderMessage function
    }
    else {

        strcpy(msgname, "Frames_from_client");
        // Copy the message name "Frames_from_client_Sent" to msgname
        for (; i < win_size; i++) {
            // Loop for win_size iterations
            cMessage *msg[win_size];
            // Create an array of win_size message pointers
            seq_count = seq_count + 1;
            // Increment seq_count
            msg[i] = new cMessage(msgname);
            // Create a new message with the specified name
            msg[i]->addPar("seq_no");
            // Add the "seq_no" parameter to the message
            msg[i]->par("seq_no").setLongValue(seq_count);
            // Set the value of the "seq_no" parameter to seq_count
            senderMessage(msg[i]);
            // Send the message using the senderMessage function
            if (seq_count >= 255) {
                seq_count = 0;
                // Reset seq_count to 0 if it exceeds 255
            }
        }
    }
    return;
}

void Client::senderMessage(cMessage *msg) {
    send(msg, "out");

}

void Client::receiverMessage(cMessage *msg) {
    if (msg->par("ack_no").longValue() == 0) {
        // Check if the "ack_no" parameter of the received message is 0

        EV << "Message received from SERVER: " << msg->getName() << "\n" << "ack_no = " << msg->par("ack_no").longValue() << "\n";
        // Log a message indicating the message is received, including the message name and "ack_no" parameter

        EV << "'window size: " << msg->par("win_size").longValue() << "\n";
        // Log the "window size" parameter of the received message

        win_size = msg->par("win_size").longValue();
        // Set the win_size variable to the value of the "win_size" parameter
        delete msg;

    } else
    {
        EV << "'Message received from: " << msg->getName() << "\n" << "ack_no =" << msg->par("ack_no").longValue() << "\n";
        // Log a message indicating the message is received, including the message name and "ack_no" parameter

        seq_count = msg->par("ack_no").longValue();
        // Set seq_count to the value of the "ack_no" parameter

        if (seq_count >= 255) {
            seq_count = 0;
            // Reset seq_count to 0 if it exceeds 255
        }
        delete msg;

}
}
