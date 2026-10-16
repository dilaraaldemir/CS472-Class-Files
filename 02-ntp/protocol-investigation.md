INVESTIGATION 1: Why does NTP need four timestamps?

*Implementation Context*

Four different timestamps are used by the NTP client to determine clock offset and network delay:

-T1 = client transmit time
-T2 = server receive time
-T3 = server transmit time
-T4 = client receive time

These values are used in calculate_ntp_offset():

t1 = ntp_time_to_double(&request->xmit_time);
t2 = ntp_time_to_double(&response->recv_time);
t3 = ntp_time_to_double(&response->xmit_time);
t4 = ntp_time_to_double(recv_time);

result->delay = (t4 - t1) - (t3 - t2);
result->offset = ((t2 - t1) + (t3 - t4)) / 2.0;

Before examining this, it seemed that it would have been easier for me to compare my local time with the time on the server. I needed to find out the reason why the protocol requires four timestamps.

*Investigation Journey*

uestion 1: Why can't I just compare my local time with the server's transmit time?
Initially, it appeared to be the easiest approach since the client is aware of the current time while the server sends the current time, so I would calculate:
Server time – Local time
The issue here is that the two clocks are being viewed at different times. There is time taken by the packet to travel from the client to the server and then back. Consequently, the time stamp received by the client corresponds to the past.
The clock synchronization in my implementation should take into consideration travel time of the packet rather than considering the time stamps as taken at the same time.
Aha moment: I understood that clock synchronization is not only the comparison of two clocks but also a measurement process due to the fact that there is a network distance between the two clocks.

Question 2: Why do I need T1/T4 and T2/T3?
T1 and T4 are determined by the client:
-T1: at the point when the request is sent
-T4: at the point when the response comes to the client
T2 and T3 are determined by the server:
-T2: at the point when the request arrives
-T3: at the point when the response is sent
It provides me with the timestamps for the whole request/response process.
The difference between T4 and T1 shows the round-trip time. Nevertheless, this value contains the time needed by the server to process the request. It can be calculated from (T3 – T2).
This is why in my code I calculate:
result->delay = (t4 - t1) - (t3 - t2);

Question 3: Why does the offset formula have two different calculations?
My code performs:
result->offset = ((t2 - t1) + (t3 - t4)) / 2.0;
In the first calculation, (t2 - t1), the time of reception at the server is compared to the time of transmission at the client.
In the second calculation, (t3 - t4), the time of transmission at the server is compared to the time of reception at the client.
Rather than depending on one single direction of transmission, both directions are used in this calculation and averaged.
This makes more sense since the client does not know the exact transmission time for each network link.

Question 4: What would be the case if the delay between the client and server is different than the delay between the server and client?
This was the reason why the four timestamps became very significant.
The calculation presumes that the network delay is symmetric. In the case when the request takes more or less time than the response, there may be an error in the estimated offset.
In case:
client to server = 10 ms
server to client = 50 ms
The client would not be able to identify how much of the difference is caused by the clock offset and different network delays.
The four timestamps are unable to solve this issue entirely; however, they provide enough information for NTP to do this better than using two timestamps.

Question 5: Why measure delay and offset independently?
In my program, I have these two measures:
Round Trip Delay
Time Offset
When I measured pool.ntp.org, the client returned:
Round Trip Delay: 0.050321 seconds
Time Offset: 0.140772 seconds
When I measured time.google.com, the client returned:
Round Trip Delay: 0.032072 seconds
Time Offset: 0.137856 seconds
They represent different parameters.
Delay represents the network round-trip delay, whereas offset represents the difference between the client’s clock and the server’s clock.
It also demonstrated to me that even with a small network delay, this doesn’t mean that the clocks are synchronized. My Google test had a smaller delay compared to the pool.ntp.org, however, the clock offset was about 138 ms.

Question 6: What is wrong with using only two timestamps?
A simpler method would involve having just one timestamp from the client and another from the server. This way, however, it will combine the clock offset and the network delay.
With four timestamps, it becomes possible to distinguish these two factors:
T1 - request departs the client
T2 - request arrives at the server
T3 - response departs the server
T4 - response arrives at the client
The protocol can get information regarding both directions of communication.

Design Rationale
Four-timestamp scheme is the compromise between complexity of protocol and its precision.
Simpler protocol will use fewer timestamps, but it will also have less data about how much time the request and the response spent in the network. NTP should estimate clock offset while taking into account network delay, that is why the extra timestamps are important.
In my program implementation I illustrate this by the following calculations:
result->delay = (t4 – t1) – (t3 – t2);
result->offset = ((t2 – t1) + (t3 – t4)) / 2.0;
If there were no T2 and T3 timestamps, I could neither subtract the processing time on the server nor compute the offset as above

Implementation Insight
Prior to this investigation, my view of NTP had been something like “ask the server for the time”. However, after analyzing the four timestamps and experimenting with the client, I found out that it’s not hard to get the server time; the hard thing is to estimate what the time on the server was with respect to my time when the data was passing through the network.
This was illustrated by the test results as the different servers showed different delays, yet virtually the same offset.

--------------------------------------------------------------------------------------------

INVESTIGATION 2: Why does NTP use network byte order conversions?

Implementation Context

NTP packets use multi-byte integers to store values like timestamp, root delay, root dispersion, and reference identifier.
In my implementation, I convert these fields prior to sending the packet:

