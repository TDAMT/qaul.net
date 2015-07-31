/*
 * qaul.net is free software
 * licensed under GPL (version 3)
 */

/**
 * linux specific functions of qaulhelper.
 *
 * usage:
 *   qaulhelper startolsrd <ISGATEWAY yes|no> <INTERFACE>
 *   qaulhelper startolsrd yes wlan0
 *   qaulhelper stopolsrd
 *   qaulhelper stopolsrd
 *   qaulhelper startportforwarding <INTERFACE> <IP>
 *   qaulhelper startportforwarding wlan0 10.213.28.55
 *   qaulhelper stopportforwarding
 *   qaulhelper stopportforwarding
 *   qaulhelper startgateway <INTERFACE>
 *   qaulhelper startgateway wlan0
 *   qaulhelper stopgateway
 *   qaulhelper stopgateway
 *   qaulhelper startnetworkmanager
 *   qaulhelper startnetworkmanager
 *   qaulhelper stopnetworkmanager
 *   qaulhelper stopnetworkmanager
 *   qaulhelper enablewifi <INTERFACE>
 *   qaulhelper enablewifi wlan0
 *   qaulhelper configurewifi <INTERFACE> <ESSID> <CHANNEL> [<BSSID>]
 *   qaulhelper configurewifi wlan0 qaul.net 11 02:11:87:88:D6:FF
 *   qaulhelper setip <INTERFACE> <IP> <SUBNET> <BROADCAST>
 *   qaulhelper setip wlan0 10.213.28.55 8 10.255.255.255
 *   qaulhelper setdns <INTERFACE>
 *   qaulhelper setdns wlan0
 *
 */

#include "qaulhelper.h"


int start_olsrd (int argc, const char * argv[])
{
    pid_t pid1;
    int status, fd;
    char s[256];
    printf("start olsrd\n");

    if(argc >= 4)
    {
        // validate arguments
        if(strncmp(argv[3], "yes", 3)==0)
            sprintf(s,"/opt/qaul/etc/olsrd_linux_gw.conf");
        else
            sprintf(s,"/opt/qaul/etc/olsrd_linux.conf");

        if (validate_interface(argv[3]) == 0)
        {
            printf("argument 2 not valid\n");
            return 0;
        }

        // become root
        setuid(0);

        // start olsrd
        pid1 = fork();
        if (pid1 < 0)
            printf("fork for pid1 failed\n");
        else if(pid1 == 0)
        {
            fd = open("/dev/null", O_WRONLY | O_CREAT | O_APPEND);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            // execute program
            execl("/opt/qaul/bin/olsrd", "olsrd", "-f", s, "-i", argv[3], "-d", "0", (char*)0);
        }
        else
            waitpid(pid1, &status, 0);

        printf("olsrd started\n");
    }
    else
        printf("missing argument\n");

	return 0;
}

int stop_olsrd (int argc, const char * argv[])
{
    pid_t pid1;
    int status;
    printf("stop olsrd\n");

    // become root
    setuid(0);

    // kill olsrd
    pid1 = fork();
    if (pid1 < 0)
        printf("fork for pid1 failed\n");
    else if(pid1 == 0)
        execl("/usr/bin/killall", "killall", "olsrd", (char*)0);
    else
        waitpid(pid1, &status, 0);

    printf("olsrd stopped\n");
    return 0;
}

