
local tut_ctrl_key = nil;
local tut_alt_key = nil;

function tut_key(command)
   local s = nh.eckey(command);
   local m = s:match("^^([A-Z])$"); -- ^X is Ctrl-X
   if (m ~= nil) then
      tut_ctrl_key = m;
      return "Ctrl-" .. m;
   end

   m = s:match("^M%-([A-Z])$"); -- M-X is Alt-X
   if (m ~= nil) then
      tut_alt_key = m;
      return "Alt-" .. m;
   end

   return s;
end

function tut_key_help(x, y)
   if (tut_ctrl_key ~= nil) then
      des.engraving({ coord = { x,y }, type = "engrave", text = "注意: 出了教程, Ctrl-键组合会写成插入符号前缀, 像这样: '^" .. tut_ctrl_key .. "'", degrade = false });
      tut_ctrl_key = nil;
   end
end

des.level_init({ style = "solidfill", fg = " " });
des.level_flags("mazelevel", "noflip",
                "nomongen", "nodeathdrops", "noautosearch");

des.map([[
---------------------------------------------------------------------------
|-.--|.......|......|..S....|.F.......|.............|.......|.............|
|.-..........|......|--|....|.F.....|.|S-------.....|.....................|
||.--|.......|..T......|....|.F.....|.|.......|.....|.......|.............|
||.|.|.......|......|-.|....|.F.....|.|.......|.....|--------.............|
||.|.|.......|......||.|-.-----------.-.......|-S----.....................|
|-+-S---------..---.||........................|...|.......................|
|......|          |.-------------------.......|...|....--S----............|
|......|  ######  |.........|      |..S.......|...|....|.....|............|
|----.-| -+-   #  |.....---.|######+..|.......S...|....|.....|............|
|----+----.----+---.|.--|.|.|#     ------------...|....|.....F............|
|........|.|......|.|...F...|#  ........|.....+...|....|.....|............|
|.P......-S|......|------.---# .........|.....|...|....-------........----|
|..........|......+.|...|.|.S# ..--S-----.....|LLL|..................|..| |
|.W......---......|.|.|.|.|.|# ..|......|.....|LLL|..................|..--|
|....Z.L.S.F......|.|.|.|.---#   |......+.....|...|..................|..|.|
|........|--......|...|.....|####+......|.....|...+..................||...|
---------------------------------------------------------------------------
]]);


des.region(selection.area(01,01, 73, 16), "lit");

des.non_diggable();

des.teleport_region({ region = { 9,3, 9,3 } });

-- TODO:
--  - save (more of) hero state when entering

-- turn on some newbie-friendly options
nh.parse_config("OPTIONS=mention_walls");
nh.parse_config("OPTIONS=mention_decor");
nh.parse_config("OPTIONS=lit_corridor");

-- BUG? this sets the movement-hint engraving to HJKL or 4286 depending on
-- the setting of number_pad at the time the level is created, but it doesn't
-- change to match new value if the player uses 'm O' to change number_pad
-- while in the tutorial.
-- [Don't bother with a complex fix; a player who can use 'm O' doesn't need
-- the tutorial.]

local movekeys = tut_key("movewest") .. " " ..
   tut_key("movesouth") .. " " ..
   tut_key("movenorth") .. " " ..
   tut_key("moveeast");

local diagmovekeys = tut_key("movesouthwest") .. " " ..
   tut_key("movenortheast") .. " " ..
   tut_key("movesoutheast") .. " " ..
   tut_key("movenorthwest");

des.engraving({ coord = { 9,3 }, type = "engrave", text = "按" .. movekeys .. "移动", degrade = false });
des.engraving({ coord = { 5,2 }, type = "engrave", text = "按" .. diagmovekeys .. "斜向移动", degrade = false });

if (u.role == "Knight") then
   des.engraving({ coord = { 12,1 }, type = "engrave", text = "骑士可以按'" .. tut_key("jump") .. "'跳跃", degrade = false });
end

--

des.engraving({ coord = { 2,4 }, type = "engrave", text = "某些操作可能需要尝试多次才能成功", degrade = false });
des.engraving({ coord = { 2,5 }, type = "engrave", text = "朝门移动以开门", degrade = false });
des.door({ coord = { 2,6 }, state = "closed" });

