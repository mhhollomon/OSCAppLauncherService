# OSC Application Launcher Service

Small [Open Sound Control](https://opensoundcontrol.org) server to launch applications in response to OSC commands.

This was conceived as a "helper" of sorts to [Open Stage Control](https://github.com/jean-emmanuel/open-stage-control) to allow 
me to not only control my DAW but also start it using OSC.

It will most likely grow well beyond that.

## V0.2.0 Changes

- ported to linux.
- Changed format  and name of OALServer config file.


## OALServer

The server application. 

Reads an configuration file from:
- Windows = `%USER_PROFILE%\\Documents\\OALServer.cfg`
- Linux = `$HOME/.oal/OALServer.cfg`

```
server: {
	port = 7777
}
	
launch : {

	notepad = "C:\\Program Files\\Notepad++\\notepad++.exe"
	reaper  = "C:\\Program Files\\REAPER (x64)\\reaper.exe"
}
```

Listens on 127.0.0.1:{port} for UDP Datagrams. If `port` is not given in the ini file, it defaults to 8888.

if the path in the datagram is `/launch/{appkey}`, it will run the program listed for `appkey` in the ini file.

if the path in the datagram is `/exit`, it will shutdown.

Otherwise it does nothing.

- Only listens to UDP
- Currently, only listens to localhost interface.
- No arguments can be sent to the application.
- Does not handle OSC bundles
- Ignores any arguments in the OSC message.

## OALSend

Small utility to send OSC messages via UDP on `127.0.0.1:<port>`. The port and OSC path are given on the command line.

```
OSCSend.exe 7777 /launch/notepad
```

## Integrating with Open Stage Control

Add the 127.0.0.1:<port> to the `osc` property of the widget.
Add the path (e.g. `/launch/notepad`) to the `path` property of the widget.

![Widget Properties in OSC Editor](/docs/OSC_widget_config.png)

## Tech Used

* [libconfig](https://github.com/hyperrealm/libconfig) for configuration parsing.

## Reference

 * [Open Sound Control](https://opensoundcontrol.org/)
 * [Open Stage Control](https://github.com/jean-emmanuel/open-stage-control)

