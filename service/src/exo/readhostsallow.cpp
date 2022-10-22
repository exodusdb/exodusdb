#include <exodus/library.h>
libraryinit()

function main(out hostsallow) {

	hostsallow = R"V0G0N(
#dubai barsha office
sshd: 94.200.49.94
#use netmask instead of individual ips for speed
#94.200.49.144/29 144 to 150 (actually .248 includes .151
sshd: 94.200.49.144/255.255.255.248
#list individually
sshd: 94.200.49.144
sshd: 94.200.49.145
sshd: 94.200.49.146
sshd: 94.200.49.147
sshd: 94.200.49.148
sshd: 94.200.49.149
sshd: 94.200.49.150

#uk.neosys.com
sshd: 82.69.66.16

#nl10
sshd: 37.48.81.100
#nl10r
sshd: 37.48.81.101
#monitor
sshd: 37.48.81.102

#nl12
sshd: 95.211.226.208
#nl12r
sshd: 95.211.226.213
#monitor
sshd: 95.211.226.214

#localhost in case reverse tunnel
sshd: 127.

#nl13
sshd: 5.79.74.40
)V0G0N";

	//fm instead of crlf
	hostsallow.converter("\n\r", FM);

	return 0;
}

libraryexit()
