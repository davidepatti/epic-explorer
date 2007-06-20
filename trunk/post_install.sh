#!/bin/bash

# DO NOT CHANGE these paths unless you're sure of what you're doing.
# The paths below should be coherent with the ones specified into 
# Epic Eplorer source code and Trimaran sources.

BASE_DIR=$HOME
WORKSPACE=$BASE_DIR/trimaran-workspace
TRIMARAN_ROOT=$BASE_DIR/trimaran
EPIC_SOURCE=$BASE_DIR/epic
HMDES_FILE=hpl_pd_elcor_std.hmdes2

echo "Checking for trimaran installation files"

if [ -e $TRIMARAN_ROOT/bin ]; then
   echo "     ok, found trimaran installation dir, copying new tcc script..."
   cp $EPIC_SOURCE/MISC_FILES/tcc $TRIMARAN_ROOT/bin
else
   echo "     Missing $TRIMARAN_ROOT/bin directory !"
   echo "     check your trimaran installation"
fi


echo "Checking for necessary files in $WORKSPACE..."
if [ -e $WORKSPACE ]; then
    if [ -e $WORKSPACE/epic-explorer ]; then
	echo "     Ok, found $WORKSPACE/epic-explorer directory"
	else
	echo "     Creating $WORKSPACE/epic-explorer directory"
	mkdir $WORKSPACE/epic-explorer
    fi
else # missing workspace
    echo "      Can't find a valid trimaran-workspace directory in $BASE_DIR"
    echo "      Please run ./gui/trimaran-gui from Trimaran source code at"
    echo "      least once."
    exit 0
fi

if [ ! -e "$WORKSPACE/cache.cfg" ]; then
    echo "Copying cache.cfg "
    cp $EPIC_SOURCE/MISC_FILES/cache.cfg $WORKSPACE
    else
    echo "     Ok, found $WORKSPACE/cache.cfg"
fi

if [ ! -e "$WORKSPACE/epic-explorer/SUBSPACES" ]; then
    echo "Creating $WORKSPACE/epic-explorer/SUBSPACES directory"
    cp -R $EPIC_SOURCE/SUBSPACES $WORKSPACE/epic-explorer
    else
    echo "     Ok, found $WORKSPACE/epic-explorer/SUBSPACES"
fi

if [ -e $WORKSPACE/machines/$HMDES_FILE ]; then
    echo "Warning: Found a previous $HMDES_FILE, saving as $HMDES_FILE.bak"
    mv $WORKSPACE/machines/$HMDES_FILE $WORKSPACE/machines/$HMDES_FILE.bak
fi
    echo "Creating $HMDES_FILE main file"
    cp -f $EPIC_SOURCE/MISC_FILES/hmdes/$HMDES_FILE $WORKSPACE/machines
    cp -f $EPIC_SOURCE/MISC_FILES/hmdes/.hpl_pd_pristine.hmdes2 $WORKSPACE/machines
    cp -f $EPIC_SOURCE/MISC_FILES/hmdes/explorer.hmdes2 $WORKSPACE/machines

if [ -e "$WORKSPACE/epic-explorer/epic_default.conf" ]; then
    echo "Warning: Found a previous epic_default.conf, saved as epic_default-previous.conf"
    mv $WORKSPACE/epic-explorer/epic_default.conf $WORKSPACE/epic-explorer/epic_default-previous.conf
fi

echo "Creating a new epic_default.conf"
cp -f $EPIC_SOURCE/MISC_FILES/epic_default.conf $WORKSPACE/epic-explorer

echo "Done. Just type ./epic to start Epic Explorer !"

