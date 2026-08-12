import os
def getfiles(path):
    e = []
    if os.path.isdir(path):
        f = os.listdir(path)
    else:
        return path
    for i in f:
        d = getfiles('%s/%s' % (path, i))
        try:
            e = e + d
        except:
            e.append(d)
    return e
def havekanji(text):
    for char in text:
        if '\u4E00' <= char <= '\u9FFF' or '\u3400' <= char <= '\u4DBF':
            return True
    return False
files = getfiles('.')
kanjifiles = []
#common = "`1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>? \n\r\t\b\f©"
for i in files:
    if i.startswith('./.git/') or i.startswith('./opencc') or i.startswith('./submodules') or i.endswith('.png') or i.endswith('.PNG'):
        continue
    else:
        try:
            f = open(i, 'r', encoding = 'utf-8', errors = 'ignore', newline = '\n').read()
            if havekanji(f):
                kanjifiles.append(i)
                print('Found Kanji file:', i)
        except Exception as error:
            print('Not Kanji file:', i, error)
            continue
with open('./kanjifiles.txt', 'w') as f:
    for i in kanjifiles:
        print(i, file = f)
