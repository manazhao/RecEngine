# visualize the training, testing and coldstart rmse under various parameter combination

# use this library for string match
library(stringr)

setwd("~/git/RecEngine/result-analysis")
result.dir <- "amazon-result"
result.files <- list.files(result.dir)
setwd(result.dir)
setEPS()
for(lp in 1:length(result.files)){
  result.file <- result.files[lp]
  model.params <- str_match(result.file,"dim_(\\d+)\\-(\\d)")
  num.dim <- model.params[2]
  use.feature <- model.params[3]
  # read the rmse and visualize
  rmse.values <- read.csv(file=result.file,header=F,sep="\t")
  colnames(rmse.values) <- c("train","test","cs")
  param.str <- paste(c(num.dim,use.feature),collapse="-")
  postscript(paste(c(param.str,".eps"),collapse=""))
  plot(rmse.values$train,type="l",pch=0,col="red",lty=1,xlim=c(0,50),ylim=c(0.75,1.07),xlab="iteration",ylab="rmse",)
  title(main=paste(c("dimension: ",num.dim, " ,use-feature: ",use.feature),collapse=" "))
  lines(rmse.values$test,pch=1,type="l",col="green",lty=2)
  lines(rmse.values$cs,pch=2,type="l",col="blue",lty=3)
  legend(39,1.07,c("train","test","cs"),lty=c(1,2,3),col=c("red","green","blue"))
  dev.off();
}