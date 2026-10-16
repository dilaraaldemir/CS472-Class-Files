*Time in Distributed Systems*

Initially, when working with the NTP client, I viewed the concept of time synchronization as retrieving the time from the server and comparing it with the local time on the computer. After implementing the four-timestamp algorithm, I became aware that the time at which the message was sent is not equal to the time of receipt in distributed systems.
My first question is: Why can't the client simply use the transmit timestamp of the server?
I became aware that the answer comes from the fact that there is some delay in receiving the response. If I use the timestamp from the server, I will actually treat the old timestamp as the current time on the computer.
My second question is: Why NTP requires four timestamps instead of two?
The use of four timestamps enables NTP to distinguish between the time needed to travel from the client to the server and the time needed for the reverse travel. In my solution, I utilize T1, T2, T3, and T4 timestamps in calculate_ntp_offset() method:

result->delay = (t4 - t1) - (t3 - t2);  
result->offset = ((t2 - t1) + (t3 - t4)) / 2.0;

I then asked myself another question: What will happen if the network delay is high or different for both directions?
This algorithm uses the assumption that the network delay is approximate to be symmetrical. It cannot calculate the exact delay for each direction based on one request, therefore the calculated offset is approximate as well.
Then I asked: Is the result different for each server?
I used three servers for my experiments. The offset for pool.ntp.org was around 140.77 ms, time.nist.gov – 137.19 ms, and time.google.com – 137.86 ms. They also had different round-trip delays.
One more question popped up: Why do we need time for distributed systems research, if computers can synchronize their clocks?
It turned out that it can be helpful, but it does not mean that physical clocks will be synchronized. There are still some delays, drifts and failures. Therefore there should be some other way to order events.
Then I wondered can I say what event happened earlier using a timestamp?
Logical clocks were developed for use in this area. Physical timestamps can give time-related information regarding an event, but they do not always permit causation to be established.
What I realized was that NTP was not ensuring that there is perfect time in distributed systems; rather, it was providing an estimate of physical time, and other distributed systems methods were taking care of the rest.

*Real-World Failure*

Incorrect time becomes one of the primary sources of the incorrect decision-making for a distributed system.
For instance, there could be distributed database or some other type of service that uses timestamps to determine which version of data is the newest. Let us imagine that two servers get some updates of the same information. In case one of the servers' clocks is several minutes ahead, then it might add a newer timestamp to an older update.

For instance:
Server A gets update X.
Server B gets update Y immediately after.
Clocks of Server B are off by several seconds.
System checks timestamps instead of causality.
System makes an incorrect assumption that X is newer than Y.
It shows how synchronized physical time is useful but still not enough.

My NTP client suffers from the same principle but on a smaller scale. During my tests, I have found out that offsets of my calculations were between 137-141 milliseconds. It means that my local clock was not synchronized with the servers I was contacting. But NTP algorithm was able to calculate the offset but not remove network uncertainty.
There will be similar problems even with a distributed system comprising of several machines. If each machine has to periodically synchronize its clock with NTP, there will still be some disparity between them.
This is precisely the reason why ordering systems cannot blindly rely on physical timestamp for ordering.

*Physical and Logical Time*

NTP addresses physical time through synchronization of the computer clocks with an external clock. NTP relies on timestamps and the four-timestamps algorithm to find the difference between the client clock and the server's clock. For example, in my test with time.google.com the offset was around 138 milliseconds.
However, physical clocks cannot be used to order events due to the existence of network delay and clock differences.
Logical clocks like Lamport clocks do not care about time, they care about ordering the events. When event A causes a message and event B occurs after receiving it, then logical clock will conclude that A occurred before B.
The major difference I learned from this is that:

NTP: What is the time of the event?
Logical clock: Which event occurred first?

*CAP and Eventual Consistency*

Communication in distributed systems happens through networks, and this means delays and faults prevent achieving perfect synchronization. My NTP tests gave the time between 32 and 61 milliseconds to achieve round-trip, illustrating that communication does not happen instantly.
Eventual consistency enables distributed systems to have temporarily inconsistent states as opposed to forcing all machines to be in a state of agreement. In the same manner, NTP uses clock synchronization on intervals instead of synchronizing the clocks permanently.
Network failures illustrate in CAP how network failures bring trade-offs between consistency and availability. In the event of a network partition, a distributed system cannot achieve perfect agreement without sacrificing availability.
The NTP client made it clear that distributed systems have to deal with network uncertainties as opposed to working under perfectly synchronized information among all machines.

*My NTP Client in Context*

My NTP client is a good example of a distributed systems problem in which there are multiple computers in need of a shared sense of time.
get_current_ntp_time() converts UNIX time into NTP time.
ntp_to_net() and ntp_to_host() do byte order conversion for the network.
calculate_ntp_offset() calculates clock offset and network delay using T1, T2, T3, and T4.

Here are my test results:

Server			Stratum			Round Trip Delay			Clock Offset
------------------------------------------------------------------------------------------------------------
pool.ntp.org		  2 			   50.321 ms				 +140.772 ms
time.nist.gov		  1		           60.689 ms				 +137.193 ms
time.google.com		  1			   32.072 ms				 +137.856 ms

Similar clock offsets but varying delays in the tests helped verify the calculations, and the varying delays proved why synchronization could never be absolutely accurate.
I think key point to take away is that NTP is never going to make a globally synchronized clock. It makes an estimation of physical time, whereas logical clocks and other distributed system methods manage ordering, causality, and consistency.