des.engraving({ coord = { 2,7 }, type = "engrave", text = "按'" .. tut_key("close") .. "'关门", degrade = false });


--

des.engraving({ coord = { 4,5 }, type = "engrave", text = "通过魔法传送门离开本教程.", degrade = false });
des.trap({ type = "magic portal", coord = { 4,4 }, seen = true });

--

des.engraving({ coord = { 5,9 }, type = "engrave", text = "这扇门是锁着的. 按'" .. tut_key("kick") .. "'踢开它", degrade = false });
des.door({ coord = { 5,10 }, state = "locked" });

-- by default, kick is the first command that can be a ctrl-key combo
tut_key_help(6, 8);


des.engraving({ coord = { 5,12 }, type = "engrave", text = "按'" .. tut_key("glance") .. "'查看周围, ESC键退出", degrade = false });

--

des.engraving({ coord = { 10,13 }, type = "engrave", text = "按'" .. tut_key("search") .. "'搜索隐藏门", degrade = false });

des.engraving({ coord = { 10,15 }, type = "engrave", text = "没有秘密", degrade = false });

--

des.engraving({ coord = { 10,10 }, type = "engrave", text = "门后面是个昏暗的走廊", degrade = false });
des.door({ coord = { 10,9 }, state = percent(50) and "locked" or "closed" });
des.region(selection.match("#"), "unlit");
des.region(selection.match(" "), "unlit");
des.door({ coord = { 15,10 }, state = percent(50) and "locked" or "closed" });

--

des.engraving({ coord = { 15,11 }, type = "engrave", text = "你身边有四个陷阱! 去找找看.", degrade = false });
local locs = { {14,11}, {14,12}, {15,12}, {16,12}, {16,11} };
shuffle(locs);
for i = 1, 4 do
   des.trap({ type = percent(50) and "sleep gas" or "board",
              coord = locs[i], victim = false });
end

des.engraving({ coord = { 15,15 }, type = "engrave", text = "有些陷阱可以按'" .. tut_key("untrap") .. "'解除", degrade = false });
des.trap({ coord = { 15,16 }, type = "web", spider_on_web = false });

--

des.door({ coord = { 18,13 }, state = "closed" });

des.engraving({ coord = { 19,13 }, type = "engrave", text = "按'" .. tut_key("pickup") .. "'捡起", degrade = false });

local armor = (u.role == "Monk") and "leather gloves" or "leather armor";

des.object({ id = armor, spe = 0, buc = "cursed", coord = { 19,14} });

des.engraving({ coord = { 19,15 }, type = "engrave", text = "按'" .. tut_key("wear") .. "'穿上防具", degrade = false });

des.object({ id = "dagger", spe = 0, buc = "not-cursed", coord = { 21,15} });

des.engraving({ coord = { 21,14 }, type = "engrave", text = "按'" .. tut_key("wield") .. "'装备武器", degrade = false });


des.engraving({ coord = { 22,13 }, type = "engrave", text = "朝怪物移动以攻击.", degrade = false });

des.monster({ id = "lichen", coord = { 23,15 }, waiting = true, countbirth = false });

--

des.engraving({ coord = { 24,16 }, type = "engrave", text = "现在你已经掌握了最基础的知识. 你可以通过魔法传送门离开本教程.", degrade = false });

des.engraving({ coord = { 26,16 }, type = "engrave", text = "走到这个传送门上即可离开教程", degrade = false });
des.trap({ type = "magic portal", coord = { 27,16 }, seen = true });

--

des.engraving({ coord = { 25,13 }, type = "engrave", text = "朝巨石移动以推动", degrade = false });
des.object({ id = "boulder", coord = {25,12} });

--

des.engraving({ coord = { 27,9 }, type = "engrave", text = "按'" .. tut_key("takeoff") .. "'脱下防具", degrade = false });

--

des.object({ class = "?", id = "remove curse", buc = "blessed", coord = {23,11} })
des.engraving({ coord = { 22,11 }, type = "engrave", text = "某些物品的描述会随机变化, 每局游戏都不一样", degrade = false });
des.engraving({ coord = { 23,11 }, type = "engrave", text = "捡起这张卷轴, 按'" .. tut_key("read") .. "'读出来, 然后再试试脱下防具", degrade = false });

