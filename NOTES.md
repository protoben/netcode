Network Coding Notes
====================

Technologies
------------

- Algebraic Linear Network Coding<sup>[3](/NOTES.md#koetter02)</sup>
- RLNC<sup>[5](/NOTES.md#ho06)</sup>

Existing protocols
------------------

- COPE<sup>[1](/NOTES.md#katti06)</sup>
  + Uses simple XORs
  + Opportunistic Listening
    * Each node listens promiscuously and keeps packets for time T (e.g. .5s)
    * *Reception reports* (piggybacked or gratuitous) relay which packets a node
      has stored
  + Opportunistic Coding
    * A router XORs messages m<sub>1</sub>, ... m<sub>n</sub> for recipients
      p<sub>1</sub>, ... p<sub>n</sub> when each p<sub>i</sub> has all messages
      except p<sub>i</sub> stored
    * All messages are decoded (and possibly re-encoded) before being forwarded
    * Packets are never delayed
  + Learning Neighbor State
    * When info from reception reports is unavailable, routers guess
    * Uses delivery probability (e.g., ETX)
    * Already present in some link-state routing protocols (?)
    * Incorrect guessing requires retransmission
  + Uses per-packet ACKs
    * 802.11 synchroous ACKs for uncoded packets
    * Asynchronous and bundled with Tx's or reception reports for coded packets
  + Header between MAC and IP
    1. # of packets XORed
    2. IDs (source\_addr/IP\_seq\_num hash) of packets XORed
    3. # of reception reports
    4. Tuples of {ip\_addr, last heard seq\_num, bitmap of previous 8 seq\_nums}
    5. # of ACKS
    6. Tuples of {source\_mac\_addr, neighbor\_seq\_num, 8-bit bitmap}
- MORE<sup>[6](/NOTES.md#chachulski07)</sup>
  + Opportunistic routing, but solves the problem of knowing who has what
  + Applies RLNC to `K` chunks of a message, each with size `q` (2<sup>n</sup>)
  + Header
    * Batch ID
    * Source/destination address
    * *code vector*: coefficients from GF<sub>q</sub>
    * List of potential forwarders (calculated from ETX)
  + Forwarders
    * Check forwarder list for their address
    * Check whether packet is *innovative*: linearly independent with other
      overheard packets in the same batch
    * Broadcast *new* linear combination of all batch packets overheard so far
  + Receiver ACKs packet once it has decoded it
- CORE<sup>[2](/NOTES.md#krigslund13)</sup>

Security
--------

A good overview: [\[7\]](/NOTES.md#dong08)

- Confidentiality
  + Off-the-shelf crypto (?)
- Integrity
  + Packet pollution
    - Routing nodes will propagate corrupted LCs
    - Open problem, as of 2008
- Availability
  + Misinformation
    * False packet reception information
      - Routers will send undecipherable information
      - Lightweight MACs
  + Routing data pollution
    * Link state pollution
    * Neighbor set pollution (wormholes, etc.)
    * Existing routing-protocol security?
  + ACK injection
    * Again, lightweight MACs
  + Packet dropping
    * Doesn't the built-in redundancy deal with this?
- Other issues
  + Nodes can misreport coding opportunities, and so make themselves very likely
    to be chosen as forwarders
- Technologies
  + ERSS-RLNC<sup>[4](/NOTES.md#noura12)</sup>
  + Homomorphic hashing/crypto
    * Tends to be expensive
    * Find some references!

References
----------

1. <a label="katti06"/>
   Katti, S.; Rahul, H.; Wenjun Hu; Katabi, D.; Medard, M.; Crowcroft, J.,
   "XORs in the Air: Practical Wireless Network Coding,"
   in Networking, IEEE/ACM Transactions on , vol.16, no.3, pp.497-510, June 2008
2. <a label="krigslund13"/>
   Krigslund, J.; Hansen, J.; Hundeboll, M.; Lucani, D.E.; Fitzek, F.H.P.,
   "CORE: COPE with MORE in Wireless Meshed Networks,"
   in Vehicular Technology Conference (VTC Spring), 2013 IEEE 77th ,
   vol., no., pp.1-6, 2-5 June 2013
3. <a label="koetter02">
   Koetter, R.; Medard, M.,
   "Beyond routing: an algebraic approach to network coding,"
   in INFOCOM 2002. Twenty-First Annual Joint Conference of the IEEE Computer 
   and Communications Societies. Proceedings. IEEE ,
   vol.1, no., pp.122-130 vol.1, 2002
4. <a label="noura12">
   Noura, H.; Martin, S.; Al Agha, K.; Chahine, K.,
   "ERSS-RLNC: Efficient and robust secure scheme for random linear network
   coding"
   in Computer Networks, 2014. vol.75, no., pp.99-112, Dec. 2014
5. <a label="ho06">
   Tracey Ho; Medard, M.; Koetter, R.; Karger, D.R.; Effros, M.;
   Jun Shi; Leong, B.,
   "A Random Linear Network Coding Approach to Multicast,"
   in Information Theory, IEEE Transactions on ,
   vol.52, no.10, pp.4413-4430, Oct. 2006
6. <a label="chachulski07">
   Szymon Chachulski, Michael Jennings, Sachin Katti, and Dina Katabi. 2007.
   "Trading structure for randomness in wireless opportunistic routing."
   In Proceedings of the 2007 conference on Applications, technologies,
   architectures, and protocols for computer communications (SIGCOMM '07).
   ACM, New York, NY, USA, 169-180. 
7. <a label="dong08">
   Jing Dong; Curtmola, R.; Sethi, R.; Nita-Rotaru, C.,
   "Toward secure network coding in wireless networks: Threats and challenges,"
   in Secure Network Protocols, 2008. NPSec 2008. 4th Workshop on ,
   vol., no., pp.33-38, 19-19 Oct. 2008
