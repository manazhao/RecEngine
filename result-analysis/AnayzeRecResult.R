# use json library to parse the recommendation result
library(rjson)
setwd("~/git/RecEngine/result-analysis")

analyze.user.rec <- function(user.json.file, entity.latent){
  user.rec.obj <- fromJSON(file = user.json.file)
  # get user latent
  user.latent <- entity.latent[as.integer(user.rec.obj$userId) + 1,]
  user.rec.obj$latent <- user.latent
  user.features <- user.rec.obj$features
  user.rec.obj$feature.lat <- as.matrix(entity.latent[as.integer(user.rec.obj$features)+1,])
  user.rec.obj$rec.lat <- as.matrix(entity.latent[as.integer(user.rec.obj$recList)+1,])
  user.rec.obj
}


# entity profile
# entity.latent.file <- "amazon-result/rec/HHMF-d_5-f_1.latent.txt"
# entity.latent <- read.csv(file = entity.latent.file, sep=",", header=F)

# read user recommendation
user.rec.file <- "amazon-result/rec/HHMF-d_5-f_1-entity-testuser-gd_male-ag_30.json"
user.rec.obj <- analyze.user.rec(user.rec.file,entity.latent)

