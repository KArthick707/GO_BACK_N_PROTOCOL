import ned.IdealChannel;//we have set to IdealChannel, I don't why it is used.

//here we have implemented the Gates and Parameter for the client
simple Client
{
    parameters:
        @display("i=block/app2,,0");

    gates:
        input in;
        output out;
}
//here we have implemented the Gates and Parameter for the server 
simple Server
{
    parameters:
        @display("i=block/server,,0");
        int data_rate;
        int win_size;
        int seq_count;
        int N;
    gates:
        input in;
        output out;
}

//trying to implement the hub inbetween the client and server.
//question is should i try to implement the sliding window protocol for the hub???
//or should other protocol instead.!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
simple hub
{
    @display("i=device/hub,,0");
    gates:
        input in;
        output out;
        input in1;
        output out1;
}
//Here we are implementing the function Client and Server it apprenceand
network Go_Back_N
{
    @display("bgb=714,347");
    submodules:
        client: Client {
            @display("i=device/pc,white;p=85,153");
        }
        server: Server {
            @display("i=device/server;p=518,165");
        }
//Here is were connection are implemented
    connections:
        client.out --> IdealChannel --> server.in;
        server.out --> IdealChannel --> client.in;
}
