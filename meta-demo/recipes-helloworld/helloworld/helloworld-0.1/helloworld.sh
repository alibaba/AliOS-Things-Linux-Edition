#!/bin/sh

### BEGIN INIT INFO
# Provides:          helloworld
# Required-Start:
# Required-Stop:
# Default-Start:     2 3 4 5
# Default-Stop:      0 1 6
# Short-Description: A helloworld application
### END INIT INFO


run_helloworld() {
    echo "Executing helloworld ..."
    /usr/bin/helloworld > /dev/null 2>&1 &
}

case $1 in
    start)
        run_helloworld
        ;;
    stop)
        killall helloworld > /dev/null 2>&1
        ;;
    restart|reload)
        $0 stop && sleep 1 && $0 start
        ;;
    *)
        echo "$0 start|stop|restart|reload"
        ;;
esac
