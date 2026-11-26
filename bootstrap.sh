#!/usr/bin/env bash
set -euo pipefail

mkdir -p input output
sudo chown -R 1000:1000 input output

mkdir -p loki-data
sudo chown -R 10001:10001 loki-data

mkdir -p grafana-data
sudo chown -R 472:472 grafana-data

docker-compose up -d "$@"
