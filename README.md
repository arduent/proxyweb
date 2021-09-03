# proxyweb

This is a Qt c++ Desktop application that allows the user to load a web page at the specified URL into the browser component.

Firefox, Chrome and other browsers have proxy configuration settings, however when the proxy is enabled it effects all urls, unless the user specifies urls in the negative list. In practive this becomes tedious over time as the list grows. They might consider having a "load in proxy" checkbox on the url field, with default (off) or (on) as the user wishes. ??? But maybe this is already solved and it's a non-issue. proxyweb was created to load admin / private control panels / web admin that are accessible only by VPN, SSH, etc. For example if the user has Zabbix running on host x.y.z and the only access is through a wireguard vpn on wg3, so they have to route through 10.10.1.1 or something. 

proxyweb has a history table with sortable columns. Clicking on a row in the history table will load the corresponding URL.

proxyweb uses an SQLite databse to store history and settings. On FreeBSD the driver is installed by the qt5-sqldrivers-sqlite3 port. (or pkg). Check the Qt documentation for more information about how to install the SQLite driver on your system. proxyweb was developed on a FreeBSD system and should run without modification on FreeBSD and other BSD systems, and GNU/Linux, Mac and MS Windows.

## Build It

To build you will need the Qt development packages installed on your system (using qmake). 

Use make distclean just in case there's some system-specific files left in the source distribution. It will likely cause an error if no extraneous files are present. This error/warning is expected and can be safely ignored.

```
# make distclean
# qmake
# make
# ./proxyweb
```


## Questions?

contact: Waitman Gobble waitman@waitman.net 

