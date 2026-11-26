--
-- SelectLayer
--

local HeroData = require('ui.HeroData')

-- Nama hero memakai font njNaruto.ttf (pengganti sprite <hero>_font.png)
local NAME_FONT = 'Fonts/njNaruto.ttf'
local NAME_SIZE = 28
local NAME_SCALE = 0.60

-- Title (mis. "(AKATSUKI)") di bawah nama
local TITLE_FONT = 'Fonts/ReggaeOne.ttf'
local TITLE_SIZE = 25
local TITLE_SCALE = 0.40
local SHOW_TITLE = true

local OFFSETS8 = {{1,0},{-1,0},{0,1},{0,-1},{1,1},{-1,1},{1,-1},{-1,-1}}
local OFFSETS4 = {{1,0},{-1,0},{0,1},{0,-1}}

-- Nama tampilan: Klan + nama (HeroData.heroDisplayName / cleanHeroName)
local function displayName(charName)
    local base = HeroData.heroDisplayName[charName]
    if not base then
        base = HeroData.cleanHeroName(charName)
        base = base:gsub('(%l)(%u)', '%1 %2') -- "FourthRaikage" -> "Fourth Raikage"
    end
    local clan = HeroData.heroClan[HeroData.cleanHeroName(charName)]
    if clan then return clan .. ' ' .. base end
    return base
end

local function addOutlinedLabel(node, text, font, size, scale, offsets, x, y, strokeOpacity)
    for _, o in ipairs(offsets) do
        local stroke = CCLabelTTF:create(text, font, size)
        stroke:setColor(ccc3(0, 0, 0))
        stroke:setScale(scale)
        stroke:setAnchorPoint(0.5, 0.5)
        stroke:setPosition(x + o[1], y + o[2])
        if strokeOpacity then stroke:setOpacity(strokeOpacity) end
        node:addChild(stroke)
    end
    local label = CCLabelTTF:create(text, font, size)
    label:setColor(ccc3(255, 255, 255))
    label:setScale(scale)
    label:setAnchorPoint(0.5, 0.5)
    label:setPosition(x, y)
    node:addChild(label)
end

-- Satu node berisi nama (+ title); hapus node = hapus semuanya
local function newHeroNameLabel(charName, x, y)
    local node = CCNode:create()
    node:setPosition(x, y)

    addOutlinedLabel(node, displayName(charName), NAME_FONT, NAME_SIZE,
                     NAME_SCALE, OFFSETS8, 0, 8)

    if SHOW_TITLE then
        local rawTitle = HeroData.heroTitle[charName] or
                             HeroData.heroTitle[HeroData.cleanHeroName(charName)]
        if rawTitle then
            addOutlinedLabel(node, '(' .. string.upper(rawTitle) .. ')',
                             TITLE_FONT, TITLE_SIZE, TITLE_SCALE, OFFSETS4,
                             0, -4, 200)
        end
    end

    return node
end

-- Gambar half karakter: load langsung dari Img/Half/<Hero>_half.png (bukan plist)
local HALF_SCALE = 1.0
-- Skala khusus per hero (mengalahkan HALF_SCALE)
local HALF_SCALE_BY_HERO = {
    Chiyo = 0.4,
}
local function newHeroHalfImage(heroName)
    local sprite = display.newSprite('Img/Half/' .. heroName .. '_half.png', 10, 10)
    sprite:setAnchorPoint(CCPoint(0, 0))
    sprite:setScale(HALF_SCALE_BY_HERO[heroName] or HALF_SCALE)
    return sprite
end

-- Ganti gambar half (dipakai SkillLayer saat ganti page skill)
function SelectLayer:setHeroHalf(heroName)
    if self._heroHalfImage then self._heroHalfImage:removeFromParent() end
    self._heroHalfImage = newHeroHalfImage(heroName)
    self:addChild(self._heroHalfImage, 1)
end

