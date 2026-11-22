-- HeroData.lua
local HeroData = {}

HeroData.heroDisplayName = {
    FourthRaikage = 'Ay',
    TobiMasked = 'Obito',
    FuET = 'Fuu',
    Mei = 'Mei Terumi',
    Mei2 = 'Mei Terumi',
    Masked = 'Obito',
    ThirdRaikage = 'Third  Raikage',
    Kakuzu2 = 'Kakuzu (Revamp)',
    Tsunade2 = 'Tsunade (Rvp)', 
    
}

HeroData.clanGroup = {
    Senju = {'Tobirama','Hashirama','Tsunade', 'Tsunade2', },
    Uzumaki = {'Naruto','Karin', 'Nagato',},
    Namikaze = {'Minato', },
    Yakushi = {'Kabuto', },
    Haruno = {'Sakura'},
    Yamanaka = {'Ino'},
    Shimura = {'Danzo'},
    AkasunaNo = {'Sasori'},
    Umino = {'Iruka'},
    Hyuga = {'Hinata','Neji'},
    Sarutobi = {'Asuma','Hiruzen'},
    Uchiha = {'Sasuke','Madara', 'Obito', 'Itachi', 'TobiMasked', 'Masked', 'Shisui', },
    Nara = {'Shikamaru'},
    Aburame = {'Shino', 'Torune'},
    Inuzuka = {'Kiba'},
    Akimichi = {'Choji'},
    Yuhi = {'Kurenai'},
    Might = {'Guy'},
    Rock = {'Lee'},
    Tenbin = {'Jugo'},
    Momochi = {'Zabuza'},
    Yuki = {'Haku'},
    Hatake = {'Kakashi'},
    Hoshigaki = {'Kisame'},
    Hohzuki = {'Suigetsu','Gengetsu'},
    Otsutsuki = {'Kaguya','Kimimaro','Ashura', 'Indra', 'Hagoromo'},
}
HeroData.heroClan = {}

for clan, heroes in pairs(HeroData.clanGroup) do
    for _, hero in ipairs(heroes) do
        HeroData.heroClan[hero] = clan
    end
end

HeroData.TitleGroup = {
    SageMode = {'SageNaruto', 'SageJiraiya', 'SageKabuto',},
    RinneganMode = {'SasukeRinnegan',},
     RikudoMode = {'RikudoNaruto', 'ObitoRikudo', 'MadaraRikudo',},
    Jinchuriki = {'FuET','Yagura', 'KillerBee', 'Utakata', 'Roshi', 'Han', 'Yugito', 'Kinkaku',},
    EdoTensei = {'Madara','Muu','Gengetsu', 'ThirdRaikage', 'Rasa', 'Pakura', 'Torune', 'Hanzo', 'Dankato', 'ItachiET', 'Zabuza', 'SageHashirama', },
    War = {'HinataWar','SakuraWar', 'TemariWar', 'Mifune', 'Samurai', 'ARaikage', 'Kaguya', 'Mei', 'Onoki', 'FourthRaikage',},
    YellowMask = {'TobiMasked'},
    WhiteMask = {'Masked'},
    UnMask = {'Obito'},
    EightGatesMode = {'Guy8Gates'},
    Taka = {'Sasuke', 'Karin', 'Suigetsu', 'Jugo'},
    SnakeCloak = {'Kabuto'},
    Akatsuki = {'Pain', 'Nagato', 'Konan', 'Kisame', 'Itachi', 'Sasori', 'Deidara', 'Kakuzu', 'Hidan','Zetsu',},
}

HeroData.heroTitle = {}

for title, heroes in pairs(HeroData.TitleGroup) do
    for _, hero in ipairs(heroes) do
        HeroData.heroTitle[hero] = title
    end
end

function HeroData.cleanHeroName(name)

    local remove = {'Immortal', '8Gates','War','ET','2','Jonin','Sage','Rikudo', 'Killer', 'Rock', 'Rinnegan',}

    for _,v in ipairs(remove) do
        name = name:gsub(v,'')
    end

    return name
end

return HeroData
