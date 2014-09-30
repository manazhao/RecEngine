#!/bin/bash

USER_ID=$1

grep -P "^$USER_ID," /data/jian-data/shop-data/processed/mymedialite/train.csv |cut -f2- -d,|tr ',' '\n'|head -200 |cut -f1 -d: |xargs -I {} grep -P \"{}\" /data/jian-data/shop-data/processed/item.json  > $USER_ID.train

grep -P "^$USER_ID," /data/jian-data/shop-data/processed/mymedialite/test.csv |cut -f2- -d,|tr ',' '\n'|head -200 |cut -f1 -d: |xargs -I {} grep -P \"{}\" /data/jian-data/shop-data/processed/item.json  > $USER_ID.test

grep -P "^$USER_ID," /data/jian-data/shop-data/processed/mymedialite/BPRMF/prediction_200.csv |cut -f2- -d,|tr ',' '\n'|head -200 |cut -f1 -d: |xargs -I {} grep -P \"{}\" /data/jian-data/shop-data/processed/item.json  > $USER_ID.pred
