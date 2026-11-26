--
-- GameModeLayer
--
-- Port dari Classes/UI/GameModeLayer.cpp. Tampilan (UI) dibangun di sini,
-- sisi C++ hanya menyediakan binding:
--   GameModeLayer:createLua()   -- layer kosong, UI dari GameModeLayer:init() ini
--   GameModeLayer:getModeCount()
--   GameModeLayer:getModeImage(mode)
--   GameModeLayer:startMode(mode)     -- konfirmasi mode -> SelectLayer
--   GameModeLayer:setCheats(n)
--

-- Id mode (urut sesuai enum GameMode di C++)
local MODE = {
    OneVsOne = 0,
    TwoVsTwo = 7,
    Classic = 2,
    FourVsFour = 3,
    HardCore_4Vs4 = 1,
    Clone = 5,
    Deathmatch = 6,
    RandomDeathmatch = 4
}

-- Jumlah mode yang tampil per halaman
local MODES_PER_PAGE = 3
-- Jarak antar tombol mode
local BUTTON_GAP = 20

-- Data teks tiap mode (index = id mode + 1)
local function buildModes()
    local zh = CCApplication:sharedApplication():getCurrentLanguage() ==
                   kLanguageChinese
    local list
    if zh then
        list = {
            {'1 VS 1', ''}, {'2 VS 2', ''}, {'3 VS 3', '经典模式'},
            {'4 VS 4', ''}, {'硬核模式 (4 VS 4)', '禁用装备'},
            {'克隆模式 (3 VS 3)', ''}, {'死亡竞赛 (3 VS 3)', ''},
            {'随机死亡竞赛 (3 VS 3)', ''}
        }
    else
        list = {
            {'1 VS 1', ''}, {'2 VS 2', ''}, {'3 VS 3', 'Classic Mode'},
            {'4 VS 4', ''}, {'HardCore (4 VS 4)', 'Disabled gear'},
            {'Clone (3 VS 3)', ''}, {'Deathmatch (3 VS 3)', ''},
            {'Random Deathmatch (3 VS 3)', ''}
        }
    end

    local modes = {}
    for i, v in ipairs(list) do
        modes[i] = {
            title = v[1],
            description = v[2],
            isLocked = false,
            useMask2 = false,
            hasSelected = false
        }
    end

    -- mode yang masih dalam pengembangan
    modes[MODE.Deathmatch + 1].isLocked = true
    modes[MODE.Deathmatch + 1].useMask2 = true

    for _, data in ipairs(modes) do
        if data.isLocked then
            data.description = data.description .. ' (In developtment)'
        end
    end
    return modes
end

local function fullScreenSprite(sprite)
    sprite:setScaleX(display.width / sprite:getContentSize().width)
end

function GameModeLayer:init()
    log('Initial GameModeLayer...')

    -- startmenu_title.png ada di Menu.plist
    tools.addSprites('Menu.plist')

    self.currentPage = 0
    self.entered = false
    self.modes = buildModes()
    self.modeItems = {}

    -- background
    local bg = display.newSprite('red_bg.png')
    fullScreenSprite(bg)
    bg:setAnchorPoint(CCPoint(0, 0))
    bg:setPosition(0, 0)
    self:addChild(bg, -100)

    -- menu bars
    local menu_bar_b = display.newSprite('menu_bar2.png')
    menu_bar_b:setAnchorPoint(CCPoint(0, 0))
    fullScreenSprite(menu_bar_b)
    self:addChild(menu_bar_b, 2)

    local menu_bar_t = display.newSprite('menu_bar3.png')
    menu_bar_t:setAnchorPoint(CCPoint(0, 0))
    menu_bar_t:setPosition(0, display.height -
                               menu_bar_t:getContentSize().height)
    fullScreenSprite(menu_bar_t)
    self:addChild(menu_bar_t, 2)

    local title = display.newSprite('#startmenu_title.png')
    title:setAnchorPoint(CCPoint(0, 0))
    title:setPosition(2, display.height - title:getContentSize().height - 2)
    self:addChild(title, 3)

    -- tombol mode (dibuat sekali, posisinya diatur per halaman di showPage)
    local count = math.min(GameModeLayer:getModeCount(), #self.modes)
    self.modeCount = count

    local items = {}
    for i = 1, count do
        local id = i - 1
        local data = self.modes[i]

        local item = ui.newImageMenuItem({
            image = GameModeLayer:getModeImage(id),
            listener = function() self:onModeClick(id) end
        })
        item:setAnchorPoint(0.5, 0.5)

        if data.isLocked then
            local mask = display.newSprite(data.useMask2 and
                                               'GameMode/chain_mask2.png' or
                                               'GameMode/chain_mask.png')
            local size = item:getContentSize()
            mask:setPosition(size.width / 2, size.height / 2)
            item:addChild(mask, 1)
        end

        self.modeItems[i] = item
        items[#items + 1] = item
    end

    self.modeMenu = ui.newMenu(items)
    self.modeMenu:setPosition(0, 0)
    self:addChild(self.modeMenu)

    -- label nama mode
    self.menuLabel = CCLabelTTF:create('', 'Arial', 12)
    self.menuLabel:setAnchorPoint(CCPoint(0, 0))
    self.menuLabel:setPosition(10, 2)
    self:addChild(self.menuLabel, 5)

    -- tombol Page Prev / Page Next
    local arrowY = display.height / 2 + 30

    local prev_btn = ui.newImageMenuItem({
        image = 'Img/Prev.png',
        imageSelected = 'Img/Prev_1.png',
        listener = handler(self, GameModeLayer.prevPage)
    })
    prev_btn:setAnchorPoint(0.5, 0.5)
    prev_btn:setScale(0.60)
    prev_btn:setPosition(45, arrowY)
    local mvPrev = CCMoveBy:create(1.0, CCPoint(-8, 0))
    prev_btn:runAction(CCRepeatForever:create(
                           transition.sequence({mvPrev, mvPrev:reverse()})))

    local next_btn = ui.newImageMenuItem({
        image = 'Img/Next.png',
        imageSelected = 'Img/Next_1.png',
        listener = handler(self, GameModeLayer.nextPage)
    })
    next_btn:setAnchorPoint(0.5, 0.5)
    next_btn:setScale(0.60)
    next_btn:setPosition(display.width - 45, arrowY)
    local mvNext = CCMoveBy:create(1.0, CCPoint(8, 0))
    next_btn:runAction(CCRepeatForever:create(
                           transition.sequence({mvNext, mvNext:reverse()})))

    local page_menu = ui.newMenu({prev_btn, next_btn})
    page_menu:setPosition(0, 0)
    self:addChild(page_menu, 10)

    self:showPage(0)

    -- tombol kembali
    local return_btn = ui.newImageMenuItem({
        image = 'UI/return_btn.png',
        listener = handler(self, GameModeLayer.backToMenu)
    })
    local return_menu = ui.newMenu({return_btn})
    return_menu:setPosition(display.width - 38, 65)
    self:addChild(return_menu, 5)
end

function GameModeLayer:backToMenu()
    audio.playSound('Audio/Menu/cancel.ogg')

    local menuScene = CCScene:create()
    local menuLayer = StartMenu:create()

    hook.registerInitHandlerOnly(menuLayer)
    menuScene:addChild(menuLayer)
    director.replaceSceneWithFade(menuScene, 1.0)
end

function GameModeLayer:getPageCount()
    return math.ceil(self.modeCount / MODES_PER_PAGE)
end

-- Navigasi halaman (loop): prev dari halaman pertama -> halaman terakhir,
-- next dari halaman terakhir -> halaman pertama
function GameModeLayer:prevPage()
    local count = self:getPageCount()
    self:showPage((self.currentPage + count - 1) % count)
end

function GameModeLayer:nextPage()
    local count = self:getPageCount()
    self:showPage((self.currentPage + 1) % count)
end

function GameModeLayer:showPage(page)
    self.currentPage = page

    local first = page * MODES_PER_PAGE + 1 -- index 1-based
    local last = math.min(first + MODES_PER_PAGE - 1, self.modeCount)

    -- lebar tombol berbeda-beda, jadi spasi dihitung dari lebar sebenarnya
    local totalWidth = BUTTON_GAP * (last - first)
    for i = first, last do
        totalWidth = totalWidth + self.modeItems[i]:getContentSize().width
    end

    local posY = display.height / 2 + 30
    local x = (display.width - totalWidth) / 2

    for i = 1, self.modeCount do
        self.modeItems[i]:setVisible(i >= first and i <= last)
    end

    for i = first, last do
        local item = self.modeItems[i]
        local w = item:getContentSize().width
        item:setPosition(x + w / 2, posY)
        x = x + w + BUTTON_GAP
    end

    self.menuLabel:setString('')
end

-- Tap pertama: pilih mode (tampilkan nama). Tap kedua pada mode yang sama:
-- konfirmasi dan lanjut ke SelectLayer (kecuali mode terkunci).
function GameModeLayer:onModeClick(id)
    if self.entered then return end

    audio.playSound(ns.menu.CONFIRM)

    local data = self.modes[id + 1]

    local label = data.title
    if #data.description > 0 then label = label .. ' | ' .. data.description end
    self.menuLabel:setString(label)

    if not data.isLocked and data.hasSelected then
        self.entered = true -- cegah masuk dua kali
        log('Selected mode %d', id)
        GameModeLayer:startMode(id)
        return
    end

    for _, m in ipairs(self.modes) do m.hasSelected = false end
    data.hasSelected = true
end
