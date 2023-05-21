from collections import defaultdict
import csv

import re

cendu_pr_rules = [
    # 特字部分
    (re.compile(r"Ddi"), "le"),    # 的字讀音
    (re.compile(r"Dqu4"), "qie4"),  # 去字讀音
    (re.compile(r"Dlu5"), "liu2"),  # 六字讀音
    # 通入部分
    (re.compile(r"^su5$"), "sv5"),  # 成都特殊设定・速俗v/io两可
    # 入聲派向部分
    (re.compile(r"5$"), "2"),
    # 戈歌部分
    (re.compile(r"eo([1234])$"), r"o\1"),  # 不分舒聲戈歌
    (re.compile(r"eo5"), "o5"),  # 不分入聲戈歌
    # 尖團部分
    (re.compile(r"^zyi"), "ji"),  # zyi>ji (挤)
    (re.compile(r"^cyi"), "qi"),  # cyi>qi (七)
    (re.compile(r"^syi"), "xi"),  # syi>xi (西)
    (re.compile(r"^zi([a-z]+)"), r"ji\1"),  # zi->ji- (尖)
    (re.compile(r"^ci([a-z]+)"), r"qi\1"),  # ci->qi- (雀)
    (re.compile(r"^si([a-z]+)"), r"xi\1"),  # si->xi- (仙)
    (re.compile(r"^zv"), "ju"),  # zü>ju, zü->ju- (聚)
    (re.compile(r"^cv"), "qu"),  # cü>qu, cü->qu- (趣)
    (re.compile(r"^sv"), "xu"),  # sü>xu, sü->xu- (须)
    # 平翹部分
    (re.compile(r"^([zcs])h"), r"\1"),  # 無翹舌
    # h、f部分
    (re.compile(r"^hu([12345])$"), r"fu\1"),  # hu>fu
    # 成都特字
    (re.compile(r"C"), ""),
]

def apply_regex_rules(pr, rules):
    for regex, replacement in rules:
        pr = regex.sub(replacement, pr)
    return pr

def convert_to_cendu_pr(pr):
    return apply_regex_rules(pr, cendu_pr_rules)

chars2prs = defaultdict(list)
freqs = {}

with open("shupin.simp.dict.yaml", "r") as input_file:
    for line in input_file.readlines():
        if "\t" in line:
            [char, pr] = line.split("\t")[:2]
            if len(char) == 1:
                pr = pr.split("◎")[0].strip()
                pr = convert_to_cendu_pr(pr)
                if pr.startswith("R") or pr.startswith("X"):
                    continue
                chars2prs[char].append(pr)

prs2chars = defaultdict(list)

for c, prs in chars2prs.items():
    if len(prs) == 1:
        prs2chars[prs[0]].append(c)


with open("char_freq.csv", "r") as input_file:
    reader = csv.reader(input_file)
    # skip header
    next(reader, None)
    for row in reader:
        freqs[row[1]] = int(row[2])


all_prs = set(pr for prs in chars2prs.values() for pr in prs)
print("Number of prs: {}".format(len(all_prs)))

with open("missing_pr_chars.txt", "w+") as output_file:
    for pr in all_prs - set(prs2chars.keys()):
        output_file.write(f"{pr}\n")

with open("pr_chars.txt", "w+") as output_file:
    for pr, chars in prs2chars.items():
        c = sorted(chars, key=lambda c: freqs.get(c, 0), reverse=True)[0]
        output_file.write(f"{c}\t{pr}\n")
