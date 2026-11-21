# dp Project, Part 1: Interpretation Questions

**Name: Dilara Aldemir**
**Drexel ID: da866**
**Parameters** (paste the output of `./params.sh <your-id>`):

```
DREXEL_ID=da866
DATA_DROP=23
ACK_DROP=38
SIZE_KB=96
DELAY=30
```


## Ground rules

- Answer every question in this file, below the question.
- Base every answer on **your** `evidence/` files. When a question asks for evidence, quote the exact log lines (copy them, keep the timestamps) and name the file they came from.
- An answer that does not match your evidence, or that could have been written without running your code, will not receive credit, however well written.
- Short and precise beats long and general. Most answers need a few sentences plus the quoted lines.

Useful commands:

```bash
grep 'pkt=17 ' evidence/merged-p-data.txt          # one packet's story (note the space)
grep -E 'TIMEOUT|DROP|DUP' evidence/merged-p-ack.txt
grep -v NET evidence/client-p-delay.txt            # status lines only
```

---

## P. Prediction (5 points)

**Complete and commit this section BEFORE you run `make evidence`.** Use only your parameters and your understanding of the protocol. The TA checks that this commit comes before the commit that adds `evidence/`. You are graded on your reasoning, not on whether you turn out to be right.
*My prediction:
1. In your `p-data` run the client loses DATA packet `DATA_DROP` once. Describe, step by step, what the client and server will do from that moment until packet `DATA_DROP + 1` is sent. Will the server log a `DUP`? Why or why not?
The client sends DATA packet 23, but the packet is dropped in transit. Since the client fails to get the ACK from the server within 250 ms, it sends packet 23 again with the same packet number. This is the first time that the packet reaches the server, which processes it and responds with an ACK. The client then sends packet 24. The server does not log a duplicate because packet 23 did not reach it before.

2. In your `p-ack` run the server loses the ACK for packet `ACK_DROP` once. Same question: step by step, and will the server log a `DUP`?
Packet 38 is transmitted by the client and received and processed by the server. An ACK is sent by the server, but the ACK is dropped on the way by the network. The client will time out and resend packet 38. As packet 38 has been processed by the server earlier, the server will recognize the duplicate and respond with an ACK without processing the payload for the second time. Packet 39 will then be transmitted by the client.

3. From the client's point of view (only what the client can observe), will these two runs look different? Why?
The two transmissions would be virtually identical from the perspective of the client. In both instances, the client would transmit a DATA packet, fail to receive its ACK within the allotted time, resend the DATA packet, and finally receive the ACK. The client would not be able to differentiate which was lost.

4. Your `p-delay` run sends `SIZE_KB` KB with a random delay of 0 to `DELAY` ms on every packet in each direction. Estimate how long the transfer will take. Show your arithmetic.
Transfer size: 96 KB, i.e., 96 x 1024 = 98,304 bytes.
With max. payload per DATA packet as 480 bytes:

98,304 ÷ 480 = 204.8, thus, 205 DATA packets would be required.

Average random delay in each direction would be 30 ÷ 2 = 15 ms, so that each DATA/ACK transaction requires approximately 15 + 15 = 30 ms.

205 x 30 ms = 6,150 ms = 6.15 seconds.
*
---

## 1. A timeout is not proof of loss (8 points)

Use `evidence/merged-p-data.txt` and `evidence/merged-p-ack.txt`.

a. Quote the lines around the TIMEOUT in each run (from the drop through the next ACK the client receives).
Quote the `DUP` line and the lines just before and after it. Which packet nu$
In p-data, DATA packet 23 was dropped. The client timed out and retransmitted i$

3.544 ms  dp-client: NET: OUT DATA pkt=23 bytes=508 DROP (forced)
254.597 ms  dp-client: NET: TIMEOUT DATA pkt=23 retransmitting
254.702 ms  dp-client: NET: OUT DATA pkt=23 bytes=508
254.903 ms  dp-server: NET: IN DATA pkt=23 bytes=508
254.967 ms  dp-server: NET: OUT ACK pkt=23 bytes=28
255.076 ms  dp-client: NET: IN ACK pkt=23 bytes=28
In p-ack, the DATA packet reached the server, but its ACK was dropped:

