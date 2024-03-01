# VPN Intercom

This project will strive to interconnect different devices with different hardware (Raspberry Pi 2B, 4B and ESP32-CAM) under a really simple UDP protocol to transfer signals, data and images over a _Virtual Private Network_ (VPN). These devices can be anywhere you want! They will only have to have access to the internet!

The software dessign is really modular so you won't need to change much to adapt it to your necessities. It's also written in C which is the most used language on embedded devices so that you can program on as much platforms as possible!

## Purpose and usecase

Imagine that you have a cellar or an storage room. You want to control the lights, the power outlets and you also want to have video feed of the interior, just in case somebody who shouldn't be there happens to forcefully enter.

The only problem that is stoping you from just getting a surveillance camera and some smart switches is that **there is no internet down there!** Ethernet cable is out of the question... But, against all odds, you got some bars on your cellphone! Maybe you can just use your mobile phone as a hotspost or buy a SIM router and connect all the devices to that network! Nevertheles, another problem arises... How do you access all those devices **from your home**?

We're accustomed to just having an app who connects us from our cell phone to the device. The most basic device will grant us [**connection only when we're on the same network**](#under-the-same-network); and there are others, more advanced ones, which use **external servers to grant us connection from everywhere** there is internet access. The problem I have with the last option, which again, could solve our problem, is that I don't feel confortable sending images to the internet of a place I don't want people storming in. Who knows who's watching besides me!

There's a **third** option: we can connect to the devices only by knowing their **public IP address**. Whichever option we're using as internet access on the cellar, either the router or the cell phone's hotspot, the internet access will be given by a SIM card. If we're using a router we may be lucky to forward different ports to the devices to connect from the outside and, in the best case scenario, we can even get to set an static public IP, but I assure you that this path looks torturous and rather expensive.

> _As I guess that maybe not everyone reading has a total understanding of what I'm referring by public and private IPs and port forwarding, let me just give you a quick rundown. Click [here](#a-quick-rundown-of-the-basics)!_

Unlucky for us, **SIM cards don't normally have a static IP address**. To further exacerbate the problem, **multiple devices connected to the same mobile operator can have the same public IP**. This is due to [**CGNAT**](https://en.wikipedia.org/wiki/Carrier-grade_NAT), a protocol which strives to reduce the number of IPs used to battle the shortage of IPv4 addresses. This protocol will allow you to request something FROM the internet, but it won't allow you to request something TO the device from the internet, as the IP of your phone is shared between many others, so the communicator wouldn't know how to address you specifically.

You may be thinking: "So how do WhatsApp messages work then?These messages come from the outside (the internet) to our phone". My best guess is that our phone will be **polling** (continuosly checking and requesting new data to a server) in case there are any new messages. Same thing happens for email. For example, if your mobile is on "battery saving" mode, you may have noticed that email don't reach you! That may as well be because the polling interval gets stretched so to not consume that much battery. Again, this is my guess, so take it with a pinch of salt. 😅

To sum up, trying to reach the SIM with a public IP looks to be impossible. There are also static IP SIM cards, but those are rather expensive and hard to find!

# Under the same network

Let's address the _"under the same network"_ scenario. When I say the devices are on the same network, it could also be said that they are connected to the same router. 

## A quick rundown of the basics
