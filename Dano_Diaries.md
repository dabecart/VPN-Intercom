# Devlog: Dano Diaries

Here I have my TODO chores because if I write them all over the code, I'll end up loosing them. ✨

## 02/03/2024

TODOs:
- Add to UDPutils' sendXMLPacketTo() the expects response section. 
- Think about how to handle repetitions of packets in case it does not reach the device at the end!
- Think about how to handle wrong packets with wrong IPs.

## 01/03/2024: First commit, getting a dummy program to work

I think I got the basics of the software. I got my receptor and transciever working in different threads and the _Acknowledge_ signal is on the works. To test it, I've created this git to pass the code between the devices.

TODOs:
- Create the parser for the incoming XML files.
- Create a temporary safe buffer for the acknowledge packets that have to be resent.
- Test & develop.
- Continue with the README, sleepy head.