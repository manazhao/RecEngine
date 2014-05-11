library(rjson)

data.root <- '/home/manazhao/git/RecEngine/result-analysis/movielens-f3'

# load bias file
item.bias.file <- 'movielens-BB-d_10-f_0.item.bias.csv'
item.bias <- read.csv(file = paste(data.root,item.bias.file,sep='/'), header=F)
item.bias.order <- item.bias[order(-item.bias[,2]),]
# load user json
user.rec.file <- 'movielens-BB-d_10-f_0-user-test_fake_user_0.json'
user.rec.obj <- fromJSON(file=paste(data.root,user.rec.file,sep="/"))
# get the recommended item ids
user.rec.ids <- as.integer(user.rec.obj$recIdList)
# get the item bias information
rec.item.bias <- item.bias.order[item.bias.order[,1] %in% user.rec.ids,]
rec.item.bias.var <- rec.item.bias
rec.item.bias.var[,3] <- rec.item.bias.var[,3] - rec.item.bias.var[,2]^2