# Simple Client/Server

This is an experiment with implementing simple client/server applications for Windows and Linux. Currently there are a few bugs.

## Issues

- On Linux: the server sends a string and nothing is ever received by the client.
- On Windows: both the client and server throw bad file descriptor errors when they try to read.
