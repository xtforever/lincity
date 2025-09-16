#!/bin/sh
# This file is part of lincity
# Create a new module fileset from skeleton files

[[ -z $1 ]]  && { echo "modulname please" ; exit 1 ; }

MY_H="$1.h"
MY_C="$1.c"

[[ -f  $MY_H ]] && { echo "$MY_H exists, cowardly running away" ; exit 1 ; }
[[ -f $MY_C  ]] && { echo "$MY_C exists, cowardly running away" ; exit 1  ; }

cat <<EOF >$MY_H
/* ---------------------------------------------------------------------- *
 * $1.h
 * This file is part of lincity.
 * ---------------------------------------------------------------------- */

#ifndef __$1_h__
#define __$1_h__
#include "modules/modules.h"

void do_$1 (int x, int y);
void mps_$1(int x, int y);
#endif /* __$1_h__ */
EOF

cat <<EOF >$MY_C
/* ---------------------------------------------------------------------- *
 * $1.h
 * This file is part of lincity.
 * ---------------------------------------------------------------------- */
#include <modules/$1.h>

void do_$1 (int x, int y) 
{

};

void mps_$1(int x, int y) 
{

};


EOF

