# OSC Application Launcher Service

Small [Open Sound Control](https://opensoundcontrol.org) server to launch application in response to OSC commands.

Very much at the proof-of-concept stage.

- Only listens to UDP
- Currently, the port is hardcoded at 8888.
- Currently, only listens to localhost interface.
- Currently, the app list is hard coded.

## OALServer

The server application. 

Reads an ini file from `%USER_PROFILE%\\Documents\\OALServer.ini`

```
; Sample file
[Server]
port=7777

[Applications]
notepad=C:\Program Files\Notepad++\notepad++.exe
reaper=C:\Program Files\REAPER (x64)\reaper.exe
```

Listens on 127.0.0.1:{port} for UDP Datagrams. If `port` is not given in the ini file, it defaults to 8888.

if the path in the datagram is `/launch/{appkey}`, it will run the program listed for `appkey` in the ini file.

if the path in the datagram is `/exit`, it will shutdown.

Otherwise it does nothing.

## OALSend

Small utility to send OSC messages via UDP on `127.0.0.1:<port>`. The port and OSC path is given on the command line.

```
OSCSend.exe 7777 /launch/notepad
```

## Tech Used

* [inipp](https://github.com/mcmtroffaes/inipp) for ini parsing.

