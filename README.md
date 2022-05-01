# OSC Application Launcher Service

Small [Open Sound Control](https://opensoundcontrol.org) server to launch application in response to OSC commands.

Very much at the proof-of-concept stage.

- Only listens to UDP
- Currently, the port is hardcoded at 8888.
- Currently, only listens to localhost interface.
- Currently, the app list is hard coded.

## OALServer

The server application. Listens on 127.0.0.1:8888 for UDP Datagrams.

if the path in the datagram is `/launch/{appkey}`, it will run the program from the built in map corresponding to `appkey`.

if the path in the datagram is `/exit`, it will shutdown.

Otherwise it does nothing.

## OALSend

Small utility to send OSC messages via UDP on 127.0.0.1:8888. The OSC path is given on the command line.

```
OSCSend.exe /launch/notepad
```

