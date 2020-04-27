#!/bin/bash

#--------------------------------------------------------------------
# File Name: test.sh
# Description:
# Author: Dan.Cao <caodan@linuxtoy.cn>
#--------------------------------------------------------------------

INPUT_FILE="test_data/imx378_RGGB_raw10_1920x1080.raw"
WIDTH=1920
HEIGHT=1080
FORMAT=RAW10_UNPACKED
CFA=RGGB

./test_demosaic -i ${INPUT_FILE} -o ${CFA}_${WIDTH}x${HEIGHT}.rgb -w ${WIDTH} -v ${HEIGHT} --format=${FORMAT} --cfa=${CFA}