5.657 ms  dp-client: NET: OUT DATA pkt=38 bytes=508
5.716 ms  dp-server: NET: IN DATA pkt=38 bytes=508
5.725 ms  dp-server: NET: OUT ACK pkt=38 bytes=28 DROP (forced)
256.715 ms  dp-client: NET: TIMEOUT DATA pkt=38 retransmitting
256.820 ms  dp-client: NET: OUT DATA pkt=38 bytes=508
257.010 ms  dp-server: NET: IN DATA pkt=38 bytes=508
257.064 ms  dp-server: NET: DUP DATA pkt=38 already delivered, re-ACK
257.090 ms  dp-server: NET: OUT ACK pkt=38 bytes=28
257.205 ms  dp-client: NET: IN ACK pkt=38 bytes=28

b. In one run the DATA was lost; in the other the DATA arrived and the ACK was lost. List exactly what the client's **transport code** (`send_data_packet()`) observed in each case. Are they different?

   The client's `network:` statistics line *does* differ between `client-p-data.txt` and `client-p-ack.txt`. Quote both lines, explain why they differ, and explain why `send_data_packet()` cannot use that information.
In each scenario, the send_data_packet() function noticed that it sent the DATA$

The network statistics are different:

client-p-data.txt: dp-client: network: attempted=208 dropped=1 delivered=207 de$

client-p-ack.txt: dp-client: network: attempted=208 dropped=0 delivered=208 del$

While the first run had one dropped outgoing DATA packet, the second run did no$

c. Give one more cause of a timeout that involves no loss at all, and point to a TIMEOUT in `evidence/merged-slowack.txt` that shows it. Explain why the client's response is the same in all three cases.
Delaying too long without packet loss is another reason. The reason in the merg$

0.276 ms  dp-server: NET: OUT CONN_ACK pkt=0 bytes=28 delay=333ms
251.209 ms  dp-client: NET: TIMEOUT CONNECT pkt=0 retransmitting

It came after the 250 ms timeout of the client. Consequently, the client retran$

*Answer:*

---

## 2. Duplicate detection (8 points)

Use `evidence/merged-p-ack.txt`.

a. Quote the `DUP` line and the lines just before and after it. Which packet number is it?
In evidence/merged-p-ack.txt, the server receives DATA packet 38 and sends an ACK that is dropped:

5.716 ms  dp-server: NET: IN DATA pkt=38 bytes=508
5.725 ms  dp-server: NET: OUT ACK pkt=38 bytes=28 DROP (forced)

The client times out and retransmits packet 38. The server identifies it as a duplicate and sends another ACK:

256.715 ms dp-client: NET: TIMEOUT DATA pkt=38 retransmitting
256.820 ms dp-client: NET: OUT DATA pkt=38 bytes=508
257.010 ms dp-server: NET: IN DATA pkt=38 bytes=508
257.064 ms dp-server: NET: DUP DATA pkt=38 already delivered, re-ACK
257.090 ms dp-server: NET: OUT ACK pkt=38 bytes=28
257.205 ms dp-client: NET: IN ACK pkt=38 bytes=28
257.257 ms dp-client: NET: OUT DATA pkt=39 bytes=508

The duplicate packet is packet 38.

b. Explain, step by step, how the server knew this packet was a duplicate. Name the variable in your `dp-proto.c` that it compared against.
Packet 38 is identified as a duplicate since its packet number is smaller than the value stored in recv_next_packet. Packet 38 has already been delivered to the application layer, so there is no need to deliver its payload again. Therefore, the server will send an additional ACK to packet 38.

It is necessary for the server to ACK the duplicate since the initial ACK was lost. Otherwise, the client would continue timing out and retransmitting packet 38.

