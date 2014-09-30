#!/bin/bash
#
# extract category item popularity for a given item
#

# take item id from standard input
filter_result=`grep -P "i_$1" '/home/qzhao2/Dropbox/data/amazon_book_rating/popularity_result/item_merge_profile.csv' |perl FilterFeature.pl --pat="i_cip_" --dict='/home/qzhao2/Dropbox/data/amazon_book_rating/feature_dict.csv'|perl ExtractFeature.pl `
# take out the weight
echo $filter_result
feat_weight=`echo $filter_result | cut -f2 -d,`
# get the category names
categories=`echo $filter_result|cut -f1 -d, |xargs -I {} grep -P ",{}$" '/home/qzhao2/Dropbox/data/amazon_book_rating/feature_dict.csv' |cut -f1 -d, |cut -f3 -d'_'|xargs -I {} grep -P "^{}-" '/home/qzhao2/Dropbox/data/amazon_book_rating/cat_path.csv'`

# join the category name and id
echo $categories
echo $feat_weight

