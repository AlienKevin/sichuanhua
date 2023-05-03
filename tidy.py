import re
from typing import List, Tuple
from dataclasses import dataclass
import json

pinyin_pattern = re.compile(r'([a-z]+ *\d *→?(——)?,? *)+')
numeric_bullet_pattern = re.compile(r'[1-9][0-9]*\.')
definition_bullet_pattern = re.compile(r'①|②|③|④|⑤|⑥|⑦|⑧|⑨|⑩')

dictionary = {}

@dataclass
class Entry:
    word: str
    pinyin: str
    definitions: Tuple[(str, List[str])]

with open("fangyan.txt", "r") as file:
    sections = file.read().split("\n\n")
    for section in sections:
        entries = re.split('\s{4,}', section)
        section_name = entries[0].strip()
        section_name = section_name[numeric_bullet_pattern.search(section_name).end():].strip()
        section_entries = []
        for entry in entries[1:]:
            pinyin_match = pinyin_pattern.search(entry)
            if pinyin_match is None:
                print(entry)
            word = entry[:pinyin_match.start()].strip()
            pinyin = pinyin_match.group(0).replace(" ", "")
            definition = entry[pinyin_match.end():].strip()
            if "①" in definition:
                definitions = definition_bullet_pattern.split(definition)
            else:
                definitions = [definition]
            result_definitions = []
            for definition in definitions:
                if definition == "":
                    continue
                if ":" in definition:
                    if len(definition.split(":")) != 2:
                        print(definition.split(":"))
                    definition, examples = definition.split(":")
                    definition = definition.strip()
                    examples = [example.strip() for example in examples.split("|")]
                else:
                    examples = []
                result_definitions.append([definition, examples])
            section_entries.append(Entry(word, pinyin, result_definitions))
        dictionary[section_name] = section_entries

class EntryEncoder(json.JSONEncoder):
    def default(self, obj):
        if isinstance(obj, Entry):
            return obj.__dict__
        return super().default(obj)

with open("fangyan.json", "w+") as file:
    json.dump(dictionary, file, ensure_ascii=False, indent=2, cls=EntryEncoder)
