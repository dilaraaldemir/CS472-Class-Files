Part 1: Problem-Solution Mapping Table

| Problem                                       | Solution Proposed by Paper OR Why Not Addressed                                           | How We See This Today                                                         |
|-----------------------------------------------|-------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------|
| 1. Addressing different networks              | A common address system across different networks to be able to uniquely identify a host. | Today IP addresses allow devices to be identified across multiple networks.   |
| 2. Different packet sizes in between networks | Paper makes gateways into fragment packets when a packet is too large for the next network and then reassemble them at the destination host. | Today networks can still have different maximum packet sizes and IP fragmentation helps these. |
| 3. Reliability across several networks        | Paper uses acknowledgments, checksums, sequence numbers, and retransmission after timeouts to make communication reliable even when packets are lost. | Today reliable transport protocols like TCP use acknowledgments, sequencing, and retransmission to handle corrupted data. |
| 4. Flow control                               | Recipient control limit of data the sender can send by using the window size in acknowledgment messages. | Today flow control is used to prevent a fast sender from making the receiver overwhelmed. |
| 5. Routing between several networks           | Gateways connect different networks and route packets towards their destinations. | Routers still forward packets between networks based on destination information. |
| 6. Accounting and billing                     | Paper talk about accounting data that can be collected by gateways to monitor usage and billing between the networks. | Internet providers today can keep track of data usage for billing and other purposes. |

Part 2: AI-Assisted Investigation

A. 
Problem: Accounting and billing 

Scenario: Understanding what happens when I use cellular data to watch a Youtube video and how my phone carrier tracks the amount of data I use. I want to also learn how this connects to the accounting idea in the Cerf-Khan paper.

B.

Questions I have asked: 

How does my carrier know how much cellular data I use when watching YouTube?
What exactly is being counted when I use 500 MB of data?
Where is my data usage measured?
How does the carrier connect that usage to my account?
How is this similar to the accounting system described in the Cerf-Kahn paper?


C. 

Connection to 1974 paper

As discussed by Cerf and Kahn, it was possible to utilize gateway accounting for keeping record about the consumption of resources and usage of the network to facilitate billing in between different networks. The concept is still pretty valid in the modern scenario.

However, modern networks are a lot more complex. Modern day systems are used for tracking millions of subscribers, devices and a lot of traffic. It was impossible to guess the appearance of smartphones and streaming services back then.

D. 

Surprise Finding

At first, I believed that the only thing my mobile device does is calculate the amount of my data consumption and send this information to my service provider. Then I understood that my service provider could monitor traffic on its own network too. It surprises me to realize that such a seemingly easy task as viewing videos on YouTube includes so many different networks.


E.

AI Conversation Documentation

Me: How does my carrier know how much cellular data I use when watching YouTube?

When you watch YouTube on cellular data, your phone sends requests through your carrier's network and receives the video data back. The carrier's network can measure how much data is being transferred through your connection. The amount of data sent and received can then be recorded as your usage.

Me: What exactly is being counted when I use 500 MB of data?

It means about 500 MB of data was transferred through your cellular connection. For example, when YouTube sends video information to your phone, those packets add to your data usage. Other things, like uploading photos or apps downloading updates, can also use cellular data.

Me: Where is my data usage measured?

Your phone can keep track of its own cellular data usage, but your carrier also tracks usage within its network. The carrier's network has systems that monitor the traffic associated with your cellular connection.

Me: How does the carrier connect that usage to my account?

Your phone's cellular connection is associated with your subscriber/account information. When data travels through the carrier's network, the network knows which subscriber is using that connection. The recorded usage can therefore be associated with your account and used for your data plan or billing.

Me: How is this similar to the accounting system described in the Cerf-Kahn paper?

The main idea is the same: network usage is measured and recorded. Cerf and Kahn proposed that gateways could collect accounting information to track usage between networks and potentially support billing. Today, carriers do something similar, but on a much larger and more complicated scale.
The important connection: 1974 → gateways track network usage → today → network systems track individual users' data usage.


Part 3: Reflection

The problem that surprised me the most is accounting and billing because I didn’t know how many details the network needs to calculate the usage of data. After the AI research, I realized that my provider can monitor how much data was transmitted via its network and associate this with my account. At first, I assumed that all my provider does is monitor the data usage on my phone and report it. In reality, however, networks use a much more complicated system to monitor the usage of data, although the principle remains basically the same as it was suggested by Cerf and Kahn back in 1974.

