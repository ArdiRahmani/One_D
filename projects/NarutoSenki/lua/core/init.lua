--
-- Core Configuration
--

require 'core.audio'

GameMode = require 'core.mode'

-- Daftar hero (dikirim ke C++ lewat HeroRegistry)
Heroes = require 'Hook.Heroes'

-- Alias AI karakter, mis. Madara = Asuma (dikirim ke C++ lewat CloneAI)
require 'Hook.CloneAI'

-- Alias AI monster/summon, mis. Chibaku = JibakuEX (dikirim ke C++ lewat CloneMon)
require 'Hook.Monster'

-- Alias AI per-tick monster (Monster::setAI), mis. Shakuton = Rocket (dikirim ke C++ lewat CloneMonAI)
require 'Hook.MonsterAI'

-- Transformasi karakter, mis. Guy = Guy8Gates (dikirim ke C++ lewat Transform)
require 'Hook.Transform'

-- Aset tambahan per karakter, mis. Guy = Guy8Gates.plist (dikirim ke C++ lewat Assets)
require 'Hook.Assets'
