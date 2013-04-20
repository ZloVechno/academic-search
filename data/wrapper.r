in_file <- file('./input.txt') 
open(in_file)
save_list <- list()

j <-0
while ((length(line <- readLines(in_file, n = 1, warn = FALSE)) > 0)) {
  save_list <- c(save_list,line)
  j <- j+1
} 
close(in_file)
author_num=length(save_list)

file.create("./output.txt")
out_file <- file('./output.txt') 
open(out_file)

#for (i in 1:author_num)
#{
  i=1
  res_vec=GScholar_Scraper (save_list[i], since=1945, write=T, citation=1)
  new_res_vec=array(res_vec)
  num=res_vec[1]
final_vec=0
if (num>0)
{
  dim=(dim(new_res_vec)-1)/2
  years=res_vec[2:(dim+1)]
  years=years[years>1945]
  citations=res_vec[(dim+2):(2*dim+1)]
  #print(citations)
 
h_index <-0
while (h_index<dim)
{
  if (citations[h_index+1]>h_index)
  {
    h_index <- h_index+1
  }
  else
  {
    break
  }
}

cit_num=min(num,40)

if_little=0
#print(h_index)
h_res=h_index
if ((h_index>=40)&&(num>40))
{
  if_little=1
  y=log(citations)
  help_x=1:40
  x=cbind(sign(help_x),log(help_x))
  theta=solve(t(x)%*%x)%*%t(x)%*%y
  h_tilde=exp(theta[1]/(1-theta[2]))
  h_res=round((2*h_tilde+40)/3)
  approx=exp(x%*%theta)
}

years_in_science=2013-min(years)
if(if_little==0)
{
  final_list=list(num,cit_num,if_little,h_res, years_in_science, citations)
  final_vec=unlist(final_list)
}
if (if_little==1)
{
  final_list=list(num,cit_num,if_little,h_res, years_in_science, citations,approx)
  final_vec=unlist(final_list)
}

  ##print("res_vec")
  ##print(new_res_vec)
}
  lapply(final_vec, write, "./output.txt", append=TRUE)
#}
close(out_file)