c. The server sent an ACK for the duplicate. What would happen to the client if it did not? What would happen to the received message if the server copied the duplicate's payload instead of discarding it? Use your actual `SIZE_KB` and `ACK_DROP` to say exactly which bytes would end up wrong.
The ACK of the duplicate must be done due to the possibility that the previous ACK was dropped. The second ACK helps the client cease from retransmitting packet 38. In case the ACK for the duplicate was not done, then the client would keep on timing out and retransmitting.

In case the duplicate is added to the application buffer, it will overwrite the next 480 bytes since they are not going to be ignored. Given that packet 38 has been delivered, the receive position will be at 18,240. This means that duplicating packet 38 at this point will overwrite byte positions 18,240 – 18,719 that hold the data for packet 39.
*Answer:*

---

## 3. Packet number vs message offset (7 points)

Use `evidence/merged-p-data.txt`.

a. Your lost packet `DATA_DROP` was retransmitted. What were its packet number and message offset on the original transmission and on the retransmission? (The offset is not printed in the log; compute it from the packet number and explain how.)
Packet 23 is seen to be first transmitted at 3.544 ms and dropped. It is retransmitted by the client at 254.702 ms, as follows:

3.544 ms   dp-client: NET: OUT DATA pkt=23 bytes=508 DROP (forced)
254.597 ms dp-client: NET: TIMEOUT DATA pkt=23 retransmitting
254.702 ms dp-client: NET: OUT DATA pkt=23 bytes=508
254.903 ms dp-server: NET: IN DATA pkt=23 bytes=508
254.967 ms dp-server: NET: OUT ACK pkt=23 bytes=28
255.076 ms dp-client: NET: IN ACK pkt=23 bytes=28

The payload carried in each DATA packet is 480 bytes since the 28 bytes of the header are not included in the payload. For packet 23, the message offset is thus (23 - 1) × 480 = 10,560. Hence, both packets carry bytes 10,560-11,039 of the message (offsets are zero-based).

b. Packet number and message offset both increase by a fixed step here. Explain why they are still two different ideas: what does each one identify, and who uses it? TCP uses a single byte sequence number for both jobs. Why can TCP get away with that?
The packet number is an identifier for a specific packet in the transmission process, and it helps with acknowledgment and retransmission detection. The message offset is an identifier indicating where the payload of a packet should be placed within an application message. They differ from each other in that the former keeps track of packets, and the latter keeps track of data.

TCP utilizes byte sequence numbers instead of packet numbers and message offsets.

c. Suppose the protocol later allowed a retransmission to carry a new packet number (QUIC does this). Which field would the receiver then have to use to detect duplicates, and why?
In case where new packet numbers were utilized for retransmission as in the QUIC protocol, the server would not be able to identify duplicates based on the packet number. Instead, it would have to rely on the message or stream offset along with the previously received byte range. This was due to the fact that the retransmitted packet would have a different packet number while carrying previously received bytes.

*Answer:*

---

## 4. FIN vs message length (5 points)

Use `evidence/merged-p-data.txt`.

a. Quote the last DATA packet of your transfer. How many payload bytes did it carry, and how do you know from the log line? Check your answer against `SIZE_KB`.
The last DATA packet in merged-p-data.txt is packet number 205:

276.738 ms dp-client: NET: OUT DATA pkt=205 bytes=412

In the log, there are 412 bytes total. As the header is 28 bytes long, then the size of the data is 384 bytes. It is quite understandable as the message is 96 KB (98,304 bytes), and all the previous 204 packets have 480 bytes each.

b. Every DATA packet already carries `message_length`. What does FIN tell the receiver that `message_length` does not? Think about Part 2, where one connection will carry several streams that end at different times.
The FIN flag indicates that this is the last DATA segment and that the stream is over. The field message_length provides information to the receiver about the total number of bytes in the message. They are different since message length represents the amount of data contained in the message, whereas FIN indicates that there is no further data that will come after in that stream. In Part 2, there are several streams, which may end at different times, hence, the usefulness of FIN flag.

