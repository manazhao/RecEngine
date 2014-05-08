
# analyze the latent vector of content features
setwd("~/git/RecEngine/result-analysis")

# load the entity latent file
result.root <- "amazon-result-f1"
model.file <- "HHMF-d_5-f_1.latent.txt"
entity.latent.file <- paste(c(result.root, model.file),collapse="/")
entity.latent <- read.csv(file = entity.latent.file, sep=",", header=F)

# load the feature dictionary
feat.file <- "query.features.csv"
feat.file <- paste(c(result.root,feat.file),collapse="/")
feat.id.map <- read.csv(file = feat.file, header = F)

# make query for each of the features
feat.lat <- entity.latent[feat.id.map[,2] + 1, ]
rownames(feat.lat) <- feat.id.map[,1]