int start_portforwarding (int argc, const char * argv[])
{
    pid_t pid1, pid2, pid3;
    int fd, status;
    printf("start portforwarding\n");

    if(argc >= 4)
    {
        // validate arguments
        if (validate_interface(argv[2]) == 0)
        {
            printf("argument 1 not valid\n");
            return 0;
        }

        // become root
        setuid(0);

        // forward tcp port 80 (http) by iptables
        pid1 = fork();
        if (pid1 < 0)
            printf("fork for pid1 failed\n");
        else if(pid1 == 0)
        {
            // redirect standart output and error to /dev/null
            // the program otherwise often didn't return correctly
            fd = open("/dev/null", O_WRONLY | O_CREAT | O_APPEND);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            // execute program
            execl("/sbin/iptables", "iptables", "-t", "nat", "-I", "PREROUTING", "1", "-i", argv[2], "-p", "tcp", "-d", argv[3], "--dport", "80", "-j", "REDIRECT", "--to-port", "8081", (char*)0);
        }
        else
            printf("tcp port 80 forwarded\n");

        // forward udp port 53 (dns) by iptables
        pid2 = fork();
        if (pid2 < 0)
            printf("fork for pid2 failed\n");
        else if(pid2 == 0)
        {
            // redirect standart output and error to /dev/null
            // the program otherwise often didn't return correctly
            fd = open("/dev/null", O_WRONLY | O_CREAT | O_APPEND);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            // execute program
            execl("/sbin/iptables", "iptables", "-t", "nat", "-I", "PREROUTING", "1", "-i", argv[2], "-p", "udp", "-d", argv[3], "--dport", "53", "-j", "REDIRECT", "--to-port", "8053", (char*)0);
        }
        else
            printf("udp port 53 forwarded\n");

        // forward DHCP port via portfwd
        pid3 = fork();
        if (pid3 < 0)
            printf("fork for pid3 failed\n");
        else if(pid3 == 0)
        {
            // redirect standart output and error to /dev/null
            // the program otherwise often didn't return correctly
            fd = open("/dev/null", O_WRONLY | O_CREAT | O_APPEND);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            // execute program
            execl("/opt/qaul/bin/portfwd", "portfwd", "-c", "/opt/qaul/etc/portfwd.conf", (char*)0);
        }
        else
            printf("udp port 67 forwarded\n");

        printf("portforwarding started\n");
    }
    else
        printf("missing argument\n");

    return 0;
}

int stop_portforwarding (int argc, const char * argv[])
{
    pid_t pid1, pid2;
    int status;
    printf("stop port forwarding\n");

    // become root
    setuid(0);

    // remove firewall rules
    pid1 = fork();
    if (pid1 < 0)
        printf("fork for pid1 failed\n");
    else if(pid1 == 0)
        execl("/sbin/iptables", "iptables", "-t", "nat", "-D", "PREROUTING", "1", (char*)0);
    else
        printf("tcp port 80 forwarding stopped\n");

    // stop portfwd
    pid2 = fork();
    if (pid2 < 0)
        printf("fork for pid2 failed\n");
    else if(pid2 == 0)
        execl("/usr/bin/killall", "killall", "socat", (char*)0);
    else
        waitpid(pid2, &status, 0);

    printf("port forwarding stopped\n");
	return 0;
}

int start_gateway (int argc, const char * argv[])
{
    pid_t pid1, pid2, pid3;
    int fd, status;
    printf("start gateway\n");

    if(argc >= 3)
    {
        // validate arguments
        if (validate_interface(argv[2]) == 0)
        {
            printf("argument 1 not valid\n");
            return 0;
        }

        // NOTE: don't do that withsetuid, for that the user
        //       has to enter password for this service
        //
        // become root
        //setuid(0);

        // set gateway variable
        pid1 = fork();
        if (pid1 < 0)
            printf("fork for pid1 failed\n");
        else if(pid1 == 0)
        {
            // redirect standart output and error to /dev/null
            // the program otherwise often didn't return correctly
            fd = open("/dev/null", O_WRONLY | O_CREAT | O_APPEND);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            // execute program
            execl("/usr/sbin/sysctl", "sysctl", "-w", "net.inet.ip.forwarding=1", (char*)0);
        }
        else
            waitpid(pid1, &status, 0);

        // start natd
        pid2 = fork();
        if (pid2 < 0)
            printf("fork for pid2 failed\n");
        else if(pid2 == 0)
        {
            // redirect standart output and error to /dev/null
            // the program otherwise often didn't return correctly
            fd = open("/dev/null", O_WRONLY | O_CREAT | O_APPEND);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            // execute program
            execl("/usr/sbin/natd", "natd", "-interface", argv[2], (char*)0);
        }
        else
            waitpid(pid2, &status, 0);

        // set
        pid3 = fork();
        if (pid3 < 0)
            printf("fork for pid3 failed\n");
        else if(pid3 == 0)
        {
            // redirect standart output and error to /dev/null
            // the program otherwise often didn't return correctly
            fd = open("/dev/null", O_WRONLY | O_CREAT | O_APPEND);
            dup2(fd, STDOUT_FILENO);
            dup2(fd, STDERR_FILENO);
            close(fd);
            // execute program
            execl("/sbin/ipfw", "ipfw", "add", "1055", "divert", "natd", "all", "from", "any", "to", "any", "via", argv[2], (char*)0);
        }
        else
            printf("NAT activated\n");

        printf("gateway started\n");
    }
    else
        printf("missing argument\n");

    return 0;
}

