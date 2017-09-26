# keepassxc-proxy
Application that works as a proxy between Native Messaging browser extension and KeePassXC

This is still heavily under development.

keepassxc-proxy listens stdin from keepassxc-browser extension and transfers the data to UDP port 19700 which KeePassXC listens.

There's a Python GUI version in the `python` folder. If you want to try semi-manual install please try the scripts from `install` folder.
If you have already installed the JSON script from KeePassXC's settings you can just change the path of the executable from the JSON file to the proxy application.
