# Example App for Ledger Nano S

This is the Application for Example hardware wallet Ledger Nano S version.

See [Ledger's documentation](http://ledger.readthedocs.io) to get started.

## Explanation

Read my articles explaining my journey in creating this Ledger App:
https://medium.com/@ahmad.ali.abdilah/developing-ledger-application-part-1-setting-up-development-environment-c218460fe14e

## How to start

1. Download suitable docker image

2. Follow documentation on the ledger website to setup the docker https://developers.ledger.com/docs/nano-app/introduction/

3. Running docker with shared folder

```c
docker run -v /path/in/the/laptop:/path/to/connect/in/docker/container -ti docker/image
```

4. Compiling and deploying the app to device

```bash
python -m ledgerblue.loadApp --targetId 0x31100002 --apdu --fileName app.hex --appName "Example Test" --appFlags 0x00
```

4. Deleting an app

```bash
python3 -m ledgerblue.deleteApp --targetId 0x31100002 --appName "Example Test"
```
