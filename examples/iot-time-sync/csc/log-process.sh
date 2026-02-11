#!/bin/sh
sed -i -e '1,/##### BEGIN/d' -e '/##### END/d' $1
