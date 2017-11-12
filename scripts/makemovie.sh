#!/bin/bash

DIR=$1

ffmpeg -r 20 -f image2 -s 1920x1080 -i $DIR/movie_%04d.png -vcodec libx264 -crf 25 -pix_fmt yuv420p $DIR/video.mp4