--

des.engraving({ coord = { 19,10 }, type = "engrave", text = "另一个魔法传送门, 可以离开教程", degrade = false });
des.trap({ type = "magic portal", coord = { 19,11 }, seen = true });

--

-- rock fall
des.object({ coord = {14, 5}, id = "rock", quantity = math.random(50,99) });
des.object({ coord = {15, 5}, id = "rock", quantity = math.random(10,30) });
des.object({ coord = {14, 4}, id = "rock", quantity = math.random(10,30) });
des.object({ coord = {15, 6}, id = "rock", quantity = math.random(30,60) });
des.object({ coord = {14, 6}, id = "rock", quantity = math.random(30,60) });
des.object({ coord = {14, 6}, id = "boulder" });

des.door({ coord = { 20,3 }, state = percent(50) and "open" or "closed" });

des.engraving({ coord = { 21,3 }, type = "engrave", text = "不要负重, 不然你行动会很慢", degrade = false });
des.engraving({ coord = { 22,3 }, type = "engrave", text = "按'" .. tut_key("drop") .. "'丢下物品", degrade = false });
des.engraving({ coord = { 22,4 }, type = "engrave", text = "你可以在输入物品栏字母前加上数字, 以丢弃部分堆叠的物品", degrade = false });

--

des.monster({ id = "yellow mold", coord = { 26,2 }, waiting = true, countbirth = false });

des.engraving({ coord = { 25,5 }, type = "engrave", text = "按'" .. tut_key("throw") .. "'扔出物品", degrade = false });

des.trap({ type = "magic portal", coord = { 21,1 }, seen = true });

--

des.monster({ id = "wolf", coord = { 29,2 }, peaceful = 0, waiting = true, countbirth = false });

des.engraving({ coord = { 37,4 }, type = "engrave", text = "发射物, 比如岩石, 用合适的发射器射出去比扔出去伤害更高", degrade = false });

des.object({ coord = { 37,3 }, id = "sling", buc = "not-cursed", spe = 9 });
des.engraving({ coord = { 37,3 }, type = "engrave", text = "装备投石器", degrade = false });
des.engraving({ coord = { 36,1 }, type = "engrave", text = "按'" .. tut_key("fire") .. "'用发射器发射弹药", degrade = false });

des.engraving({ coord = { 35,4 }, type = "engrave", text = "开火会从你的箭袋中发射; 按'" .. tut_key("quiver") .. "'往里面放东西", degrade = false });

des.engraving({ coord = { 33,4 }, type = "engrave", text = "按'" .. tut_key("wait") .. "'可以等待一回合", degrade = false });


--

des.door({ coord = { 38,6 }, state = "closed" });

des.engraving({ coord = { 39,6 }, type = "engrave", text = "按'" .. tut_key("loot") .. "'搜刮容器", degrade = false });

des.object({ coord = { 41,6 }, id = "large box", broken = true, trapped = false,
             contents = function(obj)
                des.object({ id = "secret door detection", class = "/", spe = 30 }); end
});
des.engraving({ coord = { 42,6 }, type = "engrave", text = "也可以按'" .. tut_key("tip") .. "'倒空容器", degrade = false });

des.engraving({ coord = { 45,6 }, type = "engrave", text = "按'" .. tut_key("zap") .. "'挥舞魔杖", degrade = false });

--

des.door({ coord = { 35,9 }, state = "nodoor" });
des.engraving({ coord = { 34,9 }, type = "engrave", text = "在移动前加'" .. tut_key("run") .. "'可以奔跑", degrade = false });

--

des.door({ coord = { 33,16 }, state = "nodoor" });
des.engraving({ coord = { 35,15 }, type = "engrave", text = "按'" .. tut_key("travel") .. "'可以在地图中快速旅行", degrade = false });

--

des.trap({ type = "magic portal", coord = { 27,14 }, seen = true });

--

des.engraving({ coord = { 48,1 }, type = "burn", text = "按'" .. tut_key("eat") .. "'可以吃东西", degrade = false });

