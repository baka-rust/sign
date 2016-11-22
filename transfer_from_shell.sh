#!/bin/bash

source_dir=/home/warehouse/ejvaughan/linux
rsync -a wustl:$source_dir/modules/lib .
rsync -a wustl:$source_dir/arch/arm/boot .
rsync -a wustl:$source_dir/scripts/mkknlimg .
