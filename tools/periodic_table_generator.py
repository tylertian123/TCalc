import json
from sys import argv

with open(argv[1], "r") as table_json:
    table = json.load(table_json)
    
    for element in table["elements"]:
        print("{")
        print("\t\"" + str(element["name"]) + "\",")
        print("\t\"" + str(element["symbol"]) + "\",")
        print("\t" + str(element["number"]) + ",")
        print(f"\t{round(element['atomic_mass']) - element['number']}" + ",")
        print("\t" + str(element["atomic_mass"]) + ",")
        print("\t" + str(element["xpos"]) + ",")
        print("},")