function SelectLayer:init()
    log('Initial SelectLayer...')

    self.mode = _G.mode
    self.enableCustomSelect = _G.enableCustomSelect
    if self.mode == GameMode.Clone or self.mode == GameMode.OneVsOne then
        self.is3v3Mode = false
        self.is4v4Mode = false
    else
        self.is3v3Mode = self.mode == GameMode.Classic or self.mode ==
                             GameMode.RandomDeathmatch
        self.is4v4Mode = self.mode == GameMode.FourVsFour or self.mode ==
                             GameMode.HardCore_4Vs4
    end
    self.is2v2Mode = self.mode == GameMode.TwoVsTwo

    -- Jumlah slot rekan tim (COM) yang bisa dipilih
    self.comCount = self.is4v4Mode and 3 or
                        (self.is3v3Mode and 2 or (self.is2v2Mode and 1 or 0))

    -- Jumlah slot musuh yang bisa dipilih (sesuai jumlah musuh tiap mode)
    if self.mode == GameMode.OneVsOne or self.mode == GameMode.Clone then
        self.enemyCount = 1
    elseif self.is2v2Mode then
        self.enemyCount = 2
    elseif self.is4v4Mode then
        self.enemyCount = 4
    elseif self.mode == GameMode.Classic or self.mode == GameMode.Deathmatch then
        self.enemyCount = 3
    else
        self.enemyCount = 0
    end

    -- RandomDeathmatch: semua hero diacak, tidak ada pilihan
    if self.mode == GameMode.RandomDeathmatch then
        self.enableCustomSelect = false
    end
    if self.comCount == 0 and self.enemyCount == 0 then
        self.enableCustomSelect = false
    end

    self._enemySelectors = {}
    self._enemySelects = {}
    -- Bersihkan pilihan musuh dari sesi sebelumnya (dibaca oleh C++)
    for i = 1, 4 do save.string('customEnemy' .. i, '') end

    self.enableHardCore = false

    -- 21 karakter per halaman (3 baris x 7 kolom); jumlah halaman dihitung otomatis
    self.PER_PAGE = 21
    self.currentPage = 1
    self.pageNum = math.max(1, math.ceil(#ns.CharactersLayout / self.PER_PAGE))

    self._comLabel1 = nil
    self._comLabel2 = nil
    self._comLabel3 = nil
    self._com1Select = nil
    self._com2Select = nil
    self._com3Select = nil

    self._heroName = nil
    self._heroHalfImage = nil

    self.pageLayers = {}
    self.pageButtons = {}

    self._playerSelect = nil
    self.selectHero = nil

    -- Pilihan page skill (heroPageIdx_<Hero> = index page, heroPage_<Hero> = nama varian) sengaja TIDAK direset di sini,
    -- jadi tetap tersimpan setelah keluar/selesai pertandingan.

    tools.addSprites('Record.plist', 'Record2.plist', 'Select.plist',
                     'UI.plist', 'Avator.plist', 'Report.plist', 'Ougis.plist', 'Ougis2.plist',
                     'Map.plist', 'Gears.plist')

    -- if CC_TARGET_PLATFORM == CC_PLATFORM_ANDROID then
    -- if not KTools:checkMD5() then
    -- 	SimpleAudioEngine:sharedEngine():stopBackgroundMusic()
    -- 	CCDirector:sharedDirector():end()
    -- 	return false
    -- end
    -- end

    local width, height = display.width, display.height

    local bg_src = self.enableCustomSelect and 'red_bg.png' or 'blue_bg.png'
    local bgSprite = display.newSprite(bg_src, 0, 0)
    bgSprite:setAnchorPoint(0, 0)
    bgSprite:fullScreen()
    self:addChild(bgSprite, -5)

    -- produce groud
    local gold_left = display.newSprite('#gold_left.png', 0, 20)
    gold_left:setAnchorPoint(0, 0)
    self:addChild(gold_left, 1)

    local gold_right = display.newSprite('#gold_right.png')
    gold_right:setAnchorPoint(0, 1)
    gold_right:setPosition(width - gold_right:getContentSize().width - 20,
                           height - 20)
    self:addChild(gold_right, 1)

    -- produce the cloud
    local cloud_left = display.newSprite('#cloud.png', 0, 15)
    cloud_left:setAnchorPoint(0, 0)
    cloud_left:setFlipX(true)
    cloud_left:setFlipY(true)
    self:addChild(cloud_left, 2)

    local cmv1 = CCMoveBy:create(1, CCPoint(-15, 0))
    local cseq1 = CCRepeatForever:create(
                      transition.sequence({cmv1, cmv1:reverse()}))
    cloud_left:runAction(cseq1)

    local cloud_right = display.newSprite('#cloud.png')
    cloud_right:setAnchorPoint(0, 0)
    cloud_right:setPosition(width - cloud_right:getContentSize().width, 15)
    cloud_right:setFlipY(true)
    self:addChild(cloud_right, 2)

    local cmv2 = CCMoveBy:create(1, CCPoint(15, 0))
    local cseq2 = CCRepeatForever:create(
                      transition.sequence({cmv2, cmv2:reverse()}))
    cloud_right:runAction(cseq2)

    local cloud_top = display.newSprite('#cloud_top.png')
    cloud_top:setAnchorPoint(0.5, 0)
    cloud_top:setPosition(width / 2,
                          height - (cloud_top:getContentSize().height + 12))
    self:addChild(cloud_top, 1)

    -- produce the menu_bar
    local menu_bar_b = display.newSprite('menu_bar2.png')
    menu_bar_b:setAnchorPoint(0, 0)
    menu_bar_b:fullScreen()
    self:addChild(menu_bar_b, 2)

    local menu_bar_t = display.newSprite('menu_bar3.png')
    menu_bar_t:setAnchorPoint(0, 0)
    menu_bar_t:setPosition(0, height - menu_bar_t:getContentSize().height)
    menu_bar_t:fullScreen()
    self:addChild(menu_bar_t, 2)

    local select_title = display.newSprite('#hcmode_title.png')
    select_title:setAnchorPoint(0, 0)
    select_title:setPosition(2,
                             height - select_title:getContentSize().height - 2)
    self:addChild(select_title, 3)

    local selectNameList = {}
    local selectButtons = {}

    for i = 1, self.pageNum do
        -- initial page list
        local pageLayer = TouchGroup:create() -- display.newLayer()

        self:addChild(pageLayer, 5)
        if i == self.currentPage then
            pageLayer:show()
        else
            pageLayer:hide()
            pageLayer:setPositionY(2000)
        end

        table.insert(self.pageLayers, pageLayer)
    end

    local charactersList = ns.CharactersLayout
    local pageNum = #self.pageLayers

    for i = 1, #charactersList do
        local charName = charactersList[i]

        selectNameList[#selectNameList + 1] = charName

        local idx = i - 1
        local Column = idx % 7
        local Row = toint(idx / 7)
        local Page = toint(idx / self.PER_PAGE)
        -- log(
        --     ' -> load character %s\t[ Column: %d Row: %d Page: %d ] [ pos: %f, %f ]',
        --     charName, Column, Row, Page,
        --     width / 2 - 36 + (Column - 1) * 27 + Column / 4 * 10,
        --     height - 112 - (72 * (Row - 3 * Page)))

        local select_btn = SelectButton:create('Img/Select/' .. charName .. '_select.png', true, 0.5)
        select_btn._selectLayer = self
        select_btn._charName = charName
        --           -- LAYOUT --
        -- --- LEFT ---        --- RIGHT ---
        -- 1, 2, 3, 4, padding 5, 6, 7
        local groupPadding = toint(Column / 4) * 10
        select_btn:setPosition(CCPoint(width / 2 - 36 + (Column - 1) * 27 +
                                           groupPadding,
                                       height - 112 - (72 * (Row - 3 * Page))))

        if Page + 1 > pageNum then
            log('Select page index out of range')
            break
        end
        self.pageLayers[Page + 1]:addWidget(select_btn)

        selectButtons[#selectButtons + 1] = select_btn
    end

    -- Tombol PREV / NEXT (loop: dari halaman 1 ke belakang -> halaman terakhir)
    local menuList = {}
    local btnY = 42

    local prev_btn = ui.newImageMenuItem({
        image = 'Img/Prev.png',
        imageSelected = 'Img/Prev_1.png',
        listener = handler(self, SelectLayer.onPagePrev)
    })
    prev_btn:setAnchorPoint(0.5, 0.5)
    prev_btn:setScale(0.60)
    prev_btn:setPosition(width / 2 - 20, btnY)
    table.insert(menuList, prev_btn)
    self._prevBtn = prev_btn

    local next_btn = ui.newImageMenuItem({
        image = 'Img/Next.png',
        imageSelected = 'Img/Next_1.png',
        listener = handler(self, SelectLayer.onPageNext)
    })
    next_btn:setAnchorPoint(0.5, 0.5)
    next_btn:setScale(0.60)
    next_btn:setPosition(width / 2 + 80, btnY)
    table.insert(menuList, next_btn)
    self._nextBtn = next_btn

    -- animasi gerak kecil seperti contoh
    local mvPrev = CCMoveBy:create(1, CCPoint(-8, 0))
    prev_btn:runAction(CCRepeatForever:create(
                           transition.sequence({mvPrev, mvPrev:reverse()})))
    local mvNext = CCMoveBy:create(1, CCPoint(8, 0))
    next_btn:runAction(CCRepeatForever:create(
                           transition.sequence({mvNext, mvNext:reverse()})))

    local page_menu = ui.newMenu(menuList)
    page_menu:setAnchorPoint(0, 0)
    page_menu:setPosition(0, 0)
    self:addChild(page_menu, 10)

    self.selectButtons = selectButtons
    self.selectNameList = selectNameList
    self:setSelectList(table.toCCArray(selectNameList))

    local hero = selectNameList[1]
    if hero then
        local heroHalfImage = newHeroHalfImage(hero)
        self:addChild(heroHalfImage, 1)

        local heroName = newHeroNameLabel(hero, 100, 20)
        self:addChild(heroName, 5)

        self._heroName = heroName
        self._heroHalfImage = heroHalfImage
    end

    local selectBtn = selectButtons[1]
    local selectImg = display.newSprite('#1p.png', selectBtn:getPositionX() - 2,
                                        selectBtn:getPositionY() - 2)
    selectImg:setAnchorPoint(0, 0)
    self:addChild(selectImg, 500)
    local blink = CCBlink:create(0.6, 1)
    selectImg:runAction(CCRepeatForever:create(blink))
    self._selectImg = selectImg
    self.selectHero = selectBtn._charName

    if self.enableCustomSelect then
        if self.is2v2Mode or self.is3v3Mode or self.is4v4Mode then
            self:initCustomSelectMode()
        end
        if self.enemyCount > 0 then self:initEnemySelectMode() end
    end

    local ranking_btn = ui.newImageMenuItem({
        image = '#ranking_btn.png',
        listener = handler(self, SelectLayer.onRankingButtonClick)
    })
    ranking_btn:setAnchorPoint(1, 0.5)
    local menu3 = ui.newMenu({ranking_btn})
    menu3:setPosition(width - 15, height - 34)
    self:addChild(menu3, 5)

    local start_btn = ui.newImageMenuItem({
        image = '#start_btn.png',
        -- call c++ layer function
        listener = function() self:onGameStart() end
    })
    local menu = ui.newMenu({start_btn})
    menu:setAnchorPoint(0, 0)
    menu:setPosition(width - 40, 36)
    self:addChild(menu, 5)

    local skill_btn = ui.newImageMenuItem({
        image = '#skill_btn.png',
        listener = handler(self, SelectLayer.onSkillMenuButtonClick)
    })
    local menu2 = ui.newMenu({skill_btn})
    menu2:setAnchorPoint(0, 0)
    menu2:setPosition(width - 35, 96)
    self:addChild(menu2, 5)

    -- Desktop return button
    if _G.platform == 'desktop' then
        local skill_btn = ui.newImageMenuItem({
            image = '#return_btn.png',
            listener = backToStartMenu
        })
        local menu3 = ui.newMenu({skill_btn})
        menu3:setAnchorPoint(0, 0)
        menu3:setPosition(width - 35, 135)
        self:addChild(menu3, 5)
    end

    if save.isBGM() then audio.playMusic(ns.music.SELECT_MUSIC, true) end
end

function SelectLayer:initCustomSelectMode()
    log('Initial Custom Select Mode...')
    local teamSelector = display.newLayer()
    -- local uiLayer = TouchGroup:create()
    -- teamSelector:addChild(uiLayer, 5)

    -- local hardcore_switch_btn = CheckBox:create()
    -- hardcore_switch_btn:loadTextures( -- load textures
    -- "check_bg.png", "check_bg.png", "check.png", "", "", ns.UITexType.Local)
    -- -- "", -- backGround
    -- -- "", -- backGroundSelected
    -- -- "", -- cross
    -- -- "", -- backGroundDisabled
    -- -- "", -- frontCrossDisabled
    -- -- ns.UITexType.Plist)
    -- hardcore_switch_btn:setPosition(
    --     CCPoint(text:getContentSize().width + 5, 260))
    -- hardcore_switch_btn:setScale(0.4)
    -- hardcore_switch_btn:addEventListenerCheckBox(
    --     function(_, event)
    --         self.enableHardCore = event == ns.CheckBoxEventType.selected
    --         -- log('Is enable hardcore [ %s ]', tostring(self.enableHardCore))
    --     end)
    -- uiLayer:addWidget(hardcore_switch_btn)

    local teamBg = display.newSprite('#team_bg.png', 0, 185)
    teamBg:setAnchorPoint(0, 0)
    teamSelector:addChild(teamBg)

    -- init com selector 1
    local comSelector1 = display.newSprite('#unknow_select.png', 2, 194)
    comSelector1:setAnchorPoint(0, 0)
    teamSelector:addChild(comSelector1)
    self._comSelector1 = comSelector1

    self._comLabel1 = display.newSprite('#com_label.png')
    self._comLabel1:setPosition(comSelector1:getPositionX() +
                                    comSelector1:getContentSize().width + 2 + 18,
                                comSelector1:getPositionY() +
                                    comSelector1:getContentSize().height / 2)
    teamSelector:addChild(self._comLabel1)

    -- init com selector 2
    local comSelector2 = display.newSprite('#unknow_select.png')
    comSelector2:setAnchorPoint(0, 0)
    comSelector2:setPosition(comSelector1:getPositionX() +
                                 comSelector1:getContentSize().width + 40,
                             comSelector1:getPositionY())
    teamSelector:addChild(comSelector2)

    self._comLabel2 = display.newSprite('#com_label.png')
    self._comLabel2:setPosition(comSelector2:getPositionX() +
                                    comSelector2:getContentSize().width + 2 + 18,
                                comSelector2:getPositionY() +
                                    comSelector2:getContentSize().height / 2)
    teamSelector:addChild(self._comLabel2)
    self._comSelector2 = comSelector2
    -- 2v2 hanya punya 1 rekan tim: sembunyikan slot COM 2
    if self.comCount < 2 then
        comSelector2:setVisible(false)
        self._comLabel2:setVisible(false)
    end

    -- init com selector 3
    if self.is4v4Mode then
        local comSelector3 = display.newSprite('#unknow_select.png')
        comSelector3:setAnchorPoint(0, 0)
        comSelector3:setPosition(comSelector2:getPositionX() +
                                     comSelector2:getContentSize().width + 40,
                                 comSelector2:getPositionY())
        teamSelector:addChild(comSelector3)

        self._comLabel3 = display.newSprite('#com_label.png')
        self._comLabel3:setPosition(comSelector3:getPositionX() +
                                        comSelector3:getContentSize().width + 3 +
                                        18, comSelector3:getPositionY() +
                                        comSelector3:getContentSize().height / 3)
        teamSelector:addChild(self._comLabel3)
        self._comSelector3 = comSelector3
    end

    self:addChild(teamSelector, 50)
end

function SelectLayer:onQuestBtn()
    -- not support
    self:setTip('ComingSoon')
end

function SelectLayer:onRankingButtonClick()
    -- not support
    self:setTip('ComingSoon')
end

function SelectLayer:onSkillMenuButtonClick()
    local scene = CCScene:create()
    local skillLayer = SkillLayer:create(self)
    scene:addChild(skillLayer)

    director.pushScene(scene)
end

function SelectLayer:onPagePrev()
    self:changePage(self.currentPage - 1)
end

function SelectLayer:onPageNext()
    self:changePage(self.currentPage + 1)
end

function SelectLayer:changePage(targetPage)
    audio.playSound(ns.menu.SELECT_SOUND)

    -- wrap-around
    local newPage = targetPage
    if newPage < 1 then
        newPage = self.pageNum
    elseif newPage > self.pageNum then
        newPage = 1
    end
    if newPage == self.currentPage then return end

    self.currentPage = newPage

    for i, pageLayer in ipairs(self.pageLayers) do
        if i == newPage then
            pageLayer:show()
            pageLayer:setPositionY(0)
        else
            pageLayer:hide()
            pageLayer:setPositionY(10000)
        end
    end
end

-- kompatibilitas dengan pemanggil lama
function SelectLayer:onPageButtonClick(index)
    self:changePage(index)
end

function SelectLayer:setSelected(btn)
    if self._selectImg then
        self._selectImg:setPosition(CCPoint(btn:getPositionX() - 2,
                                            btn:getPositionY() - 2))
    end

    if not self.enableCustomSelect and self._playerSelect then return end

    self.selectHero = btn._charName

    local fd = CCFadeOut:create(1.0)
    local seq = CCRepeatForever:create(transition.sequence({fd, fd:reverse()}))

    if not self._playerSelect then
        if btn._clickTime >= 2 then
            self:setSelectHero(btn._charName)
            self._playerSelect = self.selectHero

            if not self.enableCustomSelect then
                self._selectImg:removeFromParent()
                self._selectImg = nil
            else
                if self._comLabel1 then
                    self._comLabel1:runAction(seq)
                else
                    self:afterComPicked()
                end
            end
        end

        self._heroHalfImage:removeFromParent()
        self._heroHalfImage = newHeroHalfImage(btn._charName)
        self:addChild(self._heroHalfImage, 1)

        self._heroName:removeFromParent()
        self._heroName = newHeroNameLabel(btn._charName, 100, 20)
        self:addChild(self._heroName, 5)
    elseif self.comCount >= 1 and not self._com1Select then
        self._comSelector1:setDisplayFrame(
            display.newSpriteFrame(Icons.frame('small', self.selectHero)))

        if btn._clickTime >= 2 then
            self._com1Select = self.selectHero
            self:setCom1Select(self._com1Select)

            self._comLabel1:stopAllActions()
            self._comLabel1:setOpacity(255)
            self._comLabel1:setDisplayFrame(
                display.newSpriteFrame('com_label2.png'))
            if self.comCount == 1 then self:afterComPicked() end
        end
    elseif self.comCount >= 2 and not self._com2Select then
        self._comSelector2:setDisplayFrame(
            display.newSpriteFrame(Icons.frame('small', self.selectHero)))

        if btn._clickTime >= 2 then
            self._com2Select = self.selectHero
            self:setCom2Select(self._com2Select)

            self._comLabel2:stopAllActions()
            self._comLabel2:setOpacity(255)
            self._comLabel2:setDisplayFrame(
                display.newSpriteFrame('com_label2.png'))
            if self.comCount == 2 then self:afterComPicked() end
        end
    elseif self.comCount >= 3 and not self._com3Select then
        self._comSelector3:setDisplayFrame(
            display.newSpriteFrame(Icons.frame('small', self.selectHero)))

        if btn._clickTime >= 2 then
            self._com3Select = self.selectHero
            self:setCom3Select(self._com3Select)

            self._comLabel3:stopAllActions()
            self._comLabel3:setOpacity(255)
            self._comLabel3:setDisplayFrame(
                display.newSpriteFrame('com_label2.png'))
            self:afterComPicked()
        end
    else
        self:onPickEnemy(btn)
    end
end

-- Dipanggil setelah semua slot rekan tim terisi
function SelectLayer:afterComPicked()
    if self.enemyCount > 0 then
        self:startEnemyBlink(1)
    elseif self._selectImg then
        self._selectImg:removeFromParent()
        self._selectImg = nil
    end
end

function SelectLayer:startEnemyBlink(index)
    local selector = self._enemySelectors[index]
    if not selector then return end
    local fd = CCFadeOut:create(1.0)
    selector:runAction(CCRepeatForever:create(
                           transition.sequence({fd, fd:reverse()})))
end

-- Pemilihan musuh: klik dua kali pada karakter untuk mengunci
function SelectLayer:onPickEnemy(btn)
    local idx
    for i = 1, self.enemyCount do
        if not self._enemySelects[i] then
            idx = i
            break
        end
    end
    if not idx then return end

    local selector = self._enemySelectors[idx]
    selector:setDisplayFrame(
        display.newSpriteFrame(Icons.frame('small', self.selectHero)))

    if btn._clickTime >= 2 then
        self._enemySelects[idx] = self.selectHero
        -- dibaca oleh C++ (IGameModeHandler) saat game dimulai
        save.string('customEnemy' .. idx, self.selectHero)

        selector:stopAllActions()
        selector:setOpacity(255)

        if idx < self.enemyCount then
            self:startEnemyBlink(idx + 1)
        elseif self._selectImg then
            self._selectImg:removeFromParent()
            self._selectImg = nil
        end
    end
end

function SelectLayer:initEnemySelectMode()
    log('Initial Enemy Select Mode...')
    local enemyLayer = display.newLayer()

    local sample = display.newSprite('#unknow_select.png')
    local w = sample:getContentSize().width
    local h = sample:getContentSize().height

    -- Baris musuh di bawah slot rekan tim; kalau tidak ada rekan tim,
    -- pakai posisi baris yang sama
    local rowY = 194
    if self.comCount > 0 then rowY = rowY - h - 14 end

    local enemyBg = display.newSprite('#team_bg.png', 0, rowY - 9)
    enemyBg:setAnchorPoint(0, 0)
    enemyLayer:addChild(enemyBg)

    for i = 1, self.enemyCount do
        local selector = display.newSprite('#unknow_select.png')
        selector:setAnchorPoint(0, 0)
        selector:setPosition(2 + (i - 1) * (w + 8), rowY)
        -- tanda merah supaya beda dengan slot rekan tim
        pcall(function() selector:setColor(ccc3(255, 130, 130)) end)
        enemyLayer:addChild(selector)
        self._enemySelectors[i] = selector
    end

    self:addChild(enemyLayer, 50)
end

-- True kalau semua pilihan sudah terisi (tombol karakter dikunci)
function SelectLayer:noComSelect()
    if not self.enableCustomSelect then return self._playerSelect end
    if not self._playerSelect then return false end
    if self.comCount >= 1 and not self._com1Select then return false end
    if self.comCount >= 2 and not self._com2Select then return false end
    if self.comCount >= 3 and not self._com3Select then return false end
    for i = 1, self.enemyCount do
        if not self._enemySelects[i] then return false end
    end
    return true
end

function backToStartMenu()
    log('back to main menu')

    _G.mode = nil
    audio.playSound('Audio/Menu/cancel.ogg')
    local menuScene = CCScene:create()
    local menuLayer = StartMenu:create()

    hook.registerInitHandlerOnly(menuLayer)
    menuScene:addChild(menuLayer)
    director.replaceSceneWithFade(menuScene, 1)
end
