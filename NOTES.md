Network Coding Notes
====================

Existing protocols
------------------

- COPE<sup>[1](NOTES.md#katti06)</sup>
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
  + Learning Neighbor State
    * When info from reception reports is unavailable, routers guess
    * Uses delivery probability (e.g., ETX)
    * Already present in some link-state routing protocols (?)
- CORE<sup>[2](NOTES.md#krigslund13)</sup>

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
