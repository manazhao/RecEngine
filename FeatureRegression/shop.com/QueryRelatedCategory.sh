#!/bin/bash
# query related categories given query category
QUERY_CATEGORY=$1
RY=$1
grep -P "^$QUERY_CATEGORY," /data/jian-data/shop-data/processed/item.sim.txt |cut -f2- -d,|tr ',' '\n'|cut -f1 -d:|xargs -I {} grep -P ",{}$" /data/jian-data/shop-data/processed/category.csv

