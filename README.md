# keepassxc-proxy
Application that works as a proxy between Native Messaging browser extension and KeePassXC

This is still heavily under development.

keepassxc-proxy listens stdin from keepassxc-browser extension and transfers the data to Unix domain socket `/tmp/kpxc_server` which KeePassXC listens.
Under Windows this is a named pipe with the name `kpxc_server`.

To build the C++ version download the sources and just run `cmake -DCMAKE_BUILD_TYPE=Release .` and make sure you have `boost_system` library installed.
After that `make` should compile the actual proxy application.

Windows users should use the Qt version. Python version is currently missing the latest features.

If you have already installed the JSON script from KeePassXC's settings you can just change the path of the executable from the JSON file to the proxy application.
