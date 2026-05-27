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
- [ ] 物品翻译
- [ ] 许愿机制
- [ ] 灭绝机制

#### 人工审校
- [x] allmain.c
- [x] alloc.c
- [x] apply.c
- [x] artifact.c
- [x] attrib.c
- [x] ball.c
- [x] bones.c
- [ ] botl.c
- [x] calendar.c
- [ ] cfgfiles.c
- [ ] cmd.c
- [x] coloratt.c
- [x] date.c
- [ ] dbridge.c
- [ ] decl.c
- [ ] detect.c
- [ ] dig.c
- [ ] display.c
- [ ] dlb.c
- [ ] do.c
- [x] dog.c
- [ ] dogmove.c
- [x] dokick.c
- [ ] dothrow.c
- [ ] do_name.c
- [ ] do_wear.c
- [ ] drawing.c
- [ ] dungeon.c
- [ ] earlyarg.c
- [ ] eat.c
- [ ] end.c
- [ ] engrave.c
- [ ] exper.c
- [ ] explode.c
- [ ] extralev.c
- [ ] files.c
- [ ] fountain.c
- [ ] getpos.c
- [ ] glyphs.c
- [ ] hack.c
- [ ] hacklib.c
- [ ] iactions.c
- [ ] insight.c
- [ ] invent.c
- [ ] isaac64.c
- [ ] light.c
- [ ] lock.c
- [ ] mail.c
- [ ] makemon.c
- [ ] mcastu.c
- [ ] mdlib.c
- [ ] mhitm.c
- [ ] mhitu.c
- [ ] minion.c
- [ ] mklev.c
- [ ] mkmap.c
- [ ] mkmaze.c
- [ ] mkobj.c
- [ ] mkroom.c
- [ ] mon.c
- [ ] mondata.c
- [ ] monmove.c
- [ ] monst.c
- [ ] mplayer.c
- [ ] mthrowu.c
- [ ] muse.c
- [ ] music.c
- [ ] nhlobj.c
- [ ] nhlsel.c
- [x] nhlua.c
- [ ] nhmd4.c
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
- [ ] rect.c
- [ ] region.c
- [ ] report.c
- [ ] restore.c
- [ ] rip.c
- [ ] rnd.c
- [ ] role.c
- [ ] rumors.c
- [ ] save.c
- [ ] selvar.c
- [ ] sfbase.c
- [ ] sfstruct.c
- [ ] shk.c
- [ ] shknam.c
- [ ] sit.c
- [ ] sounds.c
- [ ] spell.c
- [ ] sp_lev.c
- [ ] stairs.c
- [ ] steal.c
- [ ] steed.c
- [ ] strutil.c
- [ ] symbols.c
- [ ] sys.c
- [ ] teleport.c
- [ ] tile.c
- [ ] timeout.c
- [ ] topten.c
- [ ] track.c
- [ ] trap.c
- [ ] uhitm.c
- [ ] utf8map.c
- [ ] u_init.c
- [ ] vault.c
- [ ] version.c
- [ ] vision.c
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

##### makeplural(const char *oldstr)

位置: [objnam.c](src/objnam.c)

功能: 将 oldstr 转成复数形式返回

**处理方案**: 将加后缀 s 的位置全部改成加空字符串

##### an(const char *str) / An / just_an

位置: [objnam.c](src/objnam.c)

功能: 调用了 `just_an()`，处理后，一般会给字符串前面加上 `"a "` 或者 `"an "`

**处理方案**: `just_an()` 返回 `"一个"`

##### s_suffix(const char *s)

位置: [hacklib.c](src/hacklib.c)

功能: 给字符串加 `"s"` 后缀

**处理方案**: 直接返回 `s`

##### vtense(const char *subj, const char *verb)

位置: [objnam.c](src/objnam.c)

功能: 返回在现在时第三人称下动词 `verb` 的正确形式

**处理方案**: 将加后缀 s 的位置改成加空字符串