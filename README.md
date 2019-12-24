# RDT Stop and Wait
Reliable Data Transfer over UDP using sequential Stop-And-Wait ARQ (Automatic Repeat reQuest)




This repository provides interfaces ```rdt_send()``` and ```rdt_recv()``` that are developed on top of User Datagram Protocol using sequential stop-and-wait sequence and acknowledgement number based ARQs. A static timer is used for timeout-retransmission mechanism. To use these interfaces, include ```rdt.c``` and ```rdt.h``` and compile ```rdt.c``` along with your project. Port numbers can be tweaked from ```rdt.c```.

While the performance is not better than TCP in the long run, it saves the connection establishment/termination and larger header overhead for small message sizes and should give a little performance boost. Some of the functions could have been optimal for which I chose readability over performance. For complete understanding, consider reading code comments and [slides](https://docs.google.com/presentation/d/1CMwttzw9EUZgEy74YztEuv_weAV_pDKgbUxFHhYJ1BE/edit?usp=sharing)