void ntp_ts_to_net(ntp_timestamp_t* timestamp){
    timestamp->seconds = htonl(timestamp->seconds);
    timestamp->fraction = htonl(timestamp->fraction);
}

and 

void ntp_to_net(ntp_packet_t* packet){
    packet->root_delay = htonl(packet->root_delay);
    packet->root_dispersion = htonl(packet->root_dispersion);
    packet->reference_id = htonl(packet->reference_id);

    ntp_ts_to_net(&packet->ref_time);
    ntp_ts_to_net(&packet->orig_time);
    ntp_ts_to_net(&packet->recv_time);
    ntp_ts_to_net(&packet->xmit_time);
}

After getting a packet, my program performs the following conversion:

void ntp_ts_to_host(ntp_timestamp_t* timestamp){
    timestamp->seconds = ntohl(timestamp->seconds);
    timestamp->fraction = ntohl(timestamp->fraction);
}

and ntp_to_host() works on this principle for the entire packet.
I was curious about the reason why these conversions had to be performed rather than transmitting the C structure directly.

Investigation Journey

Question 1: Why can't I just send the ntp_packet_t struct directly?

At first, it may seem reasonable as the packet is already defined by using the C struct.
The problem, however, lies in the fact that multi-byte integer might be saved in a different way on different computer architectures.
The protocol should not assume that all machines save the bytes of an integer in the same order.
NTP specifies a certain order of bytes to be used for values transmitted via the network.
Thus, what goes into the memory in my C program and what goes into the network do not have to be the same.
Aha moment: The struct is a good way of representing data in my C program, but not necessarily universally.

Question 2: What exactly is htonl()?

htonl() stands for "host to network long."
Given a 32-bit integer, it transforms its byte order on the host machine to network byte order.
In my code, I use it for all 32-bit fields in the packet structure:
packet->root_delay = htonl(packet->root_delay);
packet->root_dispersion = htonl(packet->root_dispersion);
packet->reference_id = htonl(packet->reference_id);
The timestamp transform operates similarly on two 32-bit parts of the timestamp:
timestamp->seconds = htonl(timestamp->seconds);
timestamp->fraction = htonl(timestamp->fraction);

Question 3: Why is there a need for conversion before sendto()?

The packet will eventually be sent out as raw bytes.
In case I created the packet from the C integer values without converting, the machine receiving the message would have to decode the bytes correctly.
This conversion ensures predictability in the packet structure before it gets to the socket.
Thus, the general order of operations in my code looks like:
C values
   ↓
ntp_to_net()
   ↓
network byte order
   ↓
sendto()
   ↓
network
The conversion is done before sending because this is where the information goes from my code representation to the protocol byte order.

Question 4: Why perform a reverse conversion after receiving?

Question 4: Why perform a reverse conversion after receiving?
The reverse conversion takes place after the call to recvfrom().
The data that have been received on the network is in network byte order while the rest of my C program assumes integer values.
Therefore I use:
ntp_to_host(&packet);
which eventually calls:
ntohl()
for the 32-bit values.
The whole process is:
network bytes
   ↓
recvfrom()
   ↓
ntp_to_host()
   ↓
host-order C integers
   ↓
calculations and printing
This is especially critical for my timestamp calculations, where functions like ntp_time_to_double() assume that the timestamp fields have valid integer values.

Question 5: What if I forgot to use ntohl()?

If I receive the packet but don’t convert the values, the bytes will be in the wrong order.
Values such as the following will be affected:
•Timestamp seconds
•Timestamp fractions
•Root delay
•Root dispersion
•Reference ID

For example, calculate_ntp_offset() depends on:
t1 = ntp_time_to_double(&request->xmit_time);
t2 = ntp_time_to_double(&response->recv_time);
t3 = ntp_time_to_double(&response->xmit_time);
t4 = ntp_time_to_double(recv_time);
In the case where the timestamp fields are not properly converted, these computations will be performed using wrong values.
Therefore, byte order conversion is not only related to network issues; it influences directly the correctness of the time synchronization computation.

Question 6: So what’s the difference when both machines know how to work with integers?

This question helped link up the implementation with the protocol concept.
These two machines don’t have to use the same representation inside. What they have to do is to agree on the representation which will be used during communication.
The protocol specifies the common representation.
This works just like specifying a language. Each machine is free to choose its own way of representing data, but the message should follow the agreed-upon rules.
With network byte order, the NTP packet can have a defined binary representation regardless of the host machine’s byte order.


Design Rationale
Network byte order is required since there is a need for a standardized way to represent multiple bytes in networking protocols.
The NTP packet has a lot of 32-bit quantities in it, hence just transmitting the host-memory representation of the C structure would make the protocol machine dependent.
By using htonl() before sending and ntohl() after receiving, we can have a well-defined line between:
Host representation
        ↕
Network representation

Implementation Insight

Now after understanding this more thoroughly, I had initially assumed that sendto() would send the values of my C structure. However, I now know that the socket passes bytes and not C variables.
The critical difference here is that:
C structure = How the data is represented in my code
NTP packet = How the data is represented in the protocol on the network
The conversion functions that do this are ntp_to_net() and ntp_to_host().
This also made sense of why this assignment focuses so much on binary protocol PDUs and network byte order.
