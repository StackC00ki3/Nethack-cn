import os

# 二进制/压缩包扩展名：里面不可能有需要简繁转换的文本，遇到直接跳过。
# 注意 doc/xx.pdf、doc/HYPHEN/xx.pdf 这类 PDF 是上游带进来的，
# 用 errors='ignore' 逐字节解码时随机字节会解出"汉字"，会被误判成
# 需要转换的文件，然后 opencc 读取时报 "Invalid UTF8" 并让整个 CI 挂掉。
SKIP_EXT = (
    '.png', '.PNG', '.jpg', '.jpeg', '.gif', '.bmp', '.ico', '.pdf',
    '.zip', '.gz', '.tgz', '.bz2', '.xz', '.7z', '.tar',
    '.exe', '.dll', '.so', '.dylib', '.o', '.obj', '.a', '.lib',
    '.pdb', '.ilk', '.res', '.class', '.jar', '.pyc',
    '.woff', '.woff2', '.ttf', '.otf', '.mp3', '.wav', '.ogg',
)

# 这些目录要么是版本库自身，要么是 CI 临时下载/产物，不参与转换。
SKIP_DIR = ('./.git/', './opencc', './opencc-tonggui', './submodules',
            './vspackage')

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

def is_text(path):
    """只有能完整按 UTF-8 解码、且不含 NUL 的文件才当文本处理。"""
    try:
        with open(path, 'rb') as fh:
            data = fh.read()
    except OSError:
        return False
    if b'\0' in data:
        return False
    try:
        data.decode('utf-8')
    except UnicodeDecodeError:
        return False
    return True

files = getfiles('.')
kanjifiles = []
for i in files:
    if i.startswith(SKIP_DIR) or i.lower().endswith(SKIP_EXT):
        continue
    if not is_text(i):
        print('Skipped non-text file:', i)
        continue
    try:
        f = open(i, 'r', encoding='utf-8', newline='\n').read()
        if havekanji(f):
            kanjifiles.append(i)
            print('Found Kanji file:', i)
    except Exception as error:
        print('Not Kanji file:', i, error)
        continue

# 必须写成 LF：CI 里用 `while read` 直接重定向读取这个文件，
# 行尾带 \r 会把路径读错。
with open('./kanjifiles.txt', 'w', encoding='utf-8', newline='\n') as f:
    for i in kanjifiles:
        print(i, file=f)
