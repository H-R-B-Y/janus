
# see man netlink.7

nl_groups  is  a  bit  mask  with  every bit representing a netlink group number.  Each netlink family has a set of 32 multicast groups.  When bind(2) is
called on the socket, the nl_groups field in the sockaddr_nl should be set to a bit mask of the groups which it wishes to listen to.  The  default  value
for  this  field  is  zero  which  means  that no multicasts will be received.  A socket may multicast messages to any of the multicast groups by setting
nl_groups to a bit mask of the groups it wishes to send to when it calls sendmsg(2) or does a connect(2).  Only processes with an effective UID of  0  or
the  CAP_NET_ADMIN  capability may send or listen to a netlink multicast group.  Since Linux 2.6.13, messages can't be broadcast to multiple groups.  Any
replies to a message received for a multicast group should be sent back to the sending PID and the multicast group.  Some Linux kernel subsystems may ad‐
ditionally allow other users to send and/or receive  messages.   As  at  Linux  3.0,  the  NETLINK_KOBJECT_UEVENT,  NETLINK_GENERIC,  NETLINK_ROUTE,  and
NETLINK_SELINUX groups allow other users to receive messages.  No groups allow other users to send messages.


## the following are not documented, and consiquently we are unable to determine what messages are actually part of these groups
```c
/* RTnetlink multicast groups - backwards compatibility for userspace */
#define RTMGRP_LINK		1
#define RTMGRP_NOTIFY		2
#define RTMGRP_NEIGH		4
#define RTMGRP_TC		8

#define RTMGRP_IPV4_IFADDR	0x10
#define RTMGRP_IPV4_MROUTE	0x20
#define RTMGRP_IPV4_ROUTE	0x40
#define RTMGRP_IPV4_RULE	0x80

#define RTMGRP_IPV6_IFADDR	0x100
#define RTMGRP_IPV6_MROUTE	0x200
#define RTMGRP_IPV6_ROUTE	0x400
#define RTMGRP_IPV6_IFINFO	0x800

#define RTMGRP_DECnet_IFADDR    0x1000
#define RTMGRP_DECnet_ROUTE     0x4000

#define RTMGRP_IPV6_PREFIX	0x20000
```