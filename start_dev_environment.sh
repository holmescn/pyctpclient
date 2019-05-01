#!/bin/sh
docker run -it --mount "type=bind,src=$PWD,dst=/ctp" ctp:dev /bin/bash
