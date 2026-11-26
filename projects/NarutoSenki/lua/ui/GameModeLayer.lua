GameModeLayer = class('GameModeLayer', function()
    return display.newLayer()
end)

-- Jumlah mode per halaman (sama dengan GameModeLayer::kModesPerPage di C++)
local MODES_PER_PAGE = 3
local BUTTON_GAP = 20

-- Daftar mode, urut sesuai halaman. images = kandidat gambar (dipakai yang pertama ada).
local MODE_LIST = {
    {mode = GameMode.TwoVsTwo,         images = {'GameMode/2v2.png''}},
    {mode = GameMode.Classic,          images = {'GameMode/3v3.png'}},
    {mode = GameMode.FourVsFour,       images = {'GameMode/4v4.png'}},
    {mode = GameMode.OneVsOne,         images = {'GameMode/1v1.png'}},
    {mode = GameMode.HardCore_4Vs4,    images = {'GameMode/HardCore.png'}},
    {mode = GameMode.Clone,            images = {'GameMode/Clone.png'}},
    {mode = GameMode.Deathmatch,       images = {'GameMode/Deathmatch.png'}, locked = true},
    {mode = GameMode.RandomDeathmatch, images = {'GameMode/RandomDeathmatch.png'}},
}

local function pickImage(candidates)
    local fu = CCFileUtils:sharedFileUtils()
    for _, path in ipairs(candidates) do
        if fu:isFileExist(fu:fullPathForFilename(path)) then return path end
    end
    return 'GameMode/Mode.png'
end

-- Pilih mode: WAJIB lewat GameModeApi.select supaya C++ (s_GameMode + handler)
-- ikut berubah. _G.mode saja hanya mengatur tampilan SelectLayer.lua.
-- Urutannya sama dengan GameModeLayer::selectMode di C++.
local function selectMode(mode)
    audio.playSound('Audio/Menu/confirm.ogg')
    -- aturan pilih-tim-sendiri sama persis dengan C++ (Cheats, flag dari kotak HardCore)
    local customSelect = GameModeApi.customSelect(mode)
    enterSelectLayer(mode, customSelect) -- isi _G.mode + _G.enableCustomSelect, lalu pindah scene
    GameModeApi.select(mode)             -- atur mode di C++ (Cheats, enableGear, dst.)
end

function GameModeLayer:create()
    audio.playSound('Audio/Menu/confirm.ogg')
    local layer = GameModeLayer.new()
    if layer then
        layer:init()
    else
        print('Failed to create GameModeLayer instance')
    end
    return layer
end

function GameModeLayer:init()
    local bg = display.newSprite('blue_bg.png')
    bg:setAnchorPoint(0, 0)
    bg:fullScreen()
    self:addChild(bg, -1)

    -- Tombol mode (semua dibuat sekali, posisi/visibilitas diatur per halaman)
    self._items = {}
    local items = {}
    for i, def in ipairs(MODE_LIST) do
        local item = ui.newImageMenuItem({
            image = pickImage(def.images),
            listener = function()
                if def.locked then
                    audio.playSound('Audio/Menu/cancel.ogg')
                    return
                end
                selectMode(def.mode)
            end
        })
        self._items[i] = item
        items[#items + 1] = item
    end

    local menu = ui.newMenu(items)
    menu:setPosition(0, 0)
    self:addChild(menu, 1)

    -- Penanda halaman "1/3"
    self._pageLabel = CCLabelTTF:create('', 'Fonts/Roboto-Medium.ttf', 12)
    self._pageLabel:setPosition(display.cx, display.cy - 45)
    self:addChild(self._pageLabel, 5)

    -- Tombol PREV / NEXT (loop)
    local arrowY = display.cy + 30

    local prev_btn = ui.newImageMenuItem({
        image = 'Img/Prev.png',
        imageSelected = 'Img/Prev_1.png',
        listener = function() self:prevPage() end
    })
    prev_btn:setScale(0.60)
    prev_btn:setPosition(45, arrowY)
    local mvPrev = CCMoveBy:create(1, CCPoint(-8, 0))
    prev_btn:runAction(CCRepeatForever:create(
                           transition.sequence({mvPrev, mvPrev:reverse()})))

    local next_btn = ui.newImageMenuItem({
        image = 'Img/Next.png',
        imageSelected = 'Img/Next_1.png',
        listener = function() self:nextPage() end
    })
    next_btn:setScale(0.60)
    next_btn:setPosition(display.width - 45, arrowY)
    local mvNext = CCMoveBy:create(1, CCPoint(8, 0))
    next_btn:runAction(CCRepeatForever:create(
                           transition.sequence({mvNext, mvNext:reverse()})))

    local page_menu = ui.newMenu({prev_btn, next_btn})
    page_menu:setPosition(0, 0)
    self:addChild(page_menu, 10)

    -- Tombol kembali (backToStartMenu ada di SelectLayer.lua)
    local return_btn = ui.newImageMenuItem({
        image = 'UI/return_btn.png',
        listener = function() backToStartMenu() end
    })
    return_btn:setPosition(display.width - 38, 65)
    local return_menu = ui.newMenu({return_btn})
    return_menu:setPosition(0, 0)
    self:addChild(return_menu, 5)

    self._page = 0
    self:showPage(0)
end

function GameModeLayer:getPageCount()
    return math.ceil(#MODE_LIST / MODES_PER_PAGE)
end

function GameModeLayer:prevPage()
    audio.playSound('Audio/Menu/confirm.ogg')
    local count = self:getPageCount()
    self:showPage((self._page + count - 1) % count) -- loop ke halaman terakhir
end

function GameModeLayer:nextPage()
    audio.playSound('Audio/Menu/confirm.ogg')
    self:showPage((self._page + 1) % self:getPageCount()) -- loop ke halaman pertama
end

function GameModeLayer:showPage(page)
    self._page = page
    local first = page * MODES_PER_PAGE + 1
    local last = math.min(first + MODES_PER_PAGE - 1, #MODE_LIST)

    -- lebar tombol berbeda-beda, jadi spasi dihitung dari lebar sebenarnya
    local totalWidth = 0
    for i = first, last do
        totalWidth = totalWidth + self._items[i]:getContentSize().width
    end
    totalWidth = totalWidth + BUTTON_GAP * (last - first)

    local posY = display.cy + 30
    local x = (display.width - totalWidth) / 2

    for i, item in ipairs(self._items) do
        local show = i >= first and i <= last
        item:setVisible(show)
        item:setEnabled(show) -- tombol yang tersembunyi tidak boleh bisa disentuh
    end

    for i = first, last do
        local item = self._items[i]
        local w = item:getContentSize().width
        item:setPosition(x + w / 2, posY)
        x = x + w + BUTTON_GAP
    end

    self._pageLabel:setString(string.format('%d / %d', page + 1, self:getPageCount()))
end

-- Dipanggil C++ (StartMenu::onTrainingCallBack) lewat UiFlowKeys::kEnterGameModeLayer
function enterGameModeLayer()
    local scene = CCScene:create()
    scene:addChild(GameModeLayer:create())
    director.replaceSceneWithFade(scene, 1.25)
end
