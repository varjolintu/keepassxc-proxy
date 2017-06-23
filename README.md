# keepassxc-proxy
Application that works as a proxy between Native Messaging browser extension and KeePassXC

This is still heavily under development.

The main version is written with Qt but there's also a Python version available.
keepassxc-proxy listens stdin from keepassxc-browser extension and transfers the data to UDP port 19700 which KeePassXC listens.
