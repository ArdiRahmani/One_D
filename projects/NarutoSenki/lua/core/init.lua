--
-- Core Configuration
--

require 'core.audio'

GameMode = require 'core.mode'

-- Daftar hero (dikirim ke C++ lewat HeroRegistry)
Heroes = require 'Hook.Heroes'

-- Alias AI karakter, mis. Madara = Asuma (dikirim ke C++ lewat CloneAI)
require 'Hook.CloneAI'

-- AI karakter yang ditulis di Lua, mis. Yagura = Tsunade (lua/AI/Tsunade.lua) (dikirim ke C++ lewat LuaAI)
require 'Hook.LuaAI'

-- Alias AI monster/summon, mis. Chibaku = JibakuEX (dikirim ke C++ lewat CloneMon)
require 'Hook.Monster'

-- Alias AI per-tick monster (Monster::setAI), mis. Shakuton = Rocket (dikirim ke C++ lewat CloneMonAI)
require 'Hook.MonsterAI'

-- Alias peluru (CharacterBase::setBullet), mis. Sanpu = TentenSRK (dikirim ke C++ lewat CloneBullet)
require 'Hook.Bullet'

-- Alias master tangkapan ct_hit (blok Itachi/Chiyo), mis. Guy = Itachi (dikirim ke C++ lewat CloneCatch)
require 'Hook.Catch'

-- Transformasi karakter, mis. Guy = Guy8Gates (dikirim ke C++ lewat Transform)
require 'Hook.Transform'

-- Aset tambahan per karakter, mis. Guy = Guy8Gates.plist (dikirim ke C++ lewat Assets)
require 'Hook.Assets'

-- Binding gambar unit (_rp, _rpf, _CutIn, _avator, _small), mis. Guy8Gates_rpf pakai Guy_rpf.png (dikirim ke C++ lewat Icons)
require 'Hook.Icons'
