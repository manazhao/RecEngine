library(rjson)

data.root <- '/home/qzhao2/git/RecEngine/result-analysis/amazon-result-bias'

# load bias file
item.bias.file <- 'amazon-BB-d_5-f_1.item.bias.csv'
user.bias.file <- 'amazon-BB-d_5-f_1.user.bias.csv'
item.bias <- read.csv(file = paste(data.root,item.bias.file,sep='/'), header=F)
user.bias <- read.csv(file = paste(data.root,user.bias.file,sep='/'), header=F)
item.bias.order <- item.bias[order(-item.bias[,2]),]
# load user json
user.rec.file <- 'amazon-BB-d_5-f_1-user-test_amazon_user_0.json'
user.rec.obj <- fromJSON(file=paste(data.root,user.rec.file,sep="/"))
# get the recommended item ids
user.rec.ids <- as.integer(user.rec.obj$recIdList)
# get the item bias informationv
rec.item.bias <- item.bias.order[item.bias.order[,1] %in% user.rec.ids,]
rec.item.bias.var <- rec.item.bias
rec.item.bias.var[,3] <- rec.item.bias.var[,3] - rec.item.bias.var[,2]^2