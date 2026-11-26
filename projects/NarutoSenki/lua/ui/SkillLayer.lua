--
-- SelectLayer
--
local transformList = {
    ['Naruto'] = 'SageNaruto',
    ['SageNaruto'] = 'RikudoNaruto',
    ['Jiraiya'] = 'SageJiraiya',
    ['Sasuke'] = 'ImmortalSasuke',
    ['Pain'] = 'Nagato'
}

-- Skala gambar half saat dipakai sebagai pengganti _full.png (1.0 = ukuran asli)
local HALF_AS_FULL_SCALE = 1.0
-- Skala khusus per hero (mengalahkan skala umum). Isi sesuai kebutuhan.
-- FULL_SCALE_BY_HERO: untuk gambar <n>_full.png / <n>_full2.png (plist)
-- HALF_SCALE_BY_HERO: untuk gambar half yang dipakai sebagai pengganti _full.png
local FULL_SCALE_BY_HERO = {
    Chiyo = 0.4,
}
local HALF_SCALE_BY_HERO = {
    Chiyo = 0.4,
}

local useFull2ImageList = {
    'Hinata', 'Sakura', 'Ino', 'Tsunade', 'Tenten', 'Konan', 'Karin'
}

-- Karakter yang punya beberapa halaman (page) skill.
-- Key   = nama karakter asli di SelectLayer (= page 1).
-- Value = daftar varian untuk page 2, page 3, page 4, ... (urutan = nomor page).
--   Guy = {'Guy8Gates'}            => page 1 Guy, page 2 Guy8Gates
--   X   = {'XPage2', 'XPage3'}     => page 1 X, page 2 XPage2, page 3 XPage3
-- Boleh juga string tunggal (dianggap page 2 saja) agar format lama tetap jalan.
local skillPageCharacters = {
    Guy = {'Guy8Gates'},
    Tobi = {'TobiMasked'},
    Hashirama = {'SageHashirama'},
    Pain = {'Nagato'},
    Naruto = {'SageNaruto', 'RikudoNaruto'},
    Sasuke = {'ImmortalSasuke'},
    Jiraiya = {'SageJiraiya'},
    Minato = {'MinatoET'},
    Zabuza = {'ZabuzaET'},
    Obito = {'ObitoRikudo'}
}

SkillLayer = class('SkillLayer', function() return display.newLayer() end)

-- Dipakai SelectLayer untuk mereset pilihan page saat layar pilih dibuka.
SkillLayer.skillPageCharacters = skillPageCharacters

-- Daftar varian (page 2..N) untuk hero. nil kalau hero tidak punya page.
local function getVariants(baseName)
    local v = skillPageCharacters[baseName]
    if type(v) == 'string' then return {v} end
    if type(v) == 'table' and #v > 0 then return v end
    return nil
end