*Answer:*

---

## 5. Stop-and-wait performance (10 points)

Use `evidence/client-p-delay.txt` and `evidence/merged-p-delay.txt`.

a. Report the measured transfer time and the number of DATA packets from the client's `sent` line. Compare with your Prediction.
In the file evidence/client-p-delay.txt, the client sends the following:

dp-client: sent 98304 bytes in 205 DATA packets, 7148.4 ms

The total time taken for transmission was 7,148.4 ms, which equals 7.15 seconds. This means I predicted one second slower than my estimation, which was 6.15 seconds.

b. Build a simple model: each DATA packet waits for one round trip, and each direction adds on average half of `DELAY` ms. What does the model predict? By how much (percent) does the measurement differ?
Network maximum delay = 30 ms in either direction.
Since we assume that delays have uniform distribution,
average delay = 15 ms in either direction.
Therefore, average round-trip time = 30 ms.

Since there were 205 DATA packets and stop-and-wait protocol needs one round-trip time for each packet,
205 x 30 ms = 6,150 ms = 6.15 seconds

The actual time taken was 16.2% more than the estimated value.

c. Explain the difference. Look at a few consecutive packets in the merged log: how long does one DATA/ACK exchange actually take, compared with the two `delay=` values printed for it?
From evidence/merged-p-delay.txt, the first DATA/ACK pair is:

55.353 ms  dp-client: NET: OUT DATA pkt=1 bytes=508 delay=29ms
86.748 ms  dp-server: NET: IN DATA pkt=1 bytes=508
86.819 ms  dp-server: NET: OUT ACK pkt=1 bytes=28 delay=17ms
104.276 ms dp-client: NET: IN ACK pkt=1 bytes=28

The sum of the two simulated delays is 46 ms (29 + 17). But the actual time taken from transmitting the DATA to getting its ACK was 48.923 ms. This additional time arises due to overheads of processing and scheduling. These overheads add up over multiple packets, thus resulting in the observed difference.

d. Your `clean` test sent 1 MiB in a few seconds on localhost, where the round trip is a fraction of a millisecond. If the round-trip time were 100 ms, how long would stop-and-wait take for 1 MiB? What change to the protocol would fix this? (Part 2 does it.)
A 1 MiB message comprises 1,048,576 bytes of data. If each DATA frame contains 480 bytes, then it will require 2,185 DATA frames. Assuming that the round-trip delay is 100 ms, then using stop-and-wait transfer mode will take:

2,185 x 100 ms = 218,500 ms = 218.5 s

Which is roughly 3 minutes and 39 seconds. However, to enhance the efficiency of the protocol, a sliding window mechanism can be employed to send multiple DATA frames before the ACK of the previous DATA frames are received.

*Answer:*

---

## 6. Socket timeouts and deadlines (6 points)

a. In `merged-p-data.txt`, how long after the drop did the TIMEOUT fire? Where does that number come from in the code?
In evidence/merged-p-data.txt, the packet 23 has been discarded at 3.544 ms, and the client has timed out at 254.597 ms.

254.597 – 3.544 = 251.053 ms

This means that the client has waited for about 251 ms, which is close to the 250 ms timeout specified in dp-proto.h:

#define DP_DEFAULT_TIMEOUT_MS 250

The small discrepancy is caused by overhead.

b. `SO_RCVTIMEO` makes `recvfrom()` give up after a set time. Why would a network program that waits forever for a reply be a poor design? Give one example outside this assignment and what the program should do when the timeout fires.
Infinite waiting is bad design since a missing packet or an ACK could make the application wait forever. For instance, an infinite wait may occur when the file being transferred gets stuck forever due to a crash at the receiving end or even when the network connection fails.

