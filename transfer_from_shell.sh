#!/bin/bash

source_dir=/home/warehouse/ejvaughan/linux
rsync -r wustl:$source_dir/modules/lib .
rsync -r wustl:$source_dir/arch/arm/boot .
rsync -r wustl:$source_dir/scripts/mkknlimg .