int stop_gateway (int argc, const char * argv[])
{
    pid_t pid1, pid2;
    printf("stop gateway\n");

    // become root
    setuid(0);

    // remove firewall rules
    pid1 = fork();
    if (pid1 < 0)
        printf("fork for pid1 failed\n");
    else if(pid1 == 0)
        execl("/sbin/ipfw", "ipfw", "delete", "1055", (char*)0);
    else
        printf("firewall rules removed\n");

    // stop port forwarding
    pid2 = fork();
    if (pid2 < 0)
        printf("fork for pid2 failed\n");
    else if(pid2 == 0)
        execl("/usr/bin/killall", "killall", "natd", (char*)0);
    else
        printf("natd stopped\n");

    printf("gateway stopped\n");
    return 0;
}

#ifdef WITHOUT_NETWORKMANAGER

int start_networkmanager (int argc, const char * argv[])
{
    pid_t pid1;
    int status;
    printf("start network manager\n");

	// become root
	setuid(0);

	// start network manager
	pid1 = fork();
	if (pid1 < 0)
		printf("fork for pid1 failed\n");
	else if(pid1 == 0)
        execl("/usr/bin/service", "service", "network-manager", "start", (char*)0);
	else
		waitpid(pid1, &status, 0);

	printf("network manager started\n");

    return 0;
}

int stop_networkmanager (int argc, const char * argv[])
{
    pid_t pid1;
    int status;
    printf("stop network manager\n");

    // become root
    setuid(0);

    // network manager
    pid1 = fork();
    if (pid1 < 0)
        printf("fork for pid2 failed\n");
    else if(pid1 == 0)
        execl("/usr/bin/service", "service", "network-manager", "stop", (char*)0);
    else
        waitpid(pid1, &status, 0);

    printf("network manager stopped\n");
	return 0;
}

int enable_wifi (int argc, const char * argv[])
{
    pid_t pid1, pid2;
    int status;
    printf("enable wifi\n");

	// become root
	setuid(0);

	// deblock wifi
	pid1 = fork();
	if (pid1 < 0)
		printf("fork for pid1 failed\n");
	else if(pid1 == 0)
	{
		execl("rfkill", "unblock", "all", (char*)0);
	}
	else
		waitpid(pid1, &status, 0);

	// enable wifi
	pid2 = fork();
	if (pid2 < 0)
		printf("fork for pid1 failed\n");
	else if(pid2 == 0)
	{
		execl("nmcli", "nm", "wifi", "on", (char*)0);
	}
	else
		waitpid(pid2, &status, 0);


	printf("wifi enabled\n");

	return 0;
}

