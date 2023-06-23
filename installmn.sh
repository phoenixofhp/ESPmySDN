#!/bin/sh
#Installing pip, mininet, openflow, nmap, ryu, xterm and wireshark
#Requires some confirmations during install
#Tested on Ubuntu 20.04 05.05.23

#update & upgrade packages
echo "Starting apt update-upgrade"
sudo apt update && sudo apt upgrade -y
echo "Finished apt update-upgrade"

#installing Mininet + Openflow prerequisites + nmap(optional) + Wireshark(optional) + pip + xterm
echo "Starting installing most of software"
sudo apt install git gcc make openssl autoconf pkg-config mininet nmap wireshark python3-pip xterm -y
echo "Finished installing most of software"

#cloning and building Openflow
echo "Starting installing openflow"
cd
git clone https://github.com/mininet/openflow.git
cd openflow
./boot.sh
./configure
sudo make
sudo make install
echo "Finished installing openflow"

#installing ryu and prerequisites
echo "Starting installing ryu"
sudo apt install gcc python-dev libffi-dev libssl-dev libxml2-dev libxslt1-dev zlib1g-dev -y
sudo pip install eventlet==0.30.2
sudo pip install ryu
echo "Finished installing ryu"

#installing zenmap
echo "Starting installing zenmap"
wget http://archive.ubuntu.com/ubuntu/pool/universe/p/pygtk/python-gtk2_2.24.0-5.1ubuntu2_amd64.deb
sudo apt install ./python-gtk2_2.24.0-5.1ubuntu2_amd64.deb -y
sudo rm -f ./python-gtk2_2.24.0-5.1ubuntu2_amd64.deb
wget http://archive.ubuntu.com/ubuntu/pool/universe/n/nmap/zenmap_7.60-1ubuntu5_all.deb
sudo apt install ./zenmap_7.60-1ubuntu5_all.deb -y
sudo rm -f ./zenmap_7.60-1ubuntu5_all.deb
echo "Finished installing zenmap"



#Testing mininet:
#Run "sudo mn"
#Expected output:

	#*** Creating network
	#*** Adding controller
	#*** Adding hosts:
	#h1 h2 
	#*** Adding switches:
	#s1 
	#*** Adding links:
	#(h1, s1) (h2, s1) 
	#*** Configuring hosts
	#h1 h2 
	#*** Starting controller
	#c0 
	#*** Starting 1 switches
	#s1 ...
	#*** Starting CLI:
	#mininet> 

#Run "pingall"
#Expected output:

	#*** Ping: testing ping reachability
	#h1 -> h2 
	#h2 -> h1 
	#*** Results: 0% dropped (2/2 received)




#Testing mininet+ryu:
#Run "sudo ryu-manager ryu.app.simple_switch_13"
#Expected output:

	#loading app ryu.app.simple_switch_13
	#loading app ryu.controller.ofp_handler
	#instantiating app ryu.app.simple_switch_13 of SimpleSwitch13
	#instantiating app ryu.controller.ofp_handler of OFPHandler

#Run in another terminal instance "sudo mn --topo single,3 --mac --switch ovsk --controller remote"
#Expected output:

	#*** Creating network
	#*** Adding controller
	#Connecting to remote controller at 127.0.0.1:6653
	#*** Adding hosts:
	#h1 h2 h3 
	#*** Adding switches:
	#s1 
	#*** Adding links:
	#(h1, s1) (h2, s1) (h3, s1) 
	#*** Configuring hosts
	#h1 h2 h3 
	#*** Starting controller
	#c0 
	#*** Starting 1 switches
	#s1 ...
	#*** Starting CLI:
	#mininet> 

#Run "pingall"
#Expected output:

	#*** Ping: testing ping reachability
	#h1 -> h2 h3 
	#h2 -> h1 h3 
	#h3 -> h1 h2 
	#*** Results: 0% dropped (6/6 received)
