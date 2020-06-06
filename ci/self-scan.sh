#!/usr/bin/env bash

# ...

echo "Invoking self-scan service"
echo ">> curl http://cti.xc5.io:8086/api/v2/scan/mastiff"
curl http://cti.xc5.io:8086/api/v2/scan/mastiff
echo "Finished initiating the scan."
