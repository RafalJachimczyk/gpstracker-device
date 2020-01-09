#!/bin/sh
cd src/datatypes
protoc --plugin=protoc-gen-nanopb=/Users/jachir01/workspace/nanopb/generator/protoc-gen-nanopb --nanopb_out=. *.proto
mv SpatialTelemetry.pb.* ../