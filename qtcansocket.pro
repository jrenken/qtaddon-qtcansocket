!linux-* {
    error("SocketCan is only available on Linux systems!")
}

TEMPLATE = subdirs
SUBDIRS = src examples
CONFIG += ordered

