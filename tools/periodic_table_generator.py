import json
from sys import argv

def print_element(element, table2):
    print("\t\t{")
    print("\t\t\t\"" + element["name"] + "\",")
    print("\t\t\t\"" + element["symbol"] + "\",")
    print(f"\t\t\t\"{element['valences']}\",")
    print(f"\t\t\t\"{table2['elements'][element['number'] - 1]['electronicConfiguration']}\",")

    print("\t\t\t" + (str(element["melt"]) if element["melt"] != None else "NAN") + ",")
    print("\t\t\t" + (str(element["boil"]) if element["boil"] != None else "NAN") + ",")
    print(f"\t\t\t{str(element['density']) if element['density'] != None else 'NAN'},")
    print("\t\t\t" + (str(element["electronegativity_pauling"]) if element["electronegativity_pauling"] != None else "NAN") + ",")
    print("\t\t\t" + (str(element["electron_affinity"]) if element["electron_affinity"] != None else "NAN") + ",")
    print("\t\t\t" + str(element["atomic_mass"]) + ",")

    print("\t\t\t" + str(element["number"]) + ",")
    print(f"\t\t\t{round(element['atomic_mass']) - element['number']}" + ",")

    print("\t\t\t" + str(element["xpos"]) + ",")
    if element["name"].__eq__("Oxygen"):
        print("\t\t\tNONMETAL,")
    else:
        print("\t\t\t" + str(categories.get(element["category"], "UNKNOWN")) + ",")
    print("\t\t},")

with open(argv[1], "r") as table_json, open(argv[2], "r") as table_json2:
    table = json.load(table_json)
    table2 = json.load(table_json2)

    categories = {
        "alkali metal": "ALKALI_METAL",
        "alkaline earth metal": "ALKALINE_EARTH_METAL",
        "transition metal": "TRANSITION_METAL",
        "post-transition metal": "POST_TRANSITION_METAL",
        "metalloid": "METALLOID",
        "polyatomic nonmetal": "NONMETAL",
        "diatomic nonmetal": "HALOGEN",
        "noble gas": "NOBLE_GAS",
        "lanthanide": "LANTHANIDE",
        "actinide": "ACTINIDE",
    }
    
    for i in range(1, 8):
        print(f"\tconst Element PERIOD_{i}_ELEMENTS[] = {{")
        for element in table["elements"]:
            if element["ypos"] == i and element["category"] != "lanthanide" and element["category"] != "actinide":
                print_element(element, table2)
        print("\t};\n")
    
    print("\tconst Element LANTHANIDES[] = {")
    for element in table["elements"]:
        if element["category"] == "lanthanide":
            print_element(element, table2)
    print("\t};\n")

    print("\tconst Element ACTINIDES[] = {")
    for element in table["elements"]:
        if element["category"] == "actinide":
            print_element(element, table2)
    print("\t};\n")
