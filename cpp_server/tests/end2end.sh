#!/bin/bash

build/geeky &
pid=$?
nc localhost 8081

kill $pid
