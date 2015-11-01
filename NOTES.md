Network Coding Notes
====================

Technologies
------------

- Algebraic Linear Network Coding<sup>[3](/REFS.md#koetter02)</sup>
- RLNC<sup>[5](/REFS.md#ho06)</sup>

Existing protocols
------------------

- COPE<sup>[1](/REFS.md#katti06)</sup>
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
- MORE<sup>[6](/REFS.md#chachulski07)</sup>
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
- CORE<sup>[2](/REFS.md#krigslund13)</sup>
- TCP/NC<sup>[8](/REFS.md#sundarajan11)</sup>
- CTCP<sup>[9](/REFS.md#kim13)</sup>

Security
--------

A good overview: [\[7\]](/REFS.md#dong08)

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
  + ERSS-RLNC<sup>[4](/REFS.md#noura12)</sup>
  + Homomorphic hashing/crypto
    * Tends to be expensive
    * Find some references!

Ideas
-----

1. Implement MORE and try to make it secure against byzantine attackers
   - On top of batman-adv (?)
2. Rather than sending a vector of coefficients, each sender/router sends
   PK-encrypted seed to a cryptographic PRNG (cf.
   CTCP<sup>[9](/REFS.md#kim13)</sup>)
   - Receiver decrypts seed using private key and uses it to generate
     coefficients
   - Intermediate nodes can't recover coefficients, but they can still create
     new linear combinations and send their own PK-encrypted seeds
   - How do public keys get distributed?
   - Does this help against pollution attacks?
   - Vulnerable to integrity compomises, I think
     + To modify byte *n* without decoding it, just multiply it by some
       value and don't change the coefficients.
     + Add a hash to the message? We already have PK-infrastructure overhead...
