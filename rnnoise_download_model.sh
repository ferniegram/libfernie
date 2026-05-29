#!/bin/sh

wget() {
    curl -OL "$@"
}

. download_model.sh
exit $?
