-- NetHack core nhcore.lua	$NHDT-Date: 1652196284 2022/05/10 15:24:44 $  $NHDT-Branch: NetHack-3.7 $:$NHDT-Revision: 1.0 $
--	Copyright (c) 2021 by Pasi Kallinen
-- NetHack may be freely redistributed.  See license for details.
-- This file contains lua code used by NetHack core.
-- Is it loaded once, at game start, and kept in memory until game exit.

-- Data in nh_lua_variables table can be set and queried with nh.variable()
-- This table is saved and restored along with the game.
nh_lua_variables = {
};

-- wrapper to simplify calling from nethack core
function get_variables_string()
   return "nh_lua_variables=" .. table_stringify(nh_lua_variables) .. ";";
end

function nh_callback_set(cb, fn)
   local cbname = "_CB_" .. cb;

   -- pline("callback_set(%s,%s)", cb, fn);

   if (type(nh_lua_variables[cbname]) ~= "table") then
      nh_lua_variables[cbname] = {};
   end
   nh_lua_variables[cbname][fn] = true;
end

function nh_callback_rm(cb, fn)
   local cbname = "_CB_" .. cb;

   -- pline("callback_RM(%s,%s)", cb, fn);

   if (type(nh_lua_variables[cbname]) ~= "table") then
      nh_lua_variables[cbname] = {};
   end
   nh_lua_variables[cbname][fn] = nil;
end

function nh_callback_run(cb, ...)
   local cbname = "_CB_" .. cb;

   -- pline("callback_run(%s)", cb);
   -- pline("TYPE:%s", type(nh_lua_variables[cbname]));

   if (type(nh_lua_variables[cbname]) ~= "table") then
      nh_lua_variables[cbname] = {};
   end
   for k, v in pairs(nh_lua_variables[cbname]) do
      if (not _G[k](table.unpack{...})) then
         return false;
      end
   end
   return true;
end

-- This is an example of generating an external file during gameplay,
-- which is updated periodically.
-- Intended for public servers using dgamelaunch as their login manager.
local prev_dgl_extrainfo = 0;
function mk_dgl_extrainfo()
    if ((prev_dgl_extrainfo == 0) or (prev_dgl_extrainfo + 50 < u.moves)) then
        local filename = nh.dump_fmtstr("/tmp/nethack.%n.%d.log");
        local extrai, err = io.open(filename, "w");
        if extrai then
            local sortval = 0;
            local dname = nh.dnum_name(u.dnum);
            local dstr = "";
            local astr = " ";
            if u.uhave_amulet == 1 then
                sortval = sortval + 1024;
                astr = "A";
            end
            if dname == "Fort Ludios" then
                dstr = "Knx";
                sortval = sortval + 245;
            elseif dname == "The Quest" then
                dstr = "Q" .. u.dlevel;
                sortval = sortval + 250 + u.dlevel;
            elseif dname == "The Elemental Planes" then
                dstr = "End";
                sortval = sortval + 256;
            elseif dname == "Vlad's Tower" then
                dstr = "T" .. u.dlevel;
                sortval = sortval + 235 + u.depth;
            elseif dname == "Sokoban" then
                dstr = "S" .. u.dlevel;
                sortval = sortval + 225 + u.depth;
            elseif dname == "The Gnomish Mines" then
                dstr = "M" .. u.dlevel;
                sortval = sortval + 215 + u.dlevel;
            else
                dstr = "D" .. u.depth;
                sortval = sortval + u.depth;
            end
            local str = sortval .. "|" .. astr .. " " .. dstr;

            extrai:write(str);
            extrai:close();
        else
            -- failed to open the file.
            nh.pline("Failed to open dgl extrainfo file: " .. err);
        end
        prev_dgl_extrainfo = u.moves;
    end
end

-- Show a helpful tip when player first uses getpos()
function show_getpos_tip()
   nh.text([[
提示: 远景浏览或选择地图位置

你现在处于"远景浏览"模式--移动键控制光标移
动, 而非角色移动. 游戏时间不会推进. 此模式
用于环顾地图, 或在地图上选择位置.

在此模式下, 你可以按ESC键返回正常游戏模式,
按?键将显示按键帮助.
]]);
end

-- Callback functions
nhcore = {
    -- start_new_game called once, when starting a new game
    -- after "Welcome to NetHack" message has been given.
    -- start_new_game = function() nh.pline("NEW GAME!"); end,

    -- restore_old_game called once, when restoring a saved game
    -- after "Welcome back to NetHack" message has been given.
    -- restore_old_game = function() nh.pline("RESTORED OLD GAME!"); end,

    -- moveloop_turn is called once per turn.
    -- moveloop_turn = mk_dgl_extrainfo,

    -- game_exit is called when the game exits (quit, saved, ...)
    -- game_exit = function() end,

    -- getpos_tip is called the first time the code enters getpos()
    getpos_tip = show_getpos_tip,

    -- enter_tutorial and leave_tutorial
    enter_tutorial = tutorial_enter,
    leave_tutorial = tutorial_leave,
};

