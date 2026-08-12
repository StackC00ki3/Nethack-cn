# 通用翻译标准

## 谓词（src/mondata.c:1367；src/uhitm.c:1653；src/potion.c:1121；src/worn.c:981；src/objnam.c:1391）

|英文|中文|
|----|----|
|bounce|反弹|
|reflect|反射|
|hit(结果)|击中|
|better（恢复）|好些了|
|much better|好多了|
|shine|照耀<sup>[1](#note1)</sup>|
|glow|发/散发|
|violently glow|爆发|
|shatter|粉碎|
|wobble|摇晃|
|flutter|扑腾|
|stagger|踉跄|
|slither|蠕动|
|falter|蹒跚|
|flop|扑腾|
|ooze|滑动|
|tremble|颤抖|
|wiggle|扭动|
|pulsate|震动（注意不同于“振动”）|
|crawl|爬行|
|bash|猛击|
|lash|抽打|
|smite|重击|
|hit(攻击动作)|打|
|sear(银质物品对恶魔)|烧灼|
|crush(攻击)|挤压|
|queasy|反胃|
|sick(如果没有致病)|不适|
|squeak|嘎吱作响|
|sluggish|呆滞|
|lethargic|疲惫|
|gallop|疾驰|
|(being worn)|(穿戴中)|
|(alternate weapon; not wielded)|(副武器;未装备)|
|(on left hand)|(在左手上)|
|(on right hand)|(在右手上)|
|tingle|颤动|
|itch|发痒|
|twitch|抽搐|
|transform|转变|
|polymorph|变形|
|open,closed(门)|打开的,关上的|

## 体词（include/monsters.h:2127；include/objects.h:141；include/defsym.h:110；include/defsym.h:177；src/trap.c:1905）

|英文|中文|
|----|----|
|form(变形)|形态|
|quantum mechanic(怪物)|量子技工<sup>[2](#note2)</sup>|
|thou, thee, thy, thine|汝，汝，尔，尔<sup>[3](#note3)</sup>|
|arrow, bolt|箭<sup>[4](#note4)</sup>|
|barb|倒刺|
|debris(空气元素)|碎片|
|flash|闪光|
|aura|光晕|
|light|光线|
|anti-magic field|反魔法力场|
|spike(陷阱里的)|尖刺|
|shape changers|变形怪|
|iron bars|铁栅栏|


## 死因（src/zap.c:5778；src/wield.c:150；src/worn.c:1328；src/uhitm.c:2162；src/uhitm.c:2470）

|英文|中文|
|----|----|
|shattered potion|药水冻裂|
|boiling potion|药水沸腾|
|exploding potion|药水爆炸|
|burning scroll|卷轴燃烧|
|burning book|书燃烧|
|exploding wand|魔杖爆炸|
|wielding %s bare-handed|徒手手持%s|
|falling off %s|从%s身上跌落|
|stolen|偷窃|
|expire|消散|


## 声音（src/worn.c:1199；src/zap.c:5426；src/sounds.c:222；src/trap.c:1200；dat/data.base:2849）

|英文|中文|
|----|----|
|cracking sound|破裂声|
|tipping sound|撕裂声|
|crumbling sound|碎裂声|
|clank|当啷声|
|crackling|劈啪声|
|hissing|嘶嘶声|
|cough|咳嗽声|
|click|咔哒声|
|rumbling|隆隆声|
|slow drip|滴答声|
|gurgling|咕噜声|
|clatter|哐当声|
|snicker-snack|咔嚓|

## 感叹词/拟声词（src/uhitm.c:233；src/potion.c:774；src/trap.c:2558；src/region.c:989；src/sounds.c:549）

|英文|中文|
|----|----|
|Wait!|等等!|
|Ouch!|哎呦!|
|Splat!|啪!|
|Splash!|哗啦!|
|Burrrrp!|嗝!|
|KAABLAMM!|轰隆!|
|Kaablamm!|轰!|
|Click!|咔哒!|
|Thump!|砰!|
|Bummer!|真倒霉!|
|Phew|呼!|
|Whoops...|哎呀...|
|KABOOM|嘭!|
|Egads!|天哪!|
|Ulch!|呃!|
|Ooph!|呼!|
|Crash!|哗啦!|


## 技能（src/weapon.c:52；src/weapon.c:1092）

|英文|中文|
|----|----|
|no skill|无技能|
|bare hands|徒手|
|two weapon combat|双持|
|riding|骑乘|
|polearms|长棍|
|saber|军刀|
|hammer|锤子|
|whip|鞭子|
|attack spells|攻击法术|
|healing spells|治疗法术|
|divination spells|预测法术|
|enchantment spells|附魔法术|
|clerical spells|神圣法术|
|escape spells|逃脱法术|
|matter spells|物质法术|
|bare handed combat|徒手格斗|
|martial arts|武术|
|Unskilled（熟练度）|无技能|
|Basic|基本|
|Skilled|熟练|
|Expert|专家|
|Master|大师|
|Grand Master|宗师|
|Unknown|未知|

## 状态/属性（src/timeout.c:27）

|英文|中文|
|----|----|
|invulnerable|无敌|
|petrifying|石化|
|becoming slime|变成黏液|
|strangling|窒息|
|fatally sick|濒死|
|stunned|眩晕|
|confused|混乱|
|hallucinating|幻觉|
|blinded|失明|
|deafness|失聪|
|vomiting|呕吐|
|slippery fingers|手滑|
|wounded legs|腿受伤|
|sleepy|困倦|
|teleporting|传送|
|polymorphing|变形|
|levitating|飘浮|
|very fast|加速|
|clairvoyant|超视|
|monster detection|感知怪物|
|see invisible|看透隐形|
|invisible|隐形|
|acid resistance|酸抗|
|stoning resistance|石化抗性|
|displaced|换位|
|pass thru walls|穿墙|
|magical breathing|魔法呼吸|
|water walking|水上行走|
|fire resistance|火焰|
|cold resistance|寒冷抗性|
|disintegration resistance|分解抗性|
|shock resistance|闪电抗性|
|poison resistance|毒素抗性|
|drain resistance|降级抗性|
|sickness resistance|疾病抗性|
|magic resistance|魔法抗性|
|hallucination resistance|幻觉抗性|
|light-induced blindness resistance|光致失明抗性|
|fumbling|笨拙|
|voracious hunger|饥饿|
|telepathic|心灵感应|
|warn: monster type or class|警觉: monster type or class|
|warn: undead|警觉: undead|
|searching|搜索|
|adorned (+/- Cha)|装饰 (+/- Cha)|
|stealthy|潜行|
|monster aggravation|激怒怪物|
|conflict|冲突|
|jumping|跳跃|
|teleport control|传送控制|
|flying|飞行|
|swimming|游泳|
|half spell damage|魔法伤害减半|
|half physical damage|物理伤害减半|
|HP regeneration|生命再生|
|energy regeneration|魔法能量再生|
|extra protection|额外保护|
|polymorph control|变形控制|
|unchanging|变形抗性|
|reflecting|反射|
|free action|自由行动|
|life will be saved|保命|

## 地形/表面（src/dungeon.c:1750）

|英文|中文|
|----|----|
|maw|胃|
|husk|外壳|
|nonesuch|没有这种东西|
|air bubble|气泡|
|cloud|云|
|air|空气|
|bottom|底部|
|water|水|
|ice|冰|
|lava|熔岩|
|bridge|吊桥|
|altar|祭坛|
|headstone|墓碑|
|fountain|喷泉|
|stairs|楼梯|
|wall|墙|
|doorway|门口|
|floor|地板|
|ground|地|

## 移动和燃烧谓词（src/mondata.c:1367；src/mondata.c:1411）

|英文|中文|
|----|----|
|float|飘浮|
|fly|飞|
|slither(移动)|滑|
|ooze(移动)|渗|
|wiggle(无移动能力)|扭动|
|crawl(移动)|爬|
|already on fire|本来就是着火的|
|boiling|沸腾了|
|melting|熔化了|
|heating up|升温了|
|being roasted|正在被烤|
|on fire|着火了|

## 射线、法术和吐息（src/zap.c:71）

|英文|中文|
|----|----|
|magic missile|魔法飞弹|
|bolt of fire|火箭|
|bolt of cold|冰箭|
|sleep ray|睡眠射线|
|death ray|死亡射线|
|bolt of lightning|电箭|
|fireball|火球|
|cone of cold|冰锥|
|finger of death|死亡一指|
|blast of missiles|飞弹冲击|
|blast of fire|火焰冲击|
|blast of frost|冰霜冲击|
|blast of sleep gas|睡气冲击|
|blast of disintegration|分解冲击|
|blast of lightning|闪电冲击|
|blast of poison gas|毒气冲击|
|blast of acid|酸液冲击|

## 死因和毁坏原因（src/zap.c:5778；src/mcastu.c:332；src/mcastu.c:482）

|英文|中文|
|----|----|
|freezes and shatters|冻裂|
|freeze and shatter|冻裂|
|boils and explodes|沸爆|
|boil and explode|沸爆|
|ignites and explodes|燃爆|
|ignite and explode|燃爆|
|catches fire and burns|着火|
|catch fire and burn|着火|
|turns to dust and vanishes|化为尘土|
|breaks apart and explodes|解体并爆炸|
|the touch of death|死亡之触|
|strength loss|力量流失|

## 陷阱和侵蚀（src/trap.c:79；src/trap.c:178）

|英文|中文|
|----|----|
|tower of flame|一柱火焰|
|A gush of water hits|一股水流击中了|
|humid|湿润|
|odorless|无味|
|pungent|刺鼻|
|chilling|冰凉|
|acrid|呛人|
|biting|辛辣|
|smoulder|烧坏|
|rust|生锈|
|rot|腐烂|
|corrode|腐蚀|
|crack|破裂|
|burnt|烧坏的|
|rusted|生锈的|
|rotten|腐烂的|
|corroded|被腐蚀的|
|cracked|破裂的|
|heat|高温|
|oxidation|氧化|
|decay|腐烂|
|corrosion|腐蚀|
|impact|冲击|

## 怪物声音（src/sounds.c:341；src/sounds.c:351）

|英文|中文|
|----|----|
|beep|哔|
|boing|咻|
|sing|唱|
|belche|打嗝|
|creak|嘎吱|
|cough(幻觉声音)|咳|
|rattle|咔嗒|
|ululate|哀号|
|pop|噗|
|jingle|叮当|
|sniffle|抽鼻|
|tinkle|叮咚|
|eep|吱|
|clatter(幻觉声音)|哐当|
|hum|嗡|
|sizzle|滋滋|
|twitter|啁啾|
|wheeze|喘息|
|rustle|沙沙|
|honk|嘟|
|lisp|咬舌|
|yodel|假|
|coo|咕|
|burp|打饱嗝|
|moo|哞|
|boom|嘭|
|murmur|低语|
|oink|嗷嗷|
|quack|嘎嘎|
|rumble|隆隆|
|twang|鼻|
|toot|嘟嘟|
|gargle|咕噜|
|hoot|呼啸|
|warble|咏叹|
|hiss|嘶嘶|
|growl|低吼|
|roar|咆哮|
|bellow|大吼|
|buzz|嗡嗡|
|squeal|尖叫|
|screech|尖啸|
|neigh|马嘶|
|wail|哀号|
|groan|呻吟|
|low|低鸣|
|commotion|骚动|
|scream|大叫|
|yowl|发出嚎叫|
|yelp|发出尖叫|
|snarl|发出咆哮|
|screak|发出尖啸|
|whimper|呜咽|
|whine|哀嚎|

## 商店术语（src/shk.c:1012）

|英文|中文|
|----|----|
|credit|信用|
|debt|欠款|
|unpaid item|未付物品|
|payment|付款|
|price|价格|
|shopkeeper|店主|
|gold|金币|
|pay|支付|
|bill|账单|
|compensation|赔偿|

## 神殿和祭司术语（src/priest.c:320；src/priest.c:452）

|英文|中文|
|----|----|
|poohbah|大人物|
|priestess|祭司|
|priest|祭司|
|renegade|反叛的|
|invisible(祭司修饰语)|隐形的|
|high priest|高祭司|
|Moloch's Sanctum|摩洛的圣所|
|Infidel|异教徒|
|pilgrim|朝圣者|
|desecrated place|被亵渎的地方|
|holy place|圣地|
|temple|神庙|
|donation|捐款|
|donate|捐献|
|atheism|无神论|

## 其他

|英文|中文|
|----|----|
|under you|在你下面(不能是脚下，因为你可能没有脚LOL)|

<a id="note1">1</a> 见[鱼佬的解释](https://github.com/SunnyYuer/NetHack-cn/wiki#shine-shining)。

<a id="note2">2</a> 根据[Wiki](https://nethackwiki.com/wiki/Quantum_mechanic#Origin)，这是一个对quantum mechanics（单数，“量子力学”）错误逆构词导致的双关，且从[贴图](https://nethackwiki.com/wiki/File:Quantum_mechanic.png)和游戏内信息可以推断出quantum mechanic显然是人形生物，不应翻译为“量子力学”。

<a id="note3">3</a> 含有这种人称代词或shalt(shall 2nd sg)、art(be 2nd sg)、-est(2nd sg)、-eth(3rd sg)等的句子当译为文言。
~~也不一定是文言吧，或者像[浅文理和合本](https://www.bible.com/bible/1577/)那样的浅近文言？~~不行，还是不够高语域。
拉丁语我打算保留一个原句再加一个翻译。

<a id="note4">4</a> bolt只有crossbow bolt（弩箭）一种，和arrow没有最小对立。
