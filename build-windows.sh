#!/bin/sh

docker build -t ngn-windows-builder .
docker run --name=builder ngn-windows-builder &

sleep 1

docker cp builder:/build/ngn.exe .
docker kill builder
