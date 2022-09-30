# rcan

*rcan* creates a bridge between can bus(es) located on a remote computer 
and socket-can interfaces on the local computer.

The remote can buses must be accessible in a supported frame-format on a 
TCP port of the remote computer.

The currently supported formats are gvret-b and crtd.

Usage:  
    ```rcan host protocol bus0 if_name0 [bus1 if_name1 [...]]```

Example:  
    ```rcan can.not.net:23 crtd 0 vcan0 1 vcan1```  
This example connects to port 23 of the remote computer at the address can.not.net and bridges can bus 0 of the remote computer to the socket-can interface vcan0 of the local computer and can bus 1 to vcan1
 



