#!/bin/bash

if lszcrypt | grep -q -e "CEX.C.*online"; then
	ICAPATH=1 ./ecdh_test
else
	# Show output in log file for debugging
	lszcrypt
	exit 77
fi
