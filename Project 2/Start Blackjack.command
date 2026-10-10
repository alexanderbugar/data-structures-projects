#!/bin/sh
# Finder opens this launcher in Terminal.
cd "$(dirname "$0")" || exit 1
if [ ! -x ./blackjack ]; then
    printf 'Build the source copy first with: sh build.sh\n'
    read -r answer
    exit 1
fi
exec ./blackjack
