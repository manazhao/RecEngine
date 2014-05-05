
# analyze the latent vector of content features
setwd("~/git/RecEngine/result-analysis")

# load the entity latent file
entity.latent.file <- "movielens-result/HHMF-d_10-f_1.latent.txt"
entity.latent <- read.csv(file = entity.latent.file, sep=",", header=F)

# load the feature dictionary
feat.file <- "movielens-result/query.features.csv"
feat.id.map <- read.csv(file = feat.file, header = F)

# make query for each of the features
feat.lat <- entity.latent[feat.id.map[,2] + 1, ]
rownames(feat.lat) <- feat.id.map[,1]