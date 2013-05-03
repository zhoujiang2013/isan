#!/bin/sh
./isan.py \
    --model isan.common.perceptrons.Model \
    --decoder isan.common.decoder.Push_Down \
    --task isan.parsing.dep_unlabeled.Task \
    $*