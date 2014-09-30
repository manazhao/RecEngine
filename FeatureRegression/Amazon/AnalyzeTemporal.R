# analyze temporal patter of products

# plot product popularity trend w.r.t year
analyze.product <- function(){
  # ipy stands for : item popularity year
  ipy.file <- "/home/qzhao2/Dropbox/data/amazon_book_rating/frequency/ipy_0694524751.csv"
  # ipy has two columns which represent year and number of reviews (frequence)
  ipy <- read.csv(ipy.file,header=F)
  # order the frequency by year
  ipy.year <- seq(1,dim(ipy)[1])
  for (i in 1:dim(ipy)[1]){
    tmp.year <- strsplit(toString(ipy[i,1]),"_")[[1]][3];
    # convert to integer
    ipy.year[i] <- as.integer(tmp.year)
  }
  # order by year in increasing order
  ipy.sort <- ipy[order(ipy.year),]
  # dump to console
  ipy.sort
}

analyze.product()