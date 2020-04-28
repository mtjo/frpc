# frpc

启动支持方法

MKDIR /var/run/avahi-daemon
MKDIR /etc/dbus-1/system.d
RUN /usr/sbin/avahi-daemon -D
STOP /usr/sbin/avahi-daemon -k
MKDIR /userdata/timecapsule/afp/netatalk/CNID
MKDIR /var/lock
MKDIR /var/log/
RUN usr/sbin/netatalk
RUN usr/sbin/timemachine_thrift
STOP killall usr/sbin/timemachine_thrift
STOP killall netatalk
STOP killall afpd
STOP killall cnid_dbd
STOP killall cnid_metad
STOPPED avahi-daemon: running [MacBackup.local]
STOPPED usr/sbin/timemachine_thrift
STOPPED /usr/sbin/netatalk
STOPPED /usr/sbin/afpd -d -F /etc/netatalk/afp.conf
STOPPED /usr/sbin/cnid_metad -d -F /etc/netatalk/afp.conf
STOPPED /usr/sbin/cnid_dbd -F /etc/netatalk/afp.conf
