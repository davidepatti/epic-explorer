#!/bin/bash

# DO NOT CHANGE these paths unless you're sure of what you're doing.
# The paths below should be coherent with the ones specified into 
# Epic Eplorer source code and Trimaran sources.

BASE_DIR=$HOME
WORKSPACE=$BASE_DIR/trimaran-workspace
TRIMARAN_ROOT=$BASE_DIR/trimaran
EPIC_SOURCE=$BASE_DIR/epic

# checking trimaran installation ###############################
echo ""
echo "** Checking for trimaran installation files..."

if [ -e $TRIMARAN_ROOT/scripts ]; then
   echo "  ** Ok, found a trimaran 4 installation, copying new tcc script..."
   mv $TRIMARAN_ROOT/scripts/tcc $TRIMARAN_ROOT/scripts/tcc.old
   cp $EPIC_SOURCE/MISC_FILES/tcc $TRIMARAN_ROOT/scripts
else
   echo "  > Missing $TRIMARAN_ROOT/scripts directory !"
   echo "    check your trimaran installation!"
   exit 1
fi


# checking epic workspace ######################################
echo "** Checking for necessary files in $WORKSPACE..."

if [ -e $WORKSPACE/epic-explorer ]; then
   echo "  ** Ok, found $WORKSPACE/epic-explorer directory"
else # missing workspace
   echo " > Creating $WORKSPACE/epic-explorer directory"
    mkdir -p $WORKSPACE/epic-explorer
fi

if [ ! -e "$WORKSPACE/epic-explorer/m5elements" ]; then
    echo " > Copying m5elements configuration files..."
    cp -R $EPIC_SOURCE/MISC_FILES/m5elements $WORKSPACE/epic-explorer/
    else
    echo "  ** Ok, found $WORKSPACE/epic-explorer/m5elements"
fi

if [ ! -e "$WORKSPACE/epic-explorer/SUBSPACES" ]; then
    echo " > Creating $WORKSPACE/epic-explorer/SUBSPACES directory and copying files..."
    cp -R $EPIC_SOURCE/SUBSPACES $WORKSPACE/epic-explorer
    else
    echo "  ** Ok, found $WORKSPACE/epic-explorer/SUBSPACES"
fi

# checking machines dir ##########################################
if [ -e $WORKSPACE/epic-explorer/machines ]; then
    echo "  ** Ok, found $WORKSPACE/epic-explorer/machines"
else
    echo " > Creating $WORKSPACE/epic-explorer/machines directory..."
    cp -R $EPIC_SOURCE/MISC_FILES/machines $WORKSPACE/epic-explorer/
fi

# checking step_by_step dir ######################################
if [ -e $WORKSPACE/epic-explorer/step_by_step ]; then
    echo "  ** Ok, Found $WORKSPACE/epic-explorer/step_by_step"
else
    echo " > Creating $WORKSPACE/epic-explorer/step_by_step directory..."
    mkdir $WORKSPACE/epic-explorer/step_by_step
fi

cp -R $EPIC_SOURCE/MISC_FILES/machines $WORKSPACE/epic-explorer/step_by_step
cp -R $EPIC_SOURCE/MISC_FILES/m5elements $WORKSPACE/epic-explorer/step_by_step 

if [ -e "$WORKSPACE/epic-explorer/epic_default.conf" ]; then
    echo " ** Warning: Found a previous epic_default.conf, saved as epic_default-previous.conf"
    mv $WORKSPACE/epic-explorer/epic_default.conf $WORKSPACE/epic-explorer/epic_default-previous.conf
fi

echo " > Creating a new epic_default.conf"
cp -f $EPIC_SOURCE/MISC_FILES/epic_default.conf $WORKSPACE/epic-explorer

echo " ** Done. **"
echo " Just type ./epic to start epic explorer!"
echo ""

