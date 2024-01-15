#!/bin/bash
# Note: using /bin/bash instead of /bin/sh as dash does not support source
#first source the siso env
source ${SISODIR5}/setup-siso-env.sh

#stop any already running gs
gs stop

# now start a new gs
gs start