-- Jumlah page hero (termasuk page 1). 1 kalau hero tidak punya page tambahan.
function SkillLayer.getPageCount(baseName)
    local variants = getVariants(baseName)
    return variants and (#variants + 1) or 1
end

-- Nama aset hero untuk nomor page tertentu (page 1 = nama asli).
function SkillLayer.getPageHeroName(baseName, page)
    local variants = getVariants(baseName)
    if page and page >= 2 and variants and variants[page - 1] then
        return variants[page - 1]
    end
    return baseName
end

-- Pilihan page disimpan sebagai INDEX di UserDefault:
--   heroPageIdx_<Hero> = 1, 2, 3, 4, ...  (sumber utama, dipakai Lua)
-- Nama varian hasil index juga ditulis ke:
--   heroPage_<Hero>    = nama varian ('' untuk page 1)
-- supaya C++ (IGameModeHandler::resolveHeroVariant) tinggal membaca nama,
-- apa pun nomor page-nya. Berlaku untuk player, COM rekan, hero acak, dan musuh.
-- Page tersimpan untuk hero ini (1..N), nil kalau hero tanpa page.
function SkillLayer.getSavedPage(baseName)
    local count = SkillLayer.getPageCount(baseName)
    if count <= 1 then return nil end
    local idx = save.getInteger('heroPageIdx_' .. baseName, 0) or 0
    if idx == 0 then
        -- Save lama (hanya nama varian): cocokkan nama ke index-nya.
        local name = save.getString('heroPage_' .. baseName)
        idx = 1
        if name and name ~= '' then
            for i = 2, count do
                if SkillLayer.getPageHeroName(baseName, i) == name then idx = i break end
            end
        end
    end
    if idx < 1 or idx > count then idx = 1 end
    return idx
end

function SkillLayer.savePageChoice(baseName, page)
    local count = SkillLayer.getPageCount(baseName)
    if count <= 1 then return end
    if not page or page < 1 or page > count then page = 1 end
    save.integer('heroPageIdx_' .. baseName, page)
    save.string('heroPage_' .. baseName,
                page == 1 and '' or SkillLayer.getPageHeroName(baseName, page))
end

function SkillLayer:ctor(selectLayer)
    self._selectLayer = selectLayer
    self._skillExplain = nil

    self._buyLayer = nil
    self._buyType = SelectButton.Type.Menu
    self._fengSprite = nil
    self._fengSprite2 = nil
    self._autoMove = true

    self.coinLabel = nil
    self._heroFull = nil
    self.changemenu = nil
    self.skillGroup = nil
    self.bgSprite = nil
    self.bonds = nil

    self:init()
    self:initInterface()
end

function SkillLayer:create(selectLayer) return SkillLayer.new(selectLayer) end

function SkillLayer:init()
    log('Initial SkillLayer')

    -- produce groud
    local gold_left = display.newSprite('#gold_left.png')
    gold_left:setAnchorPoint(0, 0)
    gold_left:setPosition(0, 20)
    self:addChild(gold_left, 1)

    local gold_right = display.newSprite('#gold_right.png')
    gold_right:setAnchorPoint(0, 1)
    gold_right:setPosition(display.width - gold_right:getContentSize().width -
                               20, display.height - 20)
    self:addChild(gold_right, 1)

    -- produce the cloud
    local cloud_left = display.newSprite('#cloud.png')
    cloud_left:setPosition(0, 15)
    cloud_left:setFlipX(true)
    cloud_left:setFlipY(true)
    cloud_left:setAnchorPoint(0, 0)
    self:addChild(cloud_left, 2)

    local cmv1 = CCMoveBy:create(1, CCPoint(-15, 0))
    local cseq1 = CCRepeatForever:create(
                      transition.sequence({cmv1, cmv1:reverse()}))
    cloud_left:runAction(cseq1)

    local cloud_right = display.newSprite('#cloud.png')
    cloud_right:setPosition(display.width - cloud_right:getContentSize().width,
                            15)
    cloud_right:setFlipY(true)
    cloud_right:setAnchorPoint(0, 0)
    self:addChild(cloud_right, 2)

    local cmv2 = CCMoveBy:create(1, CCPoint(15, 0))
    local cseq2 = CCRepeatForever:create(
                      transition.sequence({cmv2, cmv2:reverse()}))
    cloud_right:runAction(cseq2)

    local cloud_top = display.newSprite('#cloud_top.png')
    cloud_top:setPosition(display.cx, display.height -
                              (cloud_top:getContentSize().height + 12))
    cloud_top:setAnchorPoint(0.5, 0)
    self:addChild(cloud_top, 1)

    -- produce the menu_bar
    local menu_bar_b = display.newSprite('menu_bar2.png')
    menu_bar_b:setAnchorPoint(0, 0)
    menu_bar_b:fullScreen()
    self:addChild(menu_bar_b, 2)

    local menu_bar_t = display.newSprite('menu_bar3.png')
    menu_bar_t:setAnchorPoint(0, 0)
    menu_bar_t:setPosition(0,
                           display.height - menu_bar_t:getContentSize().height)
    menu_bar_t:fullScreen()
    self:addChild(menu_bar_t, 2)

    local record_title = display.newSprite('#record_title.png')
    record_title:setAnchorPoint(0, 0)
    record_title:setPosition(2, display.height -
                                 record_title:getContentSize().height - 2)
    self:addChild(record_title, 3)

    local start_bt = ui.newImageMenuItem({
        image = display.newSprite('#return_btn.png'),
        listener = handler(self, SkillLayer.onCancel)
    })
    local menu = ui.newMenu({start_bt})
    menu:setPosition(display.width - 38, 86)
    self:addChild(menu, 5)

    return true
end

function SkillLayer:initInterface()
    if self._selectLayer then
        -- Nama karakter asli tidak diubah agar tetap cocok dengan pilihan C++.
        self._skillBaseHero = self._selectLayer.selectHero
        self._selectLayer._skillPageByHero = self._selectLayer._skillPageByHero or {}
        -- Sumber utama = UserDefault (bertahan setelah keluar/selesai pertandingan)
        local savedPage = SkillLayer.getSavedPage(self._skillBaseHero)
        self._skillPage = savedPage or self._selectLayer._skillPageByHero[self._skillBaseHero] or 1
        self.selectHero = self:getSkillPageHero(self._skillPage)
        self:updateSelectLayerHalfImage()
    end

    local bgSprite = display.newSprite('blue_bg.png', 0, 0)
    bgSprite:fullScreen()
    bgSprite:setAnchorPoint(0, 0)
    self:addChild(bgSprite, -5)

    local winNum = tools.readWinNumFromSQL(self._skillBaseHero or self.selectHero)
    -- Jumlah kalah dibaca bersama jumlah menang (column2 di CharRecord)
    local loseNum = tools.readLoseNumFromSQL(self._skillBaseHero or self.selectHero)
    self._loseNum = loseNum

    local rank_src
    local isBlink = false
    local skillbg_src
    if winNum >= 300 then
        skillbg_src = 'skill_bg6.png'
        rank_src = 'rank_sss.png'
        isBlink = true
    elseif winNum >= 200 then
        skillbg_src = 'skill_bg5.png'
        rank_src = 'rank_ss.png'
        isBlink = true
    elseif winNum >= 100 then
        skillbg_src = 'skill_bg4.png'
        rank_src = 'rank_s.png'
        isBlink = true
    elseif winNum >= 50 then
        skillbg_src = 'skill_bg3.png'
        rank_src = 'rank_a.png'
    elseif winNum >= 25 then
        skillbg_src = 'skill_bg2.png'
        rank_src = 'rank_b.png'
    else
        skillbg_src = 'skill_bg1.png'
        rank_src = 'rank_c.png'
    end

    local skill_bg = display.newSprite('#' .. skillbg_src, display.cx,
                                       display.cy)
    self:addChild(skill_bg, 2)
    self.bgSprite = skill_bg

    self:trySetCharacterChangeButton(self.selectHero)

    local goldBG = display.newSprite('#gold_bg.png')
    goldBG:setAnchorPoint(0.5, 0)
    goldBG:setPosition(display.cx + skill_bg:getContentSize().width / 4 + 25,
                       display.cy + skill_bg:getContentSize().width / 2 - 74)
    self:addChild(goldBG, 9)

    local coinsNum = tools.readCoinFromSQL()
    self.coinLabel = CCLabelBMFont:create(tostring(coinsNum), 'Fonts/1.fnt')
    self.coinLabel:setScale(0.3)
    self.coinLabel:setAnchorPoint(0.5, 0)
    self.coinLabel:setPosition(goldBG:getPositionX() -
                                   goldBG:getContentSize().width / 2 + 40,
                               goldBG:getPositionY() + 4)
    self:addChild(self.coinLabel, 15)

    local bondBG = display.newSprite('#golds_1.png')
    bondBG:setAnchorPoint(0.5, 0)
    bondBG:setPosition(
        goldBG:getPositionX() - goldBG:getContentSize().width / 2 + 15,
        goldBG:getPositionY() + 4)
    self:addChild(bondBG, 15)

    local bondBG2 = display.newSprite('#golds_2.png')
    local bonds = display.newProgressTimer(bondBG2, display.PROGRESS_TIMER_BAR)
    self.bonds = bonds
    bonds:setAnchorPoint(0.5, 0)
    -- bonds:setMidpoint(CCPoint(0, 0)) -- is default value
    bonds:setBarChangeRate(CCPoint(0, 1))

    if coinsNum >= 99999 then
        bonds:setPercentage(100)
    elseif coinsNum >= 80000 then
        bonds:setPercentage(coinsNum / 1000)
    end

    bonds:setPosition(
        goldBG:getPositionX() - goldBG:getContentSize().width / 2 + 15,
        goldBG:getPositionY() + 4)
    self:addChild(bonds, 20)

    self._heroFullX = display.cx
    self._heroFullY = display.cy - skill_bg:getContentSize().height / 2 + 23
    self:setHeroFullImage(self.selectHero,
                          table.has(useFull2ImageList, self.selectHero) and winNum >= 100)

    self:initSkillPageButtons()

    if isBlink then
        local blink = display.newSprite('#skill_blink6.png', display.cx,
                                        display.cy)
        self:addChild(blink, 9)

        local tempArray = CCArray:create()

        for i = 1, 6 do
            local frame = display.newSpriteFrame('skill_blink' .. i .. '.png')
            tempArray:addObject(frame)
        end

        local tempAnimation = CCAnimation:createWithSpriteFrames(tempArray, 0.1)

        blink:runAction(CCRepeatForever:create(
                            transition.sequence({
                CCAnimate:create(tempAnimation), CCDelayTime:create(2.0)
            })))
    end

    local frameSprite = display.newSprite('#skill_frame.png', display.cx,
                                          display.cy)
    self:addChild(frameSprite, 10)

    local rankSprite = display.newSprite('#' .. rank_src)
    rankSprite:setAnchorPoint(0, 0)
    rankSprite:setPosition(skill_bg:getPositionX() -
                               skill_bg:getContentSize().width / 2 + 5,
                           skill_bg:getPositionY() +
                               skill_bg:getContentSize().height / 2 - 20)
    self:addChild(rankSprite, 11)

    local detailBG1 = display.newSprite('#detail_bg.png')
    detailBG1:setAnchorPoint(0, 0)
    detailBG1:setPosition(skill_bg:getPositionX() -
                              skill_bg:getContentSize().width / 2 + 8,
                          skill_bg:getPositionY() -
                              skill_bg:getContentSize().height / 2 + 74)
    self:addChild(detailBG1, 5)

    local recordTime = tools.readRecordTimeFromSQL(self._skillBaseHero or self.selectHero)
    local bestLabel
    if recordTime == '' then
        bestLabel = CCLabelBMFont:create('00:00:00', 'Fonts/1.fnt')
    else
        bestLabel = CCLabelBMFont:create(recordTime, 'Fonts/1.fnt')
    end

    bestLabel:setScale(0.3)
    bestLabel:setAnchorPoint(0, 0)
    bestLabel:setPosition(detailBG1:getPositionX() + 8,
                          detailBG1:getPositionY() + 7)
    self:addChild(bestLabel, 6)

    local detailBG2 = display.newSprite('#detail_bg.png')
    detailBG2:setAnchorPoint(0, 0)
    detailBG2:setPosition(skill_bg:getPositionX() +
                              skill_bg:getContentSize().width / 2 -
                              detailBG2:getContentSize().width - 8,
                          skill_bg:getPositionY() -
                              skill_bg:getContentSize().height / 2 + 74)
    self:addChild(detailBG2, 5)

    local recordLabel = CCLabelBMFont:create(winNum .. ' bonds', 'Fonts/1.fnt')
    recordLabel:setScale(0.3)
    recordLabel:setAnchorPoint(0, 0)
    recordLabel:setPosition(detailBG2:getPositionX() + 12,
                            detailBG2:getPositionY() + 7)
    self:addChild(recordLabel, 6)

    self:updateSkillGroup()
    self:scheduleUpdateWithPriorityLua(handler(self, SkillLayer.update), 0)
end

-- Gambar besar hero di SkillLayer. Urutan: <n>_full2.png (kalau useFull2) -> <n>_full.png (plist)
-- -> Img/Half/<n>_half.png (file; ikut GameOver half/file/path) kalau frame _full tidak ada.
-- Sprite dibuat ulang karena frame plist dan file half tidak bisa saling menggantikan lewat setDisplayFrame.
function SkillLayer:setHeroFullImage(heroName, useFull2)
    local cache = CCSpriteFrameCache:sharedSpriteFrameCache()
    local frameName
    if useFull2 and cache:spriteFrameByName(heroName .. '_full2.png') then
        frameName = heroName .. '_full2.png'
    elseif cache:spriteFrameByName(heroName .. '_full.png') then
        frameName = heroName .. '_full.png'
    end

    local sprite, scale
    if frameName then
        sprite = display.newSprite('#' .. frameName)
        scale = FULL_SCALE_BY_HERO[heroName] or 1.0
    else
        local half = Icons and Icons.half and Icons.half(heroName)
        if half then
            log('[SkillLayer] ' .. heroName .. '_full.png tidak ada, pakai ' .. half)
            sprite = display.newSprite(half)
            scale = HALF_SCALE_BY_HERO[heroName] or HALF_AS_FULL_SCALE
        else
            -- tidak ada _full maupun _half: pakai perilaku lama (error frame dicatat display)
            sprite = display.newSprite('#' .. heroName .. '_full.png')
            scale = FULL_SCALE_BY_HERO[heroName] or 1.0
        end
    end

    if self._heroFull then self._heroFull:removeFromParent() end
    self._heroFull = sprite
    sprite:setAnchorPoint(0.5, 0)
    sprite:setScale(scale)
    sprite:setPosition(self._heroFullX, self._heroFullY)
    -- z=4: di bawah detail_bg (z=5) agar tidak menimpanya saat sprite dibuat ulang (ganti page/transform)
    self:addChild(sprite, 4)
end

-- Ubah hanya gambar half di SelectLayer. Font/nama karakter tidak disentuh.
function SkillLayer:updateSelectLayerHalfImage()
    if not self._selectLayer or not self._selectLayer._heroHalfImage then return end

    pcall(function()
        self._selectLayer:setHeroHalf(self.selectHero)
    end)
end

-- Mengembalikan nama aset karakter untuk halaman yang sedang dipilih.
function SkillLayer:getSkillPageHero(page)
    local baseName = self._skillBaseHero or self.selectHero
    return SkillLayer.getPageHeroName(baseName, page)
end

-- Ikon dan posisi mengikuti page button milik SelectLayer.
function SkillLayer:initSkillPageButtons()
    local pageCount = self._skillBaseHero and
                          SkillLayer.getPageCount(self._skillBaseHero) or 1
    if pageCount <= 1 then return end

    -- Frame ikon page{i}_off/on; kalau belum ada (mis. page4) pakai page terakhir yang ada.
    local cache = CCSpriteFrameCache:sharedSpriteFrameCache()
    local function iconIndex(i)
        while i > 1 and not cache:spriteFrameByName('page' .. i .. '_off.png') do
            i = i - 1
        end
        return i
    end

    local pageButtons = {}
    for i = 1, pageCount do
        local pageIndex = i
        local icon = iconIndex(i)
        local button = ui.newImageMenuItem({
            image = '#page' .. tostring(icon) .. '_off.png',
            imageSelected = '#page' .. tostring(icon) .. '_on.png',
            listener = function()
                self:onSkillPageButtonClick(pageIndex)
            end
        })
        button:setScale(0.7)
        if i == self._skillPage then
            button:selected()
        else
            button:unselected()
        end
        pageButtons[#pageButtons + 1] = button
    end

    self._skillPageButtons = pageButtons
    self._skillPageMenu = ui.newMenu(pageButtons)
    self._skillPageMenu:alignItemsHorizontallyWithPadding(20)
    self._skillPageMenu:setPosition(display.width / 2 + 48, 42)
    self:addChild(self._skillPageMenu, 20)
end

function SkillLayer:onSkillPageButtonClick(page)
    local pageCount = SkillLayer.getPageCount(self._skillBaseHero or self.selectHero)
    if page < 1 or page > pageCount then return end
    if self._skillPage == page then return end

    audio.playSound(ns.menu.SELECT_SOUND)
    self._skillPage = page
    self.selectHero = self:getSkillPageHero(page)
    self:updateSelectLayerHalfImage()
    SkillLayer.savePageChoice(self._skillBaseHero, page)

    -- Simpan pada instance SelectLayer, jadi tidak reset saat SkillLayer dibuka lagi.
    if self._selectLayer then
        self._selectLayer._skillPageByHero[self._skillBaseHero] = page
    end

    if self._skillExplain then
        self._skillExplain:removeFromParent()
        self._skillExplain = nil
    end

    self:setHeroFullImage(self.selectHero)
    self:updateSkillGroup()

    -- Perbarui status visual tombol page.
    if self._skillPageButtons then
        for i = 1, #self._skillPageButtons do
            local button = self._skillPageButtons[i]
            if i == page then
                button:selected()
            else
                button:unselected()
            end
        end
    end
end

function SkillLayer:onChangeBtn()
    audio.playSound('Audio/Menu/chang_btn.ogg')
    self.skillGroup:removeFromParent()
    self.changemenu:removeFromParent()

    local selectedHeroName = transformList[self.selectHero]
    if selectedHeroName then self.selectHero = selectedHeroName end
    self:trySetCharacterChangeButton(selectedHeroName, 2)

    self:setHeroFullImage(self.selectHero)
    self:updateSkillGroup()
end

function SkillLayer:updateSkillGroup()
    self.skillGroup = TouchGroup:create()

    for i = 1, 5 do
        local skill_btn = SelectButton:create(
                              self.selectHero .. '_skill' .. i .. '.png')

        if i == 4 then
            skill_btn._isUnlock = true
        elseif i == 5 then
            skill_btn._isCanBuy2 = false
            skill_btn._isUnlock = true
        end

        skill_btn:setPosition(CCPoint((i - 1) * 46, 0))
        skill_btn._btnType = i + 2

        skill_btn._selectLayer = self._selectLayer
        skill_btn._skillLayer = self
        self.skillGroup:addWidget(skill_btn)
    end

    self.skillGroup:setPosition(self.bgSprite:getPositionX() -
                                    self.bgSprite:getContentSize().width / 2 + 2,
                                60)
    self:addChild(self.skillGroup, 500)
end

function SkillLayer:setSkillExplain(buttonType)
    audio.playSound('Audio/Menu/select.ogg')
    if self._skillExplain then self._skillExplain:removeFromParent() end

    local imgPath
    if buttonType == SelectButton.Type.Unlock1 then
        imgPath = 'fenglabel.png'
    elseif buttonType == SelectButton.Type.Unlock2 then
        imgPath = 'fenglabel2.png'
    else
        imgPath = self.selectHero .. '_label' .. (buttonType - 2) .. '.png'
    end

    local clipper = CCClippingNode:create()
    local stencil = display.newSprite('#clipperMask.png')
    stencil:setAnchorPoint(0, 0)
    clipper:setStencil(stencil)

    self._skillExplain = display.newSprite('#' .. imgPath)
    self._skillExplain:setAnchorPoint(0, 0)
    self._skillExplain:setPositionX(10)

    clipper:setPosition(display.cx - self.bgSprite:getContentSize().width / 2 +
                            2, display.cy -
                            self.bgSprite:getContentSize().height / 2 + 4)
    clipper:addChild(self._skillExplain)

    self:addChild(clipper, 600)
end

function SkillLayer:update(dt)
    if not self._skillExplain or not self._autoMove then return end

    local lableX = self._skillExplain:getContentSize().width

    if self._skillExplain:getPositionX() >= -lableX then
        self._skillExplain:setPositionX(self._skillExplain:getPositionX() - 0.6)
    else
        self._skillExplain:setPositionX(320)
    end
end

function SkillLayer:onCancel()
    self._skillExplain = nil
    self._buyLayer = nil
    self._fengSprite = nil
    self._fengSprite2 = nil

    self:removeFromParent()
    audio.playSound('Audio/Menu/cancel.ogg')

    director.popScene()
end

function SkillLayer:trySetCharacterChangeButton(name, level)
    if not level then level = 1 end
    if not name or level < 1 then return end

    if transformList[name] ~= nil then
        local index = level == 1 and '' or level

        local change_bt = ui.newImageMenuItem({
            image = display.newSprite('#change_btn' .. index .. '.png'),
            listener = handler(self, SkillLayer.onChangeBtn)
        })
        self.changemenu = ui.newMenu({change_bt})
        self.changemenu:setPosition(display.cx, display.cy -
                                        self.bgSprite:getContentSize().height /
                                        2 + 120)
        local fd = CCFadeOut:create(1.0)
        local seq = CCRepeatForever:create(
                        transition.sequence({fd, fd:reverse()}))
        change_bt:runAction(seq)
        self:addChild(self.changemenu, 15)
    end
end
