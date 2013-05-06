#!/bin/bash
if [ "$(whoami)" == "jwatts" -a "$(hostname)" = "oreme" ]; then
	export PATH="${HOME}/Source/Misc/xinu/cross-tools/local-prefix/bin:${PATH}"
else
	export PATH=~vcss544/local-prefix/bin:$PATH
fi


