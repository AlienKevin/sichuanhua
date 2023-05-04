from opencc import OpenCC
cc = OpenCC('t2s')
with open("shupin.dict.yaml", "r") as input_file, open("shupin.simp.dict.yaml", "w+") as output_file:
    prev_lines = set()
    for line in input_file:
        curr_line = cc.convert(line)
        # remove duplicate lines due to multiple traditional mapped to a single simplified
        if not curr_line in prev_lines:
            output_file.write(curr_line)
            prev_lines.add(curr_line)
