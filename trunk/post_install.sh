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
   cp $EPIC_SOURCE/MISC_FILES/tcc $TRIMARAN_ROOT/scripts
else
   echo "  > Missing $TRIMARAN_ROOT/scripts directory !"
   echo "    check your trimaran installation!"
   exit 1
fi


# checking epic workspace ######################################
echo "** Checking for necessary files in $WORKSPACE..."

if [ -e $WORKSPACE/epic-explorer ]; then
   echo "  ** WARNING: found a previous $WORKSPACE/epic-explorer directory"
   echo "It's strongly suggested to move/rename it in order to avoid conflicts with "
   echo "newer versions."
   exit -1
else # missing workspace
   echo " > Creating $WORKSPACE/epic-explorer directory"
    mkdir -p $WORKSPACE/epic-explorer
fi

echo " > Copying SUBSPACES directory..."
cp -Rf $EPIC_SOURCE/SUBSPACES $WORKSPACE/epic-explorer

echo " > Copying m5elements configuration files..."
cp -Rf $EPIC_SOURCE/MISC_FILES/m5elements $WORKSPACE/epic-explorer/

echo " > Creating $WORKSPACE/epic-explorer/machines directory..."
cp -Rf $EPIC_SOURCE/MISC_FILES/machines $WORKSPACE/epic-explorer/

echo " > Creating $WORKSPACE/epic-explorer/step_by_step directory..."
mkdir -p $WORKSPACE/epic-explorer/step_by_step

cp -Rf $EPIC_SOURCE/MISC_FILES/machines $WORKSPACE/epic-explorer/step_by_step
cp -Rf $EPIC_SOURCE/MISC_FILES/m5elements $WORKSPACE/epic-explorer/step_by_step 

echo " > Creating a new epic_default.conf"
cp -f $EPIC_SOURCE/MISC_FILES/epic_default.conf $WORKSPACE/epic-explorer

echo " ** Done. **"
echo " Just type ./epic to start epic explorer!"
echo ""

