from opencc import OpenCC
cc = OpenCC('t2s')
with open("shupin.dict.yaml", "r") as input_file, open("shupin.simp.dict.yaml", "w+") as output_file:
    for line in input_file:
        output_file.write(cc.convert(line))