int configure_wifi (int argc, const char * argv[])
{
    pid_t pid1, pid2, pid3, pid4, pid5, pid6;
    int status;
    char s[256];
    printf("create or join ibss\n");

    if(argc >= 5)
    {
        // validate arguments
        if (validate_interface(argv[2]) == 0)
        {
            printf("argument 1 not valid\n");
            return 0;
        }
        if (validate_essid(argv[3]) == 0)
        {
            printf("argument 2 not valid\n");
            return 0;
        }
        if (validate_number(argv[4]) == 0)
        {
            printf("argument 3 not valid\n");
            return 0;
        }

        // become root
        setuid(0);

        // take wifi interface down
        pid1 = fork();
        if (pid1 < 0)
            printf("fork for pid1 failed\n");
        else if(pid1 == 0)
        {
            execl("/bin/ip", "ip", "link", "set", argv[2], "down", (char*)0);
        }
        else
            waitpid(pid1, &status, 0);

		printf("wifi interface down\n");

        // set adhoc mode
        pid2 = fork();
        if (pid2 < 0)
            printf("fork for pid2 failed\n");
        else if(pid2 == 0)
        {
            execl("/sbin/iwconfig", argv[2], "mode", "ad-hoc", (char*)0);
        }
        else
            waitpid(pid2, &status, 0);

        printf("adhoc mode set\n");

        // set channel
        pid3 = fork();
        if (pid3 < 0)
            printf("fork for pid3 failed\n");
        else if(pid3 == 0)
        {
            execl("/sbin/iwconfig", argv[2], "channel", argv[4], (char*)0);
        }
        else
            waitpid(pid3, &status, 0);

		printf("channel set\n");

        // set essid
        pid4 = fork();
        if (pid4 < 0)
            printf("fork for pid4 failed\n");
        else if(pid4 == 0)
        {
            sprintf(s, "'%s'", argv[3]);
            execl("/sbin/iwconfig", argv[2], "essid", s, (char*)0);
        }
        else
            waitpid(pid4, &status, 0);

		printf("essid set\n");

		// configure BSSID
		if(argc >= 6)
		{
			// validate argument
			if (validate_interface(argv[5]) == 0)
			{
				printf("argument 4 not valid\n");
				return 0;
			}

			// take wifi interface down
			pid5 = fork();
			if (pid5 < 0)
				printf("fork for pid5 failed\n");
			else if(pid5 == 0)
			{
				execl("/sbin/iwconfig", argv[2], "ap", argv[5], (char*)0);
			}
			else
				waitpid(pid5, &status, 0);

			printf("BSSID set\n");
		}

        // bring wifi interface up
        pid6 = fork();
        if (pid6 < 0)
            printf("fork for pid6 failed\n");
        else if(pid6 == 0)
        {
            execl("/bin/ip", "ip", "link", "set", argv[2], "up", (char*)0);
        }
        else
            waitpid(pid6, &status, 0);

        printf("wifi configured\n");
    }
    else
        printf("missing argument\n");

	return 0;
}

int set_ip (int argc, const char * argv[])
{
    pid_t pid1;
    int status;
    char s[256];
    printf("configure ip\n");

    if(argc >= 6)
    {
        // validate arguments
        if (validate_interface(argv[2]) == 0)
        {
            printf("argument 1 not valid\n");
            return 0;
        }
        if (validate_ip(argv[3]) == 0)
        {
            printf("argument 2 not valid\n");
            return 0;
        }
        if (validate_number(argv[4]) == 0)
        {
            printf("argument 3 not valid\n");
            return 0;
        }
        if (validate_ip(argv[5]) == 0)
        {
            printf("argument 4 not valid\n");
            return 0;
        }

        // become root
        setuid(0);

        // set ip manually
        pid1 = fork();
        if (pid1 < 0)
            printf("fork for pid1 failed\n");
        else if(pid1 == 0)
        {
            sprintf(s,"%s/%s", argv[3], argv[4]);
            execl("/bin/ip", "ip", "addr", "add", s, "dev", argv[2], "broadcast", argv[5], (char*)0);
		}
        else
            waitpid(pid1, &status, 0);

        printf("ip configured\n");
    }
    else
        printf("missing argument\n");

	return 0;
}

int set_dns (int argc, const char * argv[])
{
    pid_t pid1, pid2, pid3;
    int status;
    printf("set DNS\n");

	// set root rights
	setuid(0);

	// remove tail
	pid1 = fork();
	if (pid1 < 0)
		printf("fork for pid1 failed\n");
	else if(pid1 == 0)
	{
		execl("/bin/rm", "rm", "/etc/resolvconf/resolv.conf.d/tail", (char*)0);
	}
	else
		waitpid(pid1, &status, 0);

	// set dns
	pid2 = fork();
	if (pid2 < 0)
		printf("fork for pid2 failed\n");
	else if(pid2 == 0)
	{
		execl("/bin/cp", "/opt/qaul/etc/tail", "/etc/resolvconf/resolv.conf.d/tail", (char*)0);
	}
	else
		waitpid(pid2, &status, 0);

	// reload resolv file
	pid3 = fork();
	if (pid3 < 0)
		printf("fork for pid3 failed\n");
	else if(pid3 == 0)
	{
		execl("/sbin/resolvconf", "resolvconf", "-u", (char*)0);
	}
	else
		waitpid(pid3, &status, 0);

	printf("DNS set\n");

    return 0;
}

#endif // WITHOUT_NETWORKMANAGER