#!/bin/bash

#--------------------------------------------------------------------
# File Name: test.sh
# Description:
# Author: Dan.Cao <caodan@linuxtoy.cn>
#--------------------------------------------------------------------

INPUT_FILE="../imx378_raw10_1920x1080.raw"
WIDTH=1920
HEIGHT=1080
FORMAT=RAW10_UNPACKED

#INPUT_FILE="../1DSM3_14bit_6144x3160.raw"
#WIDTH=6144
#HEIGHT=3160
#FORMAT=RAW14_UNPACKED

./test_demosaic -i ${INPUT_FILE} -o RGGB_${WIDTH}x${HEIGHT}.rgb -w ${WIDTH} -v ${HEIGHT} --format=${FORMAT} --cfa=RGGB
./test_demosaic -i ${INPUT_FILE} -o GRBG_${WIDTH}x${HEIGHT}.rgb -w ${WIDTH} -v ${HEIGHT} --format=${FORMAT} --cfa=GRBG
./test_demosaic -i ${INPUT_FILE} -o GBRG_${WIDTH}x${HEIGHT}.rgb -w ${WIDTH} -v ${HEIGHT} --format=${FORMAT} --cfa=GBRG
./test_demosaic -i ${INPUT_FILE} -o BGGR_${WIDTH}x${HEIGHT}.rgb -w ${WIDTH} -v ${HEIGHT} --format=${FORMAT} --cfa=BGGR
