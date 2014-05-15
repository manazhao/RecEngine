library(rjson)

data.root <- '/home/qzhao2/git/RecEngine/result-analysis/amazon-result-bias'

# load bias file
item.bias.file <- 'amazon-BB-d_5-f_1.item.bias.csv'
user.bias.file <- 'amazon-BB-d_5-f_1.user.bias.csv'
rating.file <- 'amazon-BB-d_5-f_1.rating.csv'

item.bias <- read.csv(file = paste(data.root,item.bias.file,sep='/'), header=F)
user.bias <- read.csv(file = paste(data.root,user.bias.file,sep='/'), header=F)
ratings <- read.csv(file = paste(data.root,rating.file,sep='/'), header=F)

user.bias.var <- cbind(user.bias[,1:2], user.bias[,3] - user.bias[,2]^2)
item.bias.var <- cbind(item.bias[,1:2], item.bias[,3] - item.bias[,2]^2)
colnames(user.bias.var) <- c("uid","mean","var")
colnames(item.bias.var) <- c("iid","mean","var")

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

# analyze the top ranked items
first.item.id <- rec.item.bias.var[1,1]
first.item.ratings <- ratings[ratings[,2] == first.item.id,]
first.item.user.bias <- user.bias.var[user.bias.var[,1] %in% first.item.ratings[,1],]
# get user bias
user.avg.ratings <- matrix(nrow = dim(first.item.ratings)[1], ncol = 2)
for(i in 1 : length(first.item.ratings[,1])){
  tmp.user <- first.item.ratings[i,1]
  user.avg.ratings[i,] <- c(tmp.user, mean(ratings[ratings[,1] == tmp.user,3]))
}






