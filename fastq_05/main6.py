import os

class FastQ:
    def __init__(this, name, seq, qual):
        this.name = name
        this.seq = seq
        this.qual = qual

list = []

with open("test.fastq", "r") as fp:
    while True:
        name = fp.readline().rstrip(os.linesep)
        if name == "":
            break
        seq = fp.readline().rstrip(os.linesep)
        dummy = fp.readline().rstrip(os.linesep)
        qual = fp.readline().rstrip(os.linesep)
        list.append(FastQ(name, seq, qual))

for x in list:
    print(x.name)
    print(x.seq)
    print("+")
    print(x.qual)

