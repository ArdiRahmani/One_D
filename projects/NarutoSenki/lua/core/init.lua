--
-- Core Configuration
--

require 'core.audio'

GameMode = require 'core.mode'

-- Daftar hero (dikirim ke C++ lewat HeroRegistry)
Heroes = require 'Hook.Heroes'

-- Alias AI karakter, mis. Madara = Asuma (dikirim ke C++ lewat CloneAI)
require 'Hook.CloneAI'
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

-- Flag perilaku per karakter (holdBuff, lockWalk), mis. Madara = Hashirama (dikirim ke C++ lewat UnitFlags)
require 'Hook.UnitFlags'

-- Alias tipe buff, mis. h2Buff = hBuff (dikirim ke C++ lewat Buffs)
require 'Hook.Buffs'

-- Aset tambahan per karakter, mis. Guy = Guy8Gates.plist (dikirim ke C++ lewat Assets)
require 'Hook.Assets'

-- Binding gambar hero (_rp, _rpf, _CutIn, _avator, _small), mis. bind Madara = Obito -> Madara_rpf pakai Obito_rpf.png (dikirim ke C++ lewat Icons)
require 'Hook.Icons'

-- Layar hasil akhir: gambar half.png, daftar flipX, nama catatan (dikirim ke C++ lewat GameOver)
require 'Hook.GameOver'