des.object({ coord = { 50,3 }, id = "apple", buc = "not-cursed"  });
des.object({ coord = { 50,3 }, id = "candy bar", buc = "not-cursed"  });

des.object({ coord = { 50,3 }, id = "corpse", montype = "lichen", buc = "not-cursed" });

--

des.door({ coord = { 46,11 }, state = "closed" });

des.engraving({ coord = { 43,11 }, type = "burn", text = "按'" .. tut_key("twoweapon") .. "'同时使用两件武器", degrade = false });
des.object({ coord = { 43,13 }, id = "knife", buc = "uncursed" });
des.object({ coord = { 43,14 }, id = "dagger", buc = "blessed" });

des.engraving({ coord = { 43,16 }, type = "burn", text = "按'" .. tut_key("swap") .. "'快速切换武器", degrade = false });

des.door({ coord = { 40,15 }, state = "random" });

--

des.object({ coord = { 48,7 }, id = "ring of levitation", buc = "not-cursed" });

des.engraving({ coord = { 48,10 }, type = "burn", text = "按'" .. tut_key("puton") .. "'戴上饰品", degrade = false });

des.engraving({ coord = { 48,16 }, type = "burn", text = "按'" .. tut_key("remove") .. "'摘下饰品", degrade = false });

des.door({ coord = { 50,16 }, state = "closed" });


--

des.engraving({ coord = { 58,9 }, type = "burn", text = "按'" .. tut_key("down") .. "'下楼梯", degrade = false });
des.stair({ dir = "down", coord = { 58,10 } });

--

-- one more ctrl-key help, if needed
tut_key_help(64, 4);

des.engraving({ coord = { 65,3 }, type = "burn", text = "建设中", degrade = false });

des.trap({ type = "magic portal", coord = { 66,2 }, seen = true });

--

-- squeezing through small gaps


des.engraving({ coord = { 69,12 }, type = "burn", text = "过不来? 你带的东西太多了.", degrade = false });
des.engraving({ coord = { 70,13 }, type = "burn", text = "按'" .. tut_key("droptype") .. "'批量丢下物品", degrade = false });

-- try to squeeze over boulders, find a trap door

des.object({ id = "boulder", coord = {71,16} });
des.object({ id = "boulder", coord = {72,16} });
des.object({ id = "boulder", coord = {73,16} });
des.trap({ type = "trap door", coord = { 73,15 } });

--

des.engraving({ coord = { 60,2 }, type = "engrave", text = "施法", degrade = false });
if (u.uenmax < 5) then
   -- TODO: make sure hero has enough Pw to cast the spell (5 pw) instead?
   -- TODO: ensure the first cast of this spell succeeds?
   des.engraving({ coord = { 59,2 }, type = "engrave", text = "不幸的是, 你的能量不够用来施放法术.", degrade = false });
end
des.engraving({ coord = { 57,2 }, type = "engrave", text = "按'" .. tut_key("pickup") .. "'捡起魔法书", degrade = false });
des.object({ coord = { 57,2 }, id = "spellbook of light", buc = "blessed" });
des.engraving({ coord = { 55,2 }, type = "engrave", text = "按'" .. tut_key("read") .. "'阅读魔法书", degrade = false });
des.engraving({ coord = { 53,2 }, type = "engrave", text = "按'" .. tut_key("cast") .. "'施放法术", degrade = false });
des.region(selection.area(53,01, 59, 3), "unlit");

--

des.engraving({ coord = { 72,2 }, type = "engrave", text = "按'" .. tut_key("quaff") .. "'喝药水", degrade = false });
des.object({ coord = { 72,2 }, id = "potion of object detection", buc = "blessed" });


----------------

-- entering and leaving tutorial _branch_ now handled by core
-- // nh.callback("cmd_before", "tutorial_cmd_before");
-- // nh.callback("level_enter", "tutorial_enter");
-- // nh.callback("level_leave", "tutorial_leave");
-- // nh.callback("end_turn", "tutorial_turn");

----------------

-- temporary stuff here
-- des.trap({ type = "magic portal", coord = { 9,5 }, seen = true });
-- des.trap({ type = "magic portal", coord = { 9,1 }, seen = true });
-- des.object({ id = "leather armor", spe = 0, coord = { 9,2} });

