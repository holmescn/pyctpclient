#!/bin/sh
docker run -it --mount "type=bind,src=$PWD,dst=/src" ctp:build /bin/bash
