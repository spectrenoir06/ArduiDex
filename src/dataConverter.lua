--input = io.open ("data.csv" ,"r")

i = 0
for line in io.lines("data.csv") do
	print(line)
	file = io.open(i..".dat","w+")
	file:write(line)
	i=i+1
end