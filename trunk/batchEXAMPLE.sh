#!/bin/bash

# ##################################
# ### 1. Options for all batches ###
# ##################################

# Note:
# 1. Any options can be moved down to the per batch section
# 2. These _BATCH_ options override any settings in configlocal.cmd

# If you want to override configlocal.cmd/version.cmd
# -------------------------------------------------
# export EXO_BATCH_MAJOR_VER=
# export EXO_BATCH_MINOR_VER=
# export EXO_BATCH_MICRO_VER=
# export EXO_BATCH_BUILD_VER=

# Usually export per batch in section 4.x below
# ------------------------------------------
# export BATCH_TARGET_CPU=
# export BATCH_CONFIGURATION=

# Commonly only one toolset etc installed so best defined in
# configlocal.cmd but can be defined here per batch if multiple
# tools etc are available
# -------------------------------------------------------------
# export EXO_BATCH_TOOLSET=
# export EXO_BATCH_POSTGRES_VER=
# export EXO_BATCH_BOOST_VER=
# export EXO_BATCH_BOOSTPRO=
# export EXO_BATCH_BUILD_ROOT=
# export EXO_BATCH_PROGRAMFILES_ROOT=

# ##########################################
# ### 2. Change the default batch stages ###
# ##########################################
# *=default, un-#ark the alternative if desired

# export EXO_BATCH_CLEAN=NO*
# export EXO_BATCH_CLEAN=YES

# export EXO_BATCH_MAKE=YES*
# export EXO_BATCH_MAKE=NO

# export EXO_BATCH_PACK=YES*
# export EXO_BATCH_PACK=NO

# export EXO_BATCH_UPLOAD=NO*
# export EXO_BATCH_UPLOAD=YES

# ################################
# ### 3. Standard Preparation ####
# ################################

# ----------------------------
# --- clean the batch logs ---
# ----------------------------
    test -f batch.log && rm -f batch.log
    test -f batch.err && rm -f batch.err


# ######################
# #### 4.x Do a batch ##
# ######################

    export BATCH_TARGET_CPU=x64
    export BATCH_CONFIGURATION=Release
    ./batchone.sh

# ######################
# #### 4.x Do a batch ##
# ######################

#    export BATCH_TARGET_CPU=x86
#    export BATCH_CONFIGURATION=Release
#    ./batchone.sh

# ##########################################
# #### 5. Store and/or email the results ###
# ##########################################
test -f batch.log && cat batch.log
test -f batch.err && cat batch.err
