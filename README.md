# keepassxc-proxy
Application that works as a proxy between Native Messaging browser extension and KeePassXC

This is still heavily under development.

keepassxc-proxy listens stdin from keepassxc-browser extension and transfers the data to UDP port 19700 which KeePassXC listens.

To use the C++ version download the sources and just run `cmake -DCMAKE_BUILD_TYPE=Release .` and make sure you have `boost_system` library installed.
After that `make` should compile the actual proxy application.

There's a Python GUI version in the `python` folder. If you want to try semi-manual install please try the scripts from `install` folder.
If you have already installed the JSON script from KeePassXC's settings you can just change the path of the executable from the JSON file to the proxy application.
