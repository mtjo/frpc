#!/bin/ash
# model=$(cat /proc/xiaoqiang/model)
# setconfig model $model
# version=$(/frp/frpc -v);
# setconfig version $version
# configType=$(getconfig configType)
# if [[ "$configType" == "base" ]]; then
# 	/frp/frpc -c /etc/frpc_config.ini &>/dev/null 
# else
# 	/frp/frpc -c /etc/frpc_user_config.ini &>/dev/null
# fi
chmod -R 666 /userdata/Frp
echo 1 >> /userdata/Frp/step.txt
/frp/frpc -c /bin/frpc_router.ini >> /userdata/Frp/frpc.log&
echo 2 >> /userdata/Frp/step.txt
# pid=$(pidof frpc);

while :
do
	# PID=$(pidof frpc);
	# if [[ "$pid" != "$PID" ]]; then
	# 	setconfig PID $PID
	# 	pid=$PID
	# fi
	sleep 3
done