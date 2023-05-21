#!/bin/sh

WDIR=$(dirname "$0")

find "$WDIR" -type f | \
grep -vE "^\.$|\.git|\.pio|\.vscode|.*tft$|.*png$|.*jfif$|.*jpg$|.*HMI$|.*md$|.*README.*|.*LICENSE.*|\.DS_Store$|.*/web-interface/.*"
