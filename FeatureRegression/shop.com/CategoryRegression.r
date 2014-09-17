# product category regression

library(Matrix)
library(glmnet)

feature.file <- '/data/jian-data/shop-data/mlr/orderdata.gt5.lt200.train.glmnet.x.csv'
response.file <- '/data/jian-data/shop-data/mlr/orderdata.gt5.lt200.train.glmnet.y.csv'
# sparse matrix which is in the format i,j,v
print(sprintf("load features from: %s",feature.file));
sm.ijv <- read.csv(file=feature.file,header=F, sep=",")
# remove the offset of the features
min.feature.idx <- min(sm.ijv[,2])
sm.ijv[,2] <- sm.ijv[,2] - min.feature.idx + 1
# construct sparse matrix out of the feature
print(sprintf("construct sparsematrix"));
feature.sm <- sparseMatrix(i=sm.ijv[,1], j=sm.ijv[,2], x=sm.ijv[,3])
# load response value
print(sprintf("load response variables from: %s",response.file))
y <- as.integer(as.matrix(read.csv(file=response.file, header=F)))
num.samples <- length(y)
# use part of the training samples
sample.rnd.idx <- sample(1:num.samples,replace=F)
sample.rnd.size <- as.integer(num.samples * 0.2)
train.selected.idx <- sample.rnd.idx[1:sample.rnd.size]


# runt he glmnet for multinomial logistic regression
print("running multinomial regression")
fit.result <- cv.glmnet(x = feature.sm, y = y, family = "multinomial")
# save space
save(file="fit.result", fit.result)

