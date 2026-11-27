--
-- MapLayer
-- Layar pilih map (dibuka dari SelectLayer setelah tombol Start ditekan).
-- Panah kiri/kanan = ganti map (loop). Teks "MapsN" hanya menampilkan pilihan.
-- Tombol Start = konfirmasi map lalu lanjut ke SelectLayer:onGameStart().
-- Tombol return = kembali ke layar pilih hero.
--

MapLayer = class('MapLayer', function() return display.newLayer() end)

local MAP_FONT = 'Fonts/njNaruto.ttf'
local MAP_SIZE = 42

-- Jumlah map = jumlah Maps/N.tmx berurutan dari 1 (sama dengan GameLayer::getMapCount)
local function countMaps()
    local ok, n = pcall(function()
        local fu = CCFileUtils:sharedFileUtils()
        local c = 0
        while fu:isFileExist('Maps/' .. (c + 1) .. '.tmx') do c = c + 1 end
        return c
    end)
    if ok and n and n > 0 then return n end
    return 7 -- cadangan kalau pengecekan file tidak tersedia
end

function MapLayer:ctor(selectLayer)
    self._selectLayer = selectLayer
    self._mapCount = countMaps()
    self._current = 1
    self._confirmed = false
    self._ready = false
    self:init()
end

function MapLayer:create(selectLayer) return MapLayer.new(selectLayer) end

function MapLayer:init()
    log('Initial MapLayer')
    local width, height = display.width, display.height

    local bgSprite = display.newSprite('red_bg.png', 0, 0)
    bgSprite:setAnchorPoint(0, 0)
    bgSprite:fullScreen()
    self:addChild(bgSprite, -100)

    local menu_bar_b = display.newSprite('menu_bar2.png')
    menu_bar_b:setAnchorPoint(0, 0)
    menu_bar_b:fullScreen()
    self:addChild(menu_bar_b, 2)

    local menu_bar_t = display.newSprite('menu_bar3.png')
    menu_bar_t:setAnchorPoint(0, 0)
    menu_bar_t:setPosition(0, height - menu_bar_t:getContentSize().height)
    menu_bar_t:fullScreen()
    self:addChild(menu_bar_t, 2)

    local title = display.newSprite('#startmenu_title.png')
    title:setAnchorPoint(0, 0)
    title:setPosition(2, height - title:getContentSize().height - 2)
    self:addChild(title, 3)

    -- teks "MapsN" (hanya tampilan, tidak bisa ditekan)
    self._mapLabel = ui.newTTFLabel({
        text = 'Maps1', font = MAP_FONT, size = MAP_SIZE})
    self._mapLabel:setAnchorPoint(0.5, 0.5)
    self._mapLabel:setPosition(width / 2, height / 2 + 30)
    self:addChild(self._mapLabel, 5)

    local prevItem = ui.newImageMenuItem({
        image = 'Img/Prev.png', imageSelected = 'Img/Prev_1.png',
        listener = handler(self, MapLayer.onPrev)})
    prevItem:setScale(0.60)
    local nextItem = ui.newImageMenuItem({
        image = 'Img/Next.png', imageSelected = 'Img/Next_1.png',
        listener = handler(self, MapLayer.onNext)})
    nextItem:setScale(0.60)

    local midY = height / 2 + 30
    prevItem:setPosition(width / 2 - 150, midY)
    nextItem:setPosition(width / 2 + 150, midY)
    local mvPrev = CCMoveBy:create(1, CCPoint(-8, 0))
    prevItem:runAction(CCRepeatForever:create(
                           transition.sequence({mvPrev, mvPrev:reverse()})))
    local mvNext = CCMoveBy:create(1, CCPoint(8, 0))
    nextItem:runAction(CCRepeatForever:create(
                           transition.sequence({mvNext, mvNext:reverse()})))

    local menu = ui.newMenu({prevItem, nextItem})
    menu:setPosition(0, 0)
    self:addChild(menu, 5)

    -- tombol Start (posisi sama seperti di SelectLayer): konfirmasi map -> onGameStart.
    -- Baru aktif setelah 0.5 detik supaya ketukan dari tombol Start di SelectLayer
    -- (atau ketukan ganda) tidak langsung melewati pemilihan map.
    local start_btn = ui.newImageMenuItem({
        image = '#start_btn.png',
        listener = handler(self, MapLayer.onConfirm)})
    local smenu = ui.newMenu({start_btn})
    smenu:setAnchorPoint(0, 0)
    smenu:setPosition(width - 40, 36)
    self:addChild(smenu, 5)

    self._ready = false
    self:runAction(transition.sequence({
        CCDelayTime:create(0.5),
        CCCallFunc:create(function() self._ready = true end)
    }))

    -- tombol return (sama seperti SkillLayer)
    local return_btn = ui.newImageMenuItem({
        image = display.newSprite('#return_btn.png'),
        listener = handler(self, MapLayer.onCancel)})
    local rmenu = ui.newMenu({return_btn})
    rmenu:setPosition(width - 38, 86)
    self:addChild(rmenu, 5)

    return true
end

function MapLayer:refresh()
    self._mapLabel:setString('Maps' .. self._current)
end

function MapLayer:onPrev()
    audio.playSound('Audio/Menu/confirm.ogg')
    -- Maps1 -> map terakhir
    self._current = (self._current - 2 + self._mapCount) % self._mapCount + 1
    self:refresh()
end

function MapLayer:onNext()
    audio.playSound('Audio/Menu/confirm.ogg')
    -- map terakhir -> Maps1
    self._current = self._current % self._mapCount + 1
    self:refresh()
end

function MapLayer:onCancel()
    if self._confirmed then return end
    audio.playSound('Audio/Menu/cancel.ogg')
    director.popScene()
end

-- Map terpilih: simpan (dibaca C++ GameLayer::initTileMap), kembali ke
-- SelectLayer lalu teruskan ke tujuan asli (SelectLayer:onGameStart).
function MapLayer:onConfirm()
    if self._confirmed or not self._ready then return end
    self._confirmed = true

    save.integer('selectedMap', self._current)
    audio.playSound('Audio/Menu/confirm.ogg')

    local selectLayer = self._selectLayer
    director.popScene()
    if selectLayer then selectLayer:onGameStart() end
end
