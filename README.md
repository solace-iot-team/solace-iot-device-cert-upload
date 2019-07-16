# solace-iot-device-cert-upload
A command line tool to upload device credentials to the SIM Card via the SCC API.

## Download the binary release

https://github.com/solace-iot-team/solace-iot-device-cert-upload/releases/tag/v0.1.0

## Upload to Raspberry Pi

Upload utility and device credentials to the Raspberry Pi
```
scp solace-iot-device-cert-upload {user}@{rapberry-pi}:{target-path}
```
The device credentials names include the device id, please upload the private key, device certificate and CA certificate:
```
scp {device-id}.der.key {user}@{raspberry-pi-host}:{target-directory}
scp {device-id}.der.crt {user}@{raspberry-pi-host}:{target-directory}
scp digicert-ca.der.crt {user}@{raspberry-pi-host}:{target-directory}

```

## Running the utility
Connect to the Raspberry Pi via SSH, then execute the utility:
```
ssh {user}@{raspberry-pi-host}
```
Make the utility executable:
```
chmod +x {target-directory}/solace-iot-device-cert-upload
```


Run this command:
```
./solace-iot-device-cert-upload {target-directory}/{device-id}.der.key {target-directory}/{device-id}.der.crt {target-directory}/digicert-ca.der.crt
```
