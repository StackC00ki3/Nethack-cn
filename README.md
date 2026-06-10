## Nethack-cn

[![Build Status](https://github.com/StackC00ki3/nethack-cn/actions/workflows//nethack-vs-package.yml/badge.svg)](http://github.com/stackC00ki3/nethack-cn/releases)

English README：[README_en.md](README_en.md)

### 快速开始
无需本地编译，可直接在[本项目 Release 页面](http://github.com/stackC00ki3/nethack-cn/releases)下载自动构建的汉化预览版

### 路线图

- [x] tty 界面 UTF-8 支持
- [x] curses 界面 UTF-8 支持
- [x] win32 界面 UTF-8 支持
- [x] 合并来自 [SunnyYuer/NetHack-cn](https://github.com/SunnyYuer/NetHack-cn) 的翻译
- [x] 使用 deepseek-v4-flash 完成初步 AI 翻译
- [x] 怪物翻译
- [x] 物品翻译
- [x] 跨平台前端：**[直接启动浏览器版](https://stackc00ki3.github.io/nethack-3d/)** 仓库：[nethack-3d](https://github.com/StackC00ki3/nethack-3d)
- [x] 安卓版：**[点我下载apk](https://github.com/StackC00ki3/ANetHack-cn/releases)** 仓库：[Anethack-cn](https://github.com/StackC00ki3/ANetHack-cn)
- [x] 中文输入
- [ ] 许愿机制
- [ ] 灭绝机制
- [ ] 跨层传送机制

### 翻译标准化

[物品简中译名标准（objects.h）](doc/objects_translation_standard_zh_cn.md)

[怪物简中译名标准（monsters.h）](doc/monsters_translation_standard_zh_cn.md)

[通用简中翻译标准（谓词、体词、死因、声音、技能等）](doc/common_translation_standard_zh_cn.md)

**当前译名标准仍需要更多玩家参与讨论和校对，欢迎在以下讨论中提出意见：**

[讨论物品简中译名](https://github.com/StackC00ki3/Nethack-cn/discussions/3)

[讨论怪物简中译名](https://github.com/StackC00ki3/Nethack-cn/discussions/4)

[讨论通用译名](https://github.com/StackC00ki3/Nethack-cn/discussions/7)

#### 代码规范

翻译时请去除所有英文单词间的空格，仅在遇到数字时用空格分割，例如：“这里有 %ld 金币。”

翻译时标点符号请使用半角符号即英文标点符号

在遇到逗号时请在逗号后加一个空格

如果要修改语序（变量在字符串中出现的顺序），请在行最后添加注释：/*修改语序:(修改前的代码)*/

如果要用到不存在的（待补充的）函数，请把修改后的代码写到行最后的注释里：/*待写:(修改后的代码)*/

如果有冗余的代码，请在注释掉的代码前标注上“冗余：”：/*冗余:(冗余的代码)*/

如果对修改后的代码没有把握，请在行最后添加注释：/*危险:(修改前的代码)*/

#### 人工审校
##### 源代码

- [x] allmain.c
- [x] alloc.c
- [x] apply.c
- [x] artifact.c
- [x] attrib.c
- [x] ball.c
- [x] bones.c
- [x] botl.c
- [x] calendar.c
- [x] cfgfiles.c
- [x] cmd.c
- [x] coloratt.c
- [x] date.c
- [x] dbridge.c
- [x] decl.c
- [x] detect.c
- [x] dig.c
- [x] display.c
- [x] dlb.c
- [x] do.c
- [x] dog.c
- [x] dogmove.c
- [x] dokick.c
- [x] dothrow.c
- [x] do_name.c
- [x] do_wear.c
- [x] drawing.c
- [x] dungeon.c
- [x] earlyarg.c
- [x] eat.c
- [x] end.c
- [x] engrave.c
- [x] exper.c
- [x] explode.c
- [x] extralev.c
- [x] files.c
- [x] fountain.c
- [x] getpos.c
- [x] glyphs.c
- [x] hack.c
- [x] hacklib.c
- [x] iactions.c
- [x] insight.c
- [x] invent.c
- [x] isaac64.c
- [x] light.c
- [x] lock.c
- [x] mail.c
- [x] makemon.c
- [x] mcastu.c
- [x] mdlib.c
- [x] mhitm.c
- [x] mhitu.c
- [x] minion.c
- [x] mklev.c
- [x] mkmap.c
- [x] mkmaze.c
- [x] mkobj.c
- [x] mkroom.c
- [x] mon.c
- [x] mondata.c
- [x] monmove.c
- [x] monst.c
- [x] mplayer.c
- [x] mthrowu.c
- [x] muse.c
- [x] music.c
- [x] nhlobj.c
- [x] nhlsel.c
- [x] nhlua.c
- [x] nhmd4.c
- [ ] objects.c
- [ ] objnam.c
- [ ] options.c
- [ ] o_init.c
- [ ] pager.c
- [ ] pickup.c
- [ ] pline.c
- [ ] polyself.c
- [ ] potion.c
- [ ] pray.c
- [ ] priest.c
- [ ] quest.c
- [ ] questpgr.c
- [ ] read.c
- [x] rect.c (无需翻译)
- [ ] region.c
- [ ] report.c
- [ ] restore.c
- [ ] rip.c
- [x] rnd.c (无需翻译)
- [ ] role.c
- [ ] rumors.c
- [ ] save.c
- [x] selvar.c (无需翻译)
- [x] sfbase.c (无需翻译)
- [x] sfstruct.c (无需翻译)
- [ ] shk.c
- [ ] shknam.c
- [ ] sit.c
- [ ] sounds.c
- [ ] spell.c
- [x] sp_lev.c (无需翻译)
- [ ] stairs.c
- [ ] steal.c
- [ ] steed.c
- [ ] strutil.c
- [x] symbols.c (无需翻译)
- [x] sys.c (无需翻译)
- [ ] teleport.c
- [ ] tile.c
- [ ] timeout.c
- [ ] topten.c
- [x] track.c (无需翻译)
- [ ] trap.c
- [ ] uhitm.c
- [x] utf8map.c (无需翻译)
- [x] u_init.c (无需翻译)
- [ ] vault.c
- [x] version.c (无需翻译)
- [x] vision.c (无需翻译)
- [ ] weapon.c
- [ ] were.c
- [ ] wield.c
- [ ] windows.c
- [ ] wizard.c
- [ ] wizcmds.c
- [ ] worm.c
- [ ] worn.c
- [ ] write.c
- [ ] zap.c

##### 文本文件(主要的几个，还有好多)

- [ ] bogusmon.txt
- [ ] dungeon.lua
- [ ] engrave.txt
- [ ] epitaph.txt
- [ ] oracles.txt
- [ ] rumors.fal
- [ ] rumors.tru
- [ ] tribute
- [ ] quest.lua

### 技术细节

#### tty utf-8 支持

发现最后输出使用函数 `getchar` 逐个字符输出，而 `getchar` 支持宽字节。

于是调整输出逻辑：在当前指针指向的是 utf-8 内容时将整个字符串转为 `wchar_t *` 然后输出。

同时要调整 `console.cursor` 屏幕指针移动逻辑，当是宽字节时一次移动两个字符。

新增一种 cell 类型 `wide_char_follower_cell`, 用于标记宽字符的下一个cell为占用状态，使得清屏等操作能正确渲染。

#### curses utf-8 支持

编译 Nethack 时定义宏 `CURSES_UNICODE`, `PDC_WIDE`, `PDC_FORCE_UTF8`, `PDC_RGB`

对 pdcursesmod/pdcurses/refresh.c 进行了补丁，修复了一处 assert 引起的崩溃 bug。

#### win32 utf-8 支持

使用宏劫持 windows API 函数 `drawTextA`, `drawText`, `ListView_InsertColumn`。将它们替换成自定义的支持 utf8 的版本。

#### 英语语法函数

##### plur(x)

位置: [hack.h](include/hack.h)

功能: 根据数量参数 x 获取复数后缀的宏。

**处理方案**: 统一返回空字符串，不区分单复数形式。

##### makeplural(const char \*oldstr)

位置: [objnam.c](src/objnam.c)

功能: 将 oldstr 转成复数形式返回

**处理方案**: 将加后缀 s 的位置全部改成加空字符串

##### an(const char \*str) / An / just_an

位置: [objnam.c](src/objnam.c)

功能: 调用了 `just_an()`，处理后，一般会给字符串前面加上 `"a "` 或者 `"an "`

**处理方案**: `just_an()` 返回 `"一个"`

##### s_suffix(const char \*s)

位置: [hacklib.c](src/hacklib.c)

功能: 给字符串加 `"s"` 后缀

**处理方案**: 直接返回 `s`

##### ing_suffix(const char \*s)

位置: [hacklib.c](src/hacklib.c)

功能: 给英文动词加 `"ing"` 后缀。

**处理方案**: 直接返回 `s`；调用处需要“正在/进行”等语义时改用完整中文短语或重排格式串。

##### u_locomotion(const char \*def)

位置: [hack.c](src/hack.c)

功能: 根据玩家当前状态返回移动方式动词，例如普通移动、漂浮或飞行。

**处理方案**: 调用处可传入中文默认动词；漂浮和飞行分支返回中文动词，涉及语序时改写为完整中文句式。

##### vtense(const char \*subj, const char \*verb)

位置: [objnam.c](src/objnam.c)

功能: 返回在现在时第三人称下动词 `verb` 的正确形式

**处理方案**: 将加后缀 s 的位置改成加空字符串

##### uhe(), uhim(), uhis()

位置: [you.h](include/you.h)

功能: 返回人称代词的主格、宾格、形容词性物主代词（男："he"、"him"、"his"；女："she"、"her"、"her"；）

##### ordin(int n)

位置: [hacklib.c](src/hacklib.c)

功能: 返回数字 n 对应的序数词后缀（1→st，2→nd，3→rd……）

**处理方案**: 返回一个空字符串""

##### arti_light_description(wep)

位置: [light.c](src/light.c)

功能: 返回“radiantly”/“brilliantly”/“brightly”/“dimly”/“strangely”

**处理方案**: 只返回一个不带“的”的实词，使用时请在后面加上“的光芒”。

##### objdescr_is(struct obj \*obj, const char \*descr) 

位置: [o_init.c](src\o_init.c)

功能: 对比某物品的描述（(obj_descr[(obj).oc_descr_idx].oc_descr)）与descr是否相等

**处理方案**: 改为对比其edescr，调用时请保留英文。

##### getobj(const char \*word, int (\*obj_ok)(OBJ_P), unsigned int ctrlflags)

位置: [invent.c](src/invent.c)

功能: 寻找适合obj_ok行为的所有物品供玩家选择（若没有则默认展示所有物品）。

**处理方案**: 这个\*word对字符串不敏感。它会问你："你想要"+传入的\*word+"?"（汉语的这个地方填的词可能是离合的，如：“写在什么上”）。