c. The provided `recv_until()` waits against a fixed deadline rather than calling `recvfrom()` with a fresh `SO_RCVTIMEO` each time. Describe a situation in this protocol where the fresh-timeout approach would wait much longer than 250 ms.
The fixed deadline imposes a maximum limit on the waiting time to 250 ms. Using SO_RCVTIMEO, the recvfrom() call will be able to set a new timeout value each time. The arrival of extraneous packets at the time of expiration of the timeout value may cause the waiting process to continue well beyond the 250 ms mark.
*Answer:*

---

## 7. Closing reliably (6 points)

Use `evidence/merged-loss50.txt` (nearly the same for everyone, since the seeds are fixed).

a. Find a CONN_ACK that was dropped. What did the client do, and which of your functions answered the client's next CONNECT? Quote the lines.
In evidence/merged-loss50.txt, both the initial CONNECT packet and the first CONN_ACK were lost. The client experienced a timeout and retransmitted the lost CONNECT packet 0. The second instance of the CONNECT packet was received by dp_accept() on the server side, which generated the CONN_ACK.

0.000 ms    dp-client: NET: OUT CONNECT pkt=0 bytes=28 DROP
251.046 ms  dp-client: NET: TIMEOUT CONNECT pkt=0 retransmitting
251.099 ms  dp-client: NET: OUT CONNECT pkt=0 bytes=28
251.363 ms  dp-server: NET: IN CONNECT pkt=0 bytes=28
251.419 ms  dp-server: NET: OUT CONN_ACK pkt=0 bytes=28 DROP
502.351 ms  dp-client: NET: TIMEOUT CONNECT pkt=0 retransmitting
502.429 ms  dp-client: NET: OUT CONNECT pkt=0 bytes=28
502.653 ms  dp-server: NET: IN CONNECT pkt=0 bytes=28
502.699 ms  dp-server: NET: OUT CONN_ACK pkt=0 bytes=28
502.816 ms  dp-client: NET: IN CONN_ACK pkt=0 bytes=28

b. Find a CLOSE_ACK that was dropped. Quote the lines showing the client's retransmitted CLOSE being answered.
In my merged-loss50.txt file, I don’t have a case of the CLOSE_ACK being lost. However, the initial CLOSE packet (packet 19) gets dropped. Client times out and retries sending the packet, which is received by the server and responds back with CLOSE_ACK:

16839.365 ms dp-client: NET: OUT CLOSE pkt=19 bytes=28 DROP
17090.425 ms dp-client: NET: TIMEOUT CLOSE pkt=19 retransmitting
17090.498 ms dp-client: NET: OUT CLOSE pkt=19 bytes=28
17090.707 ms dp-server: NET: IN CLOSE pkt=19 bytes=28
17090.758 ms dp-server: NET: OUT CLOSE_ACK pkt=19 bytes=28
17090.861 ms dp-client: NET: IN CLOSE_ACK pkt=19 bytes=28

This example shows how client retransmits the CLOSE packet if it doesn’t get a confirmation for it.

c. The server keeps answering for about two seconds after the client goes quiet. What would go wrong if it exited immediately after sending its first CLOSE_ACK? Can the client ever be completely sure the server received its final message? (This is a version of the Two Generals problem.) TCP faces the same problem when closing; which TCP state plays the role of dp's linger?
It takes 2 seconds or so for the server to close in order to be able to handle retransmissions of the CLOSE messages in case the previous CLOSE_ACK message had been lost. The server would not wait after sending the initial CLOSE_ACK message; then, in case of retransmission of the CLOSE message, it could happen that it already closed and the client will wait and timeout without getting another CLOSE_ACK message.

The client has no means to be absolutely sure that the last message reached the server. The server received CLOSE_ACK but the client does not know whether it received it because it is not absolutely sure whether another acknowledgment arrived back. It has something in common with the Two Generals Problem: unreliable communication between two parties means that it cannot be guaranteed that both parties got the last message.

There is a similar concept in TCP called TIME_WAIT.
*Answer:*